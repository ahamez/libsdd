#include <forward_list>
#include <iostream>

#include <boost/container/flat_map.hpp>
#include <boost/variant.hpp>

#include "sdd/sdd.hh"

/*-------------------------------------------------------------------------------------------*/

typedef sdd::conf::conf0 conf;
typedef conf::Variable variable_type;
typedef conf::Values bitset_type;
typedef sdd::SDD<conf> SDD;
typedef sdd::hom::homomorphism<conf> hom;

using sdd::hom::Cons;
using sdd::hom::Expression;
using sdd::hom::Inductive;

const SDD one = sdd::one<conf>();
const hom id = sdd::hom::Id<conf>();

/*-------------------------------------------------------------------------------------------*/

enum class binop {add, sub, mul};

struct binary_operation;
struct operand;

typedef boost::variant< boost::recursive_wrapper<binary_operation>
                      , operand>
        ast_type;


struct operand
{
  const variable_type variable_;
  mutable bitset_type values_;

  bool
  operator==(const operand& other)
  const noexcept
  {
    return variable_ == other.variable_ and values_ == other.values_;
  }
};

struct binary_operation
{
  const enum binop operation_;
  const ast_type lhs_;
  const ast_type rhs_;

  bool
  operator==(const binary_operation& other)
  const noexcept
  {
    return operation_ == other.operation_ and lhs_ == other.lhs_ and rhs_ == other.rhs_;
  }
};

/*-------------------------------------------------------------------------------------------*/

struct indexed_ast
{
  ast_type ast_;

  typedef boost::container::flat_map< variable_type
                                    , std::forward_list<bitset_type*>>
          index_type;
  index_type index_;

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
      apply_visitor(*this, bop.lhs_);
      apply_visitor(*this, bop.rhs_);
    }

    void
    operator()(const operand& op)
    const
    {
      auto lb = index_.lower_bound(op.variable_);
      if (lb != index_.end() and not index_.key_comp()(op.variable_, lb->first))
      {
        lb->second.emplace_front(&op.values_);
      }
      else
      {
        auto ins = index_.emplace_hint(lb, op.variable_, std::forward_list<bitset_type*>());
        ins->second.emplace_front(&op.values_);
      }
    }
  };

  indexed_ast(ast_type&& ast)
    : ast_(std::move(ast))
    , index_()
  {
    indexer idx(index_);
    apply_visitor(idx, ast);
  }

  bool
  operator==(const indexed_ast& other)
  const noexcept
  {
    return index_ == other.index_;
  }
};

/*-------------------------------------------------------------------------------------------*/

struct evaluator
  : public boost::static_visitor<bitset_type>
{
  bitset_type
  operator()(const operand& op)
  const noexcept
  {
    return op.values_;
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
    const bitset_type lhs = apply_visitor(*this, op.lhs_);
    const bitset_type rhs = apply_visitor(*this, op.rhs_);

    for (std::size_t i = 0; i < 64; ++i)
    {
      if (lhs.content().test(i))
      {
        for (std::size_t j = 0; j < 64; ++i)
        {
          if (rhs.content().test(i))
          {
            res.insert(apply(op.operation_, i, j));
          }
        }
      }
    }
    return res;
  }
};

bitset_type
evaluate(const indexed_ast& ast)
{
  return apply_visitor(evaluator(), ast.ast_);
}

/*-------------------------------------------------------------------------------------------*/

//namespace std {
//
//template <>
//struct hash<indexed_ast>
//{
//  std::size_t
//  operator()(const indexed_ast& idx)
//  const noexcept
//  {
//    std::size_t seed = 0;
////    ???
//    return seed;
//  }
//};


//} // namespace std

/*-------------------------------------------------------------------------------------------*/

int
main()
{

  return 0;
};
