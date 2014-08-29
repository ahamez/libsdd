#ifndef _SDD_TESTS_HOM_EXPRESSION_HH_
#define _SDD_TESTS_HOM_EXPRESSION_HH_

#include <forward_list>
#include <iostream>
#include <memory>
#include <type_traits> // underlying_type

#include <boost/container/flat_map.hpp>
#include <boost/variant.hpp>

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

  using index_type = boost::container::flat_map<identifier_type, std::forward_list<values_type*>>;
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
    : ast(std::move(a)), index()
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
      using type = std::underlying_type<enum binop>::type;
      return std::hash<type>()(static_cast<type>(op));
    }

    std::size_t
    operator()(const binary_operation<C>& op)
    const noexcept
    {
      using namespace sdd::hash;
      return seed(op.operation) (val(apply_visitor(*this, *op.lhs)))
                                (val(apply_visitor(*this, *op.rhs)));
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
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  const std::shared_ptr<indexed_ast<C>> ast1;
  const std::shared_ptr<indexed_ast<C>> ast2;
  const std::shared_ptr<indexed_ast<C>> ast3;

  hom_expression_test()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::id<C>())
    , ast1(mk_ast1())
    , ast2(mk_ast2())
    , ast3(mk_ast3())
  {}

  std::shared_ptr<indexed_ast<C>>
  mk_ast1()
  {
    binary_operation<C> op{binop::add, operand<C>{"a"}, operand<C>{"b"}};
    return std::make_shared<indexed_ast<C>>(std::move(op));
  }

  std::shared_ptr<indexed_ast<C>>
  mk_ast2()
  {
    binary_operation<C> op { binop::add
                           , operand<C>{"c"}
                           , binary_operation<C>{binop::add, operand<C>{"a"}, operand<C>{"b"}}};
    return std::make_shared<indexed_ast<C>>(std::move(op));
  }

  std::shared_ptr<indexed_ast<C>>
  mk_ast3()
  {
    binary_operation<C> a_plus_b {binop::add, operand<C>{"a"}, operand<C>{"b"}};
    binary_operation<C> c_plus_d {binop::add, operand<C>{"c"}, operand<C>{"d"}};
    binary_operation<C> op { binop::add, a_plus_b, c_plus_d};
    return std::make_shared<indexed_ast<C>>(std::move(op));
  }
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_expression_test, configurations);
#include "tests/macros.hh"
#define ast1 this->ast1
#define ast2 this->ast2
#define ast3 this->ast3

/*------------------------------------------------------------------------------------------------*/

#endif // _SDD_TESTS_HOM_EXPRESSION_HH_
