#include <forward_list>
#include <iostream>
#include <memory>
#include <type_traits> // underlying_type

#include <boost/variant.hpp>

#include "gtest/gtest.h"

#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"
#include "sdd/hom/rewrite.hh"
#include "sdd/manager.hh"
#include "sdd/order/order.hh"
#include "sdd/util/boost_flat_map_no_warnings.hh"

#include "tests/configuration.hh"
#include "tests/hom/common.hh"
#include "tests/hom/common_inductives.hh"

/*------------------------------------------------------------------------------------------------*/

enum class binop {add, sub, mul};

template <typename C>
struct binary_operation;

template <typename C>
struct operand;

template <typename C>
using ast_type = boost::variant<binary_operation<C>, operand<C>>;

template <typename C>
struct operand
{
  using identifier_type = typename C::Identifier;
  using values_type = typename C::Values;

  const identifier_type identifier;
  mutable values_type values;

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

template <typename C>
struct binary_operation
{
  const enum binop operation;
  const std::unique_ptr<ast_type<C>> lhs;
  const std::unique_ptr<ast_type<C>> rhs;

  binary_operation(enum binop op, const ast_type<C>& l, const ast_type<C>& r)
    : operation(op)
    , lhs(new ast_type<C>(l))
    , rhs(new ast_type<C>(r))
  {}

  // Deep-copy.
  binary_operation(const binary_operation& other)
    : operation(other.operation)
    , lhs(new ast_type<C>(*other.lhs))
    , rhs(new ast_type<C>(*other.rhs))
  {}

  bool
  operator==(const binary_operation& other)
  const noexcept
  {
    return operation == other.operation and *lhs == *other.lhs and *rhs == *other.rhs;
  }
};

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct indexed_ast
{
  using identifier_type = typename C::Identifier;
  using values_type = typename C::Values;

  ast_type<C> ast;

  typedef boost::container::flat_map<identifier_type, std::forward_list<values_type*>> index_type;
  index_type index;

  struct indexer
    : public boost::static_visitor<void>
  {
    index_type& index_;

    indexer(index_type& idx)
      : index_(idx)
    {}

    void
    operator()(const binary_operation<C>& bop)
    const
    {
      apply_visitor(*this, *bop.lhs);
      apply_visitor(*this, *bop.rhs);
    }

    void
    operator()(const operand<C>& op)
    const
    {
      auto lb = index_.lower_bound(op.identifier);
      if (lb != index_.end() and not index_.key_comp()(op.identifier, lb->first))
      {
        lb->second.emplace_front(&(op.values));
      }
      else
      {
        auto ins = index_.emplace_hint(lb, op.identifier, std::forward_list<values_type*>());
        ins->second.emplace_front(&(op.values));
      }
    }
  };

  indexed_ast(ast_type<C>&& a)
    : ast(std::move(a))
    , index()
  {
    indexer idx(index);
    apply_visitor(idx, ast);
  }

  void
  update(const identifier_type& id, const values_type& val)
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

template <typename C>
struct evaluation_visitor
  : public boost::static_visitor<typename C::Values>
{
  using identifier_type = typename C::Identifier;
  using values_type = typename C::Values;

  values_type
  operator()(const operand<C>& op)
  const noexcept
  {
    return op.values;
  }

  unsigned int
  apply(enum binop op, unsigned int lhs, unsigned int rhs)
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

  values_type
  operator()(const binary_operation<C>& op)
  const noexcept
  {
    values_type res;
    const values_type lhs = apply_visitor(*this, *op.lhs);
    const values_type rhs = apply_visitor(*this, *op.rhs);

    for (unsigned int i = 0; i < 64; ++i)
    {
      if (find_in_values(lhs, i))
      {
        for (unsigned int j = 0; j < 64; ++j)
        {
          if (find_in_values(rhs, j))
          {
            res.insert(apply(op.operation, i, j));
          }
        }
      }
    }
    return res;
  }

  static bool
  find_in_values(const values_type& values, unsigned int val)
  {
    return find_in_values_impl(values, val, 0);
  }

  template <typename T>
  static auto
  find_in_values_impl(const T& values, unsigned int val, int)
  -> decltype(values.test(val))
  {
    return values.test(val);
  }

  template <typename T>
  static auto
  find_in_values_impl(const T& values, unsigned int val, long)
  -> decltype(bool())
  {
    return values.find(val) != values.end();
  }
};

template <typename C>
struct evaluator
{
  using identifier_type = typename C::Identifier;
  using values_type = typename C::Values;

  std::shared_ptr<indexed_ast<C>> ast_ptr_;

  evaluator(const std::shared_ptr<indexed_ast<C>>& ast)
    : ast_ptr_(ast)
  {}

  evaluator(const evaluator<C>& other)
    : ast_ptr_(other.ast_ptr_)
  {}

  // Called by the library.
  void
  update(const identifier_type& identifier, const values_type& val)
  {
    ast_ptr_->update(identifier, val);
  }

  // Called by the library.
  values_type
  evaluate()
  {
    return apply_visitor(evaluation_visitor<C>(), ast_ptr_->ast);
  }

  bool
  operator==(const evaluator& other)
  const noexcept
  {
    return ast_ptr_->ast == other.ast_ptr_->ast;
  }
};

template <typename C>
struct print_visitor
  : public boost::static_visitor<void>
{
  std::ostream& os;

  print_visitor(std::ostream& o)
    : os(o)
  {}

  void
  operator()(const operand<C>& op)
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
  operator()(const binary_operation<C>& op)
  const noexcept
  {
    os << "(";
    apply_visitor(*this, *op.lhs);
    print_op(op.operation);
    apply_visitor(*this, *op.rhs);
    os << ")";
  }
};

template <typename C>
std::ostream&
operator<<(std::ostream& os, const indexed_ast<C>& a)
{
  print_visitor<C> v(os);
  apply_visitor(v, a.ast);
  return os;
}

template <typename C>
std::ostream&
operator<<(std::ostream& os, const evaluator<C>& e)
{
  return os << *e.ast_ptr_;
}

/*------------------------------------------------------------------------------------------------*/

namespace std {

template <typename C>
struct hash<indexed_ast<C>>
{
  struct visitor
    : public boost::static_visitor<std::size_t>
  {
    std::size_t
    operator()(const operand<C>& op)
    const noexcept
    {
      return std::hash<typename std::decay<decltype(op.identifier)>::type>()(op.identifier);
    }

    std::size_t
    hash_op(enum binop op)
    const noexcept
    {
      typedef std::underlying_type<enum binop>::type type;
      return std::hash<type>()(static_cast<type>(op));
    }

    std::size_t
    operator()(const binary_operation<C>& op)
    const noexcept
    {
      std::size_t seed = hash_op(op.operation);
      sdd::util::hash_combine(seed, apply_visitor(*this, *op.lhs));
      sdd::util::hash_combine(seed, apply_visitor(*this, *op.rhs));
      return seed;
    }
  };

  std::size_t
  operator()(const indexed_ast<C>& idx)
  const noexcept
  {
    
    return apply_visitor(visitor(), idx.ast);
  }
};

template <typename C>
struct hash<evaluator<C>>
{
  std::size_t
  operator()(const evaluator<C>& e)
  const noexcept
  {
    return std::hash<indexed_ast<C>>()(*e.ast_ptr_);
  }
};

} // namespace std

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct hom_expression_test
  : public testing::Test
{
  typedef C configuration_type;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  const std::shared_ptr<indexed_ast<C>> ast1;

  hom_expression_test()
    : m(sdd::manager<C>::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::Id<C>())
    , ast1(mk_ast1())
  {}

  std::shared_ptr<indexed_ast<C>>
  mk_ast1()
  {
    binary_operation<C> op{binop::add, operand<C>{"a"}, operand<C>{"b"}};
    return std::make_shared<indexed_ast<C>>(std::move(op));
  }
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_expression_test, configurations);
#include "tests/macros.hh"
#define ast1 this->ast1

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_expression_test, construction)
{
  {
    const auto l = {"a", "b"};
    order o(order_builder {"a", "b"});
    const auto h1 = Expression(o, evaluator<conf>(ast1), l.begin(), l.end(), "a");
    const auto h2 = Expression(o, evaluator<conf>(ast1), l.begin(), l.end(), "a");
    ASSERT_EQ(h1, h2);
  }
  {
    const auto l = {"a", "b"};
    order o(order_builder {"a", "b"});
    ASSERT_EQ(Expression(o, evaluator<conf>(ast1), l.begin(), l.begin(), "a"), id);
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_expression_test, simple_flat_evaluation)
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

//  {
//    order o(order_builder {"c", "a", "b"});
//    hom h = Expression(o, evaluator(ast1), {"a", "b"}, "c");
//    SDD s0 = SDD(2, {0}, SDD(1, {1}, SDD(0, {2}, one)))
//           + SDD(2, {0}, SDD(1, {3}, SDD(0, {4}, one)));
//    SDD s1 = SDD(2, {3}, SDD(1, {1}, SDD(0, {2}, one)))
//           + SDD(2, {7}, SDD(1, {3}, SDD(0, {4}, one)));
//    ASSERT_EQ(s1, h(o, s0));
//  }
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

//TEST_F(hom_expression_test, hierarchic)
//{
//  {
//    order o(order_builder().add("y", order_builder{"b", "c"}).add("x", order_builder{"a"}));
//    std::cout << o << std::endl;
//    hom h = Expression(o, evaluator(ast1), {"a", "b"}, "c");
//    SDD s0(1, SDD(0, {2}, one), SDD(0, SDD(1, {1}, SDD(0, {0}, one)), one));
//    SDD s1(1, SDD(0, {2}, one), SDD(0, SDD(1, {1}, SDD(0, {3}, one)), one));
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
