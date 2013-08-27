#include <forward_list>
#include <iostream>
#include <memory>
#include <type_traits> // underlying_type

#include <boost/container/flat_map.hpp>
#include <boost/variant.hpp>

#include "gtest/gtest.h"

#include "tests/hom/common.hh"

/*------------------------------------------------------------------------------------------------*/

typedef conf::Identifier identifier_type;
typedef conf::Values bitset_type;

/*------------------------------------------------------------------------------------------------*/

enum class binop {add, sub, mul};

struct binary_operation;
struct operand;

typedef boost::variant<binary_operation, operand> ast_type;

struct operand
{
  const identifier_type identifier;
  mutable bitset_type values;

  operand(const identifier_type& id)
    : identifier(id)
    , values()
  {}

  bool
  operator==(const operand& other)
  const noexcept
  {
    return identifier == other.identifier and values == other.values;
  }
};

struct binary_operation
{
  const enum binop operation;
  const std::unique_ptr<ast_type> lhs;
  const std::unique_ptr<ast_type> rhs;

  binary_operation(enum binop op, const ast_type& l, const ast_type& r)
    : operation(op)
    , lhs(new ast_type(l))
    , rhs(new ast_type(r))
  {}

  // Deep-copy.
  binary_operation(const binary_operation& other)
    : operation(other.operation)
    , lhs(new ast_type(*other.lhs))
    , rhs(new ast_type(*other.rhs))
  {}

  bool
  operator==(const binary_operation& other)
  const noexcept
  {
    return operation == other.operation and *lhs == *other.lhs and *rhs == *other.rhs;
  }
};

/*------------------------------------------------------------------------------------------------*/

struct indexed_ast
{
  ast_type ast;

  typedef boost::container::flat_map<identifier_type, std::forward_list<bitset_type*>> index_type;
  index_type index;

  struct indexer
    : public boost::static_visitor<void>
  {
    index_type& index_;

    indexer(index_type& idx)
      : index_(idx)
    {}

    void
    operator()(const binary_operation& bop)
    const
    {
      apply_visitor(*this, *bop.lhs);
      apply_visitor(*this, *bop.rhs);
    }

    void
    operator()(const operand& op)
    const
    {
      auto lb = index_.lower_bound(op.identifier);
      if (lb != index_.end() and not index_.key_comp()(op.identifier, lb->first))
      {
        lb->second.emplace_front(&(op.values));
      }
      else
      {
        auto ins = index_.emplace_hint(lb, op.identifier, std::forward_list<bitset_type*>());
        ins->second.emplace_front(&(op.values));
      }
    }
  };

  indexed_ast(ast_type&& a)
    : ast(std::move(a))
    , index()
  {
    indexer idx(index);
    apply_visitor(idx, ast);
  }

  void
  update(const identifier_type& id, bitset_type val)
  {
    for (auto& x : index.find(id)->second)
    {
      *x = val;
    }
  }

  bool
  operator==(const indexed_ast& other)
  const noexcept
  {
    return index == other.index;
  }
};

/*------------------------------------------------------------------------------------------------*/

struct evaluation_visitor
  : public boost::static_visitor<bitset_type>
{
  bitset_type
  operator()(const operand& op)
  const noexcept
  {
    return op.values;
  }

  std::size_t
  apply(enum binop op, std::size_t lhs, std::size_t rhs)
  const noexcept
  {
    switch (op)
    {
      case binop::add : return lhs + rhs;
      case binop::sub : return lhs - rhs;
      case binop::mul : return lhs * rhs;
    }
    __builtin_unreachable();
  }

  bitset_type
  operator()(const binary_operation& op)
  const noexcept
  {
    bitset_type res;
    const bitset_type lhs = apply_visitor(*this, *op.lhs);
    const bitset_type rhs = apply_visitor(*this, *op.rhs);

    for (std::size_t i = 0; i < 64; ++i)
    {
      if (lhs.content().test(i))
      {
        for (std::size_t j = 0; j < 64; ++j)
        {
          if (rhs.content().test(j))
          {
            res.insert(apply(op.operation, i, j));
          }
        }
      }
    }
    return res;
  }
};

struct evaluator
{
  std::shared_ptr<indexed_ast> ast_ptr_;

  evaluator(const std::shared_ptr<indexed_ast>& ast)
    : ast_ptr_(ast)
  {}

  // Called by the library.
  void
  update(const std::string& identifier, const bitset_type& val)
  {
    ast_ptr_->update(identifier, val);
  }

  // Called by the library.
  bitset_type
  evaluate()
  {
    return apply_visitor(evaluation_visitor(), ast_ptr_->ast);
  }

  bool
  operator==(const evaluator& other)
  const noexcept
  {
    return ast_ptr_->ast == other.ast_ptr_->ast;
  }
};

struct print_visitor
  : public boost::static_visitor<void>
{
  std::ostream& os;

  print_visitor(std::ostream& o)
    : os(o)
  {}

  void
  operator()(const operand& op)
  const noexcept
  {
    os << op.identifier;
  }

  void
  print_op(enum binop op)
  const noexcept
  {
    switch (op)
    {
      case binop::add : os << " + "; break;
      case binop::sub : os << " - "; break;
      case binop::mul : os << " * "; break;
    }
  }

  void
  operator()(const binary_operation& op)
  const noexcept
  {
    os << "(";
    apply_visitor(*this, *op.lhs);
    print_op(op.operation);
    apply_visitor(*this, *op.rhs);
    os << ")";
  }
};

std::ostream&
operator<<(std::ostream& os, const indexed_ast& a)
{
  print_visitor v(os);
  apply_visitor(v, a.ast);
  return os;
}

std::ostream&
operator<<(std::ostream& os, const evaluator& e)
{
  return os << *e.ast_ptr_;
}

/*------------------------------------------------------------------------------------------------*/

namespace std {

template <>
struct hash<indexed_ast>
{
  struct visitor
    : public boost::static_visitor<std::size_t>
  {
    std::size_t
    operator()(const operand& op)
    const noexcept
    {
      return std::hash<std::decay<decltype(op.identifier)>::type>()(op.identifier);
    }

    std::size_t
    hash_op(enum binop op)
    const noexcept
    {
      typedef std::underlying_type<enum binop>::type type;
      return std::hash<type>()(static_cast<type>(op));
    }

    std::size_t
    operator()(const binary_operation& op)
    const noexcept
    {
      std::size_t seed = hash_op(op.operation);
      sdd::util::hash_combine(seed, apply_visitor(*this, *op.lhs));
      sdd::util::hash_combine(seed, apply_visitor(*this, *op.rhs));
      return seed;
    }
  };

  std::size_t
  operator()(const indexed_ast& idx)
  const noexcept
  {
    
    return apply_visitor(visitor(), idx.ast);
  }
};

template <>
struct hash<evaluator>
{
  std::size_t
  operator()(const evaluator& e)
  const noexcept
  {
    return std::hash<indexed_ast>()(*e.ast_ptr_);
  }
};

} // namespace std

/*------------------------------------------------------------------------------------------------*/

struct hom_expression_test
  : public testing::Test
{
  sdd::manager<conf> m;
  sdd::hom::context<conf>& cxt;

  const SDD zero;
  const SDD one;
  const hom id;

  const std::shared_ptr<indexed_ast> ast1;

  hom_expression_test()
    : m(sdd::manager<conf>::init(small_conf()))
    , cxt(sdd::global<conf>().hom_context)
    , zero(sdd::zero<conf>())
    , one(sdd::one<conf>())
    , id(sdd::Id<conf>())
    , ast1(mk_ast1())
  {}

  std::shared_ptr<indexed_ast>
  mk_ast1()
  {
    binary_operation op{binop::add, operand{"a"}, operand{"b"}};
    return std::make_shared<indexed_ast>(std::move(op));
  }
};

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_expression_test, construction)
{
  {
    const auto l = {"a", "b"};
    order o(order_builder {"a", "b"});
    hom h1 = Expression(o, evaluator(ast1), l.begin(), l.end(), "a");
    hom h2 = Expression(o, evaluator(ast1), l.begin(), l.end(), "a");
    ASSERT_EQ(h1, h2);
  }
  {
    const auto l = {"a", "b"};
    order o(order_builder {"a", "b"});
    ASSERT_EQ(Expression(o, evaluator(ast1), l.begin(), l.begin(), "a"), id);
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_expression_test, flat_evaluation)
{
//  {
//    const auto l = {"a", "b"};
//    order o(order_builder {"c", "a", "b"});
//    hom h = Expression(o, evaluator(ast1), l.begin(), l.end(), "c");
//    SDD s0(2, {0}, SDD(1, {1}, SDD(0, {1}, one)));
//    SDD s1(2, {2}, SDD(1, {1}, SDD(0, {1}, one)));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    const auto l = {"a", "b"};
//    order o(order_builder {"c", "a", "b"});
//    hom h = Expression(o, evaluator(ast1), l.begin(), l.end(), "c");
//    SDD s0 = SDD(2, {0}, SDD(1, {1}, SDD(0, {1}, one)))
//           + SDD(2, {1}, SDD(1, {2}, SDD(0, {2}, one)));
//    SDD s1 = SDD(2, {2}, SDD(1, {1}, SDD(0, {1}, one)))
//           + SDD(2, {4}, SDD(1, {2}, SDD(0, {2}, one)));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"a", "b", "c"});
//    hom h = Expression(o, evaluator{ast1}, {"a", "b"}, "c");
//    SDD s0(2, {1}, SDD(1, {1}, SDD(0, {0}, one)));
//    SDD s1(2, {1}, SDD(1, {1}, SDD(0, {2}, one)));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"a", "b", "c"});
//    hom h = Expression(o, evaluator{ast1}, {"a", "b"}, "c");
//    SDD s0 = SDD(2, {1}, SDD(1, {1}, SDD(0, {33}, one)))
//           + SDD(2, {2}, SDD(1, {2}, SDD(0, {42}, one)));
//    SDD s1 = SDD(2, {1}, SDD(1, {1}, SDD(0, {2}, one)))
//           + SDD(2, {2}, SDD(1, {2}, SDD(0, {4}, one)));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"a", "b", "c", "d"});
//    hom h = Expression(o, evaluator{ast1}, {"a", "b"}, "c");
//    SDD s0(3, {1}, SDD(2, {1}, SDD(1, {0}, SDD(0, {63}, one))));
//    SDD s1(3, {1}, SDD(2, {1}, SDD(1, {2}, SDD(0, {63}, one))));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"a", "b", "c", "d"});
//    hom h = Expression(o, evaluator{ast1}, {"a", "b"}, "c");
//    SDD s0 = SDD(3, {1}, SDD(2, {1}, SDD(1, {0}, SDD(0, {33}, one))))
//           + SDD(3, {2}, SDD(2, {2}, SDD(1, {1}, SDD(0, {42}, one))));
//    SDD s1 = SDD(3, {1}, SDD(2, {1}, SDD(1, {2}, SDD(0, {33}, one))))
//           + SDD(3, {2}, SDD(2, {2}, SDD(1, {4}, SDD(0, {42}, one))));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"a", "b", "d", "c"});
//    hom h = Expression(o, evaluator{ast1}, {"a", "b"}, "c");
//    SDD s0(3, {1}, SDD(2, {1}, SDD(1, {33}, SDD(0, {10}, one))));
//    SDD s1(3, {1}, SDD(2, {1}, SDD(1, {33}, SDD(0, {2}, one))));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"a", "b", "d", "c"});
//    hom h = Expression(o, evaluator{ast1}, {"a", "b"}, "c");
//    SDD s0 = SDD(3, {1}, SDD(2, {1}, SDD(1, {33}, SDD(0, {10}, one))))
//           + SDD(3, {2}, SDD(2, {2}, SDD(1, {42}, SDD(0, {20}, one))));
//    SDD s1 = SDD(3, {1}, SDD(2, {1}, SDD(1, {33}, SDD(0, {2}, one))))
//           + SDD(3, {2}, SDD(2, {2}, SDD(1, {42}, SDD(0, {4}, one))));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"d", "a", "b", "c"});
//    hom h = Expression(o, evaluator{ast1}, {"a", "b"}, "c");
//    SDD s0(3, {33}, SDD(2, {1}, SDD(1, {1}, SDD(0, {10}, one))));
//    SDD s1(3, {33}, SDD(2, {1}, SDD(1, {1}, SDD(0, {2}, one))));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"d", "a", "b", "c"});
//    hom h = Expression(o, evaluator{ast1}, {"a", "b"}, "c");
//    SDD s0 = SDD(3, {33}, SDD(2, {1}, SDD(1, {1}, SDD(0, {10}, one))))
//           + SDD(3, {42}, SDD(2, {2}, SDD(1, {2}, SDD(0, {20}, one))));
//    SDD s1 = SDD(3, {33}, SDD(2, {1}, SDD(1, {1}, SDD(0, {2}, one))))
//           + SDD(3, {42}, SDD(2, {2}, SDD(1, {2}, SDD(0, {4}, one))));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"a", "d", "b", "c"});
//    hom h = Expression(o, evaluator{ast1}, {"a", "b"}, "c");
//    SDD s0(3, {1}, SDD(2, {33}, SDD(1, {1}, SDD(0, {10}, one))));
//    SDD s1(3, {1}, SDD(2, {33}, SDD(1, {1}, SDD(0, {2}, one))));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"a", "d", "b", "c"});
//    hom h = Expression(o, evaluator{ast1}, {"a", "b"}, "c");
//    SDD s0 = SDD(3, {1}, SDD(2, {33}, SDD(1, {1}, SDD(0, {10}, one))))
//           + SDD(3, {2}, SDD(2, {42}, SDD(1, {2}, SDD(0, {20}, one))));
//    SDD s1 = SDD(3, {1}, SDD(2, {33}, SDD(1, {1}, SDD(0, {2}, one))))
//           + SDD(3, {2}, SDD(2, {42}, SDD(1, {2}, SDD(0, {4}, one))));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//
//  // more complex paths...
//  {
//    order o(order_builder {"c", "a", "b"});
//    hom h = Expression(o, evaluator(ast1), {"a", "b"}, "c");
//    SDD s0 = SDD(2, {0}, SDD(1, {1}, SDD(0, {2}, one)))
//           + SDD(2, {0}, SDD(1, {2}, SDD(0, {1}, one)));
//    SDD s1 = SDD(2, {3}, SDD(1, {1}, SDD(0, {2}, one)))
//           + SDD(2, {3}, SDD(1, {2}, SDD(0, {1}, one)));
//    ASSERT_EQ(s1, h(o, s0));
//  }

  {
    order o(order_builder {"c", "a", "b"});
    hom h = Expression(o, evaluator(ast1), {"a", "b"}, "c");
    SDD s0 = SDD(2, {0}, SDD(1, {1}, SDD(0, {2}, one)))
           + SDD(2, {0}, SDD(1, {3}, SDD(0, {4}, one)));
    SDD s1 = SDD(2, {3}, SDD(1, {1}, SDD(0, {2}, one)))
           + SDD(2, {7}, SDD(1, {3}, SDD(0, {4}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

//TEST_F(hom_expression_test, flat_self_affectation)
//{
//  {
//    order o(order_builder {"a", "b"});
//    hom h = Expression(o, evaluator(ast1), {"a", "b"}, "a");
//    SDD s0(1, {1}, SDD(0, {1}, one));
//    SDD s1(1, {2}, SDD(0, {1}, one));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"a", "b"});
//    hom h = Expression(o, evaluator(ast1), {"a", "b"}, "a");
//    SDD s0 = SDD(1, {1}, SDD(0, {1}, one))
//           + SDD(1, {2}, SDD(0, {2}, one));
//    SDD s1 = SDD(1, {2}, SDD(0, {1}, one))
//           + SDD(1, {4}, SDD(0, {2}, one));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"b", "a"});
//    hom h = Expression(o, evaluator(ast1), {"a", "b"}, "a");
//    SDD s0(1, {1}, SDD(0, {1}, one));
//    SDD s1(1, {1}, SDD(0, {2}, one));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"b", "a"});
//    hom h = Expression(o, evaluator(ast1), {"a", "b"}, "a");
//    SDD s0 = SDD(1, {1}, SDD(0, {1}, one))
//           + SDD(1, {2}, SDD(0, {2}, one));
//    SDD s1 = SDD(1, {1}, SDD(0, {2}, one))
//           + SDD(1, {2}, SDD(0, {4}, one));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"c", "a", "b"});
//    hom h = Expression(o, evaluator{ast1}, {"a", "b"}, "a");
//    SDD s0(2, {33}, SDD(1, {1}, SDD(0, {1}, one)));
//    SDD s1(2, {33}, SDD(1, {2}, SDD(0, {1}, one)));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"c", "a", "b"});
//    hom h = Expression(o, evaluator{ast1}, {"a", "b"}, "a");
//    SDD s0 = SDD(2, {33}, SDD(1, {1}, SDD(0, {1}, one)))
//           + SDD(2, {42}, SDD(1, {2}, SDD(0, {2}, one)));
//    SDD s1 = SDD(2, {33}, SDD(1, {2}, SDD(0, {1}, one)))
//           + SDD(2, {42}, SDD(1, {4}, SDD(0, {2}, one)));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//}
//
///*------------------------------------------------------------------------------------------------*/
//
//TEST_F(hom_expression_test, flat_multiple_values)
//{
//  {
//    order o(order_builder {"c", "a", "b"});
//    hom h = Expression(o, evaluator(ast1), {"a", "b"}, "c");
//    SDD s0(2, {42}, SDD(1, {1,2}, SDD(0, {1,2}, one)));
//    SDD s1(2, {2,3,4}, SDD(1, {1,2}, SDD(0, {1,2}, one)));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"c", "a", "b"});
//    hom h = Expression(o, evaluator(ast1), {"a", "b"}, "c");
//    SDD s0 = SDD(2, {42}, SDD(1, {0,1}, SDD(0, {0,1}, one)))
//           + SDD(2, {44}, SDD(1, {2,3}, SDD(0, {4,5}, one)));
//    SDD s1 = SDD(2, {0, 1, 2}, SDD(1, {0,1}, SDD(0, {0, 1}, one)))
//           + SDD(2, {6, 7, 8}, SDD(1, {2,3}, SDD(0, {4,5}, one)));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"a", "b", "c"});
//    hom h = Expression(o, evaluator{ast1}, {"a", "b"}, "c");
//    SDD s0(2, {0,1}, SDD(1, {2,3}, SDD(0, {42}, one)));
//    SDD s1(2, {0,1}, SDD(1, {2,3}, SDD(0, {2,3,4}, one)));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"a", "b", "c"});
//    hom h = Expression(o, evaluator{ast1}, {"a", "b"}, "c");
//    SDD s0 = SDD(2, {0,1}, SDD(1, {2,3}, SDD(0, {33}, one)))
//           + SDD(2, {4,5}, SDD(1, {6,7}, SDD(0, {42}, one)));
//    SDD s1 = SDD(2, {0,1}, SDD(1, {2,3}, SDD(0, {2,3,4}, one)))
//           + SDD(2, {4,5}, SDD(1, {6,7}, SDD(0, {10, 11, 12}, one)));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"a", "b", "c", "d"});
//    hom h = Expression(o, evaluator{ast1}, {"a", "b"}, "c");
//    SDD s0(3, {0, 1}, SDD(2, {0, 1}, SDD(1, {0}, SDD(0, {63}, one))));
//    SDD s1(3, {0, 1}, SDD(2, {0, 1}, SDD(1, {0, 1, 2}, SDD(0, {63}, one))));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"a", "b", "c", "d"});
//    hom h = Expression(o, evaluator{ast1}, {"a", "b"}, "c");
//    SDD s0 = SDD(3, {0, 1}, SDD(2, {0, 1}, SDD(1, {0}, SDD(0, {33}, one))))
//           + SDD(3, {2, 3}, SDD(2, {2, 3}, SDD(1, {1}, SDD(0, {42}, one))));
//    SDD s1 = SDD(3, {0, 1}, SDD(2, {0, 1}, SDD(1, {0, 1, 2}, SDD(0, {33}, one))))
//           + SDD(3, {2, 3}, SDD(2, {2, 3}, SDD(1, {4, 5, 6}, SDD(0, {42}, one))));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//}
//
///*------------------------------------------------------------------------------------------------*/
//
//TEST_F(hom_expression_test, flat_self_affectation_multiple_values)
//{
//  {
//    order o(order_builder {"a", "b"});
//    hom h = Expression(o, evaluator(ast1), {"a", "b"}, "a");
//    SDD s0(1, {1, 2}, SDD(0, {3, 4}, one));
//    SDD s1(1, {4, 5, 6}, SDD(0, {3, 4}, one));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//  {
//    order o(order_builder {"a", "b"});
//    hom h = Expression(o, evaluator(ast1), {"a", "b"}, "a");
//    SDD s0 = SDD(1, {1, 2}, SDD(0, {1, 2}, one))
//           + SDD(1, {3, 4}, SDD(0, {3, 4}, one));
//    SDD s1 = SDD(1, {2, 3, 4}, SDD(0, {1, 2}, one))
//           + SDD(1, {6, 7, 8}, SDD(0, {3, 4}, one));
//    ASSERT_EQ(s1, h(o, s0));
//  }
//}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_expression_test, hierarchic)
{
  {
    order o(order_builder().add("y", order_builder{"b", "c"}).add("x", order_builder{"a"}));
    std::cout << o << std::endl;
    hom h = Expression(o, evaluator(ast1), {"a", "b"}, "c");
    SDD s0(1, SDD(0, {2}, one), SDD(0, SDD(1, {1}, SDD(0, {0}, one)), one));
    SDD s1(1, SDD(0, {2}, one), SDD(0, SDD(1, {1}, SDD(0, {3}, one)), one));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
//    order o(order_builder {"a", "b"});
//    hom h = Expression(o, evaluator(ast1), {"a", "b"}, "a");
//    SDD s0 = SDD(1, {1, 2}, SDD(0, {1, 2}, one))
//           + SDD(1, {3, 4}, SDD(0, {3, 4}, one));
//    SDD s1 = SDD(1, {2, 3, 4}, SDD(0, {1, 2}, one))
//           + SDD(1, {6, 7, 8}, SDD(0, {3, 4}, one));
//    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
