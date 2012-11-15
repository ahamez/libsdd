#include <forward_list>
#include <iostream>
#include <memory>

#include <boost/container/flat_map.hpp>
#include <boost/variant.hpp>

#include "gtest/gtest.h"

#include "tests/hom/common.hh"

/*------------------------------------------------------------------------------------------------*/

typedef sdd::conf::conf0 conf;
typedef conf::Identifier identifier_type;
typedef conf::Values bitset_type;
typedef sdd::SDD<conf> SDD;
typedef sdd::homomorphism<conf> hom;

const SDD one = sdd::one<conf>();

/*------------------------------------------------------------------------------------------------*/

enum class binop {add, sub, mul};

struct binary_operation;
struct operand;

typedef boost::variant< boost::recursive_wrapper<binary_operation>, operand> ast_type;


struct operand
{
  const identifier_type identifier;
  mutable bitset_type values;

  operand(const identifier_type& id)
    : identifier(id)
    , values()
  {
  }

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
  const ast_type lhs;
  const ast_type rhs;

  binary_operation(enum binop op, const ast_type& l, const ast_type& r)
    : operation(op)
    , lhs(l)
    , rhs(r)
  {
  }

  bool
  operator==(const binary_operation& other)
  const noexcept
  {
    return operation == other.operation and lhs == other.lhs and rhs == other.rhs;
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
    {
    }

    void
    operator()(const binary_operation& bop)
    const
    {
      apply_visitor(*this, bop.lhs);
      apply_visitor(*this, bop.rhs);
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
  }

  bitset_type
  operator()(const binary_operation& op)
  const noexcept
  {
    bitset_type res;
    const bitset_type lhs = apply_visitor(*this, op.lhs);
    const bitset_type rhs = apply_visitor(*this, op.rhs);

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
  std::shared_ptr<indexed_ast> ast_;

  evaluator(const std::shared_ptr<indexed_ast>& ast)
    : ast_(ast)
  {
  }

  void
  update(const std::string& identifier, const bitset_type& val)
  {
    ast_->update(identifier, val);
  }

  bitset_type
  evaluate()
  {
    return apply_visitor(evaluation_visitor(), ast_->ast);
  }

  bool
  operator==(const evaluator& other)
  const noexcept
  {
    // clumsy equality of pointers as there is no unification of ASTs...
    return ast_ == other.ast_;
  }
};

struct print_visitor
  : public boost::static_visitor<void>
{
  std::ostream& os;

  print_visitor(std::ostream& o)
    : os(o)
  {
  }

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
    apply_visitor(*this, op.lhs);
    print_op(op.operation);
    apply_visitor(*this, op.rhs);
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
  return os << e.ast_;
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
      return std::hash<bitset_type>()(op.values);
    }

    std::size_t
    hash_op(enum binop op)
    const noexcept
    {
      switch (op)
      {
        case binop::add : return 0;
        case binop::sub : return 1;
        case binop::mul : return 2;
      }
    }

    std::size_t
    operator()(const binary_operation& op)
    const noexcept
    {
      std::size_t seed = hash_op(op.operation);
      sdd::util::hash_combine(seed, apply_visitor(*this, op.lhs));
      sdd::util::hash_combine(seed, apply_visitor(*this, op.rhs));
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
    return std::hash<indexed_ast>()(*e.ast_);
  }
};

} // namespace std

/*------------------------------------------------------------------------------------------------*/

struct hom_expression_test
  : public testing::Test
{
  std::shared_ptr<indexed_ast> ast1;

  hom_expression_test()
    : ast1(mk_ast1())
  {
  }

  std::shared_ptr<indexed_ast>
  mk_ast1()
  {
    operand a("a");
    operand b("b");
    binary_operation op(binop::add, a, b);
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
    ASSERT_THROW(Expression(o, evaluator(ast1), l.begin(), l.begin(), "a"), std::invalid_argument);
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_expression_test, flat_evaluation)
{
  {
    const auto l = {"a", "b"};
    order o(order_builder {"c", "a", "b"});
    hom h = Expression(o, evaluator(ast1), l.begin(), l.end(), "c");
    SDD s0(2, {0}, SDD(1, {1}, SDD(0, {1}, one)));
    SDD s1(2, {2}, SDD(1, {1}, SDD(0, {1}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
