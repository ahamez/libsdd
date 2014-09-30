#pragma once

#include <algorithm> // any_of, copy, find
#include <iterator>  // back_insert
#include <iosfwd>
#include <initializer_list>
#include <memory>    // make_shared, shared_ptr, unique_ptr
#include <vector>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/expression/evaluator.hh"
#include "sdd/hom/expression/expression.hh"
#include "sdd/hom/expression/simple.hh"
#include "sdd/hom/expression/stacks.hh"
#include "sdd/hom/identity.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief expression homomorphism.
template <typename C>
struct _expression
{
  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief Pointer to the evaluator provided by the user.
  const std::unique_ptr<expr::evaluator_base<C>> eval_ptr;

  /// @brief The set of the expression's variables.
  const order_positions_type positions;

  /// @brief The target of the assignment.
  const order_position_type target;

  /// @brief Constructor.
  _expression( std::unique_ptr<expr::evaluator_base<C>>&& e, order_positions_type&& pos
             , order_position_type t)
    : eval_ptr(std::move(e)), positions(std::move(pos)), target(t)
  {}

  /// @brief Skip variable predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return o.position() != target and o.position() != positions.front()
       and not o.contains(o.position(), positions.front())
       and not o.contains(o.position(), target);
  }

  /// @brief Selector predicate.
  constexpr bool
  selector()
  const noexcept
  {
    return false;
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& sdd)
  const
  {
    std::shared_ptr<expr::app_stack<C>> app = nullptr;
    std::shared_ptr<expr::res_stack<C>> res = nullptr;
    expr::expression_pre<C> eval {cxt, target, *eval_ptr};
    return visit(eval, sdd, o, app, res, positions.cbegin(), positions.cend());
  }

  /// @brief Get the user's evaluator.
  const expr::evaluator_base<C>&
  evaluator()
  const noexcept
  {
    return *eval_ptr;
  }

  friend
  bool
  operator==(const _expression& lhs, const _expression& rhs)
  noexcept
  {
    return lhs.target == rhs.target and *lhs.eval_ptr == *rhs.eval_ptr
       and lhs.positions == rhs.positions;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const _expression& e)
  {
    os << "expression(" << e.target << " = ";
    e.evaluator().print(os);
    return os << ")";
  }

};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Simple expression homomorphism.
template <typename C>
struct _simple_expression
{
  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief Pointer to the evaluator provided by the user.
  const std::unique_ptr<expr::evaluator_base<C>> eval_ptr;

  /// @brief The set of the expression's variables.
  const order_positions_type positions;

  /// @brief The target of the assignment.
  const order_position_type target;

  /// @brief Constructor.
  _simple_expression( std::unique_ptr<expr::evaluator_base<C>>&& e
                    , order_positions_type&& pos, order_position_type t)
    : eval_ptr(std::move(e)), positions(std::move(pos)), target(t)
  {}

  /// @brief Skip variable predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return o.position() != target and o.position() != positions.front()
       and not o.contains(o.position(), positions.front())
       and not o.contains(o.position(), target);
  }

  /// @brief Selector predicate.
  constexpr bool
  selector()
  const noexcept
  {
    return false;
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& sdd)
  const
  {
    std::shared_ptr<expr::app_stack<C>> app = nullptr;
    std::shared_ptr<expr::res_stack<C>> res = nullptr;
    expr::simple<C> eval {cxt, target, *eval_ptr};
    return visit(eval, sdd, o, app, res, positions.cbegin(), positions.cend());
  }

  /// @brief Get the user's evaluator.
  const expr::evaluator_base<C>&
  evaluator()
  const noexcept
  {
    return *eval_ptr;
  }

  friend
  bool
  operator==(const _simple_expression& lhs, const _simple_expression& rhs)
  noexcept
  {
    return lhs.target == rhs.target and *lhs.eval_ptr == *rhs.eval_ptr
       and lhs.positions == rhs.positions;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const _simple_expression& e)
  {
    os << "SimpleExpr(" << e.target << " = ";
    e.evaluator().print(os);
    return os << ")";
  }

};

/*------------------------------------------------------------------------------------------------*/

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the expression homomorphism.
/// @related homomorphism
/// @todo How to handle the dumb case where there is only one identifier, which is also the target?
///
/// Elements of [begin, end) must be unique.
template <typename C, typename Evaluator, typename InputIterator>
homomorphism<C>
expression( const order<C>& o, const Evaluator& u, InputIterator begin, InputIterator end
          , const typename C::Identifier& target)
{
  using identifier_type = typename C::Identifier;
  using derived_type = hom::expr::evaluator_derived<C, Evaluator>;

  const std::size_t size = std::distance(begin, end);

  if (size == 0)
  {
    return id<C>();
  }

  const auto target_pos = o.node(target).position();

  order_positions_type positions;
  positions.reserve(size);
  std::transform( begin, end, std::back_inserter(positions)
                , [&](const identifier_type& id){return o.node(id).position();});
  std::sort(positions.begin(), positions.end());


  auto evaluator_ptr = std::make_unique<derived_type>(u);

  const auto last_position = positions.back();
  if (target_pos < last_position)
  {
    return hom::make<C, hom::_expression<C>>
      (std::move(evaluator_ptr), std::move(positions), target_pos);
  }
  else
  {
    // The target is below all operands, it's a much simpler case to handle
    return hom::make<C, hom::_simple_expression<C>>
      (std::move(evaluator_ptr), std::move(positions), target_pos);
  }
}

/// @brief Create the expression homomorphism.
/// @related homomorphism
///
/// Elements of ids must be unique.
template <typename C, typename Evaluator>
homomorphism<C>
expression( const order<C>& o, const Evaluator& u, std::initializer_list<typename C::Identifier> ids
          , const typename C::Identifier& target)
{
  return expression(o, u, ids.cbegin(), ids.cend(), target);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::_expression.
template <typename C>
struct hash<sdd::hom::_expression<C>>
{
  std::size_t
  operator()(const sdd::hom::_expression<C>& e)
  const
  {
    using namespace sdd::hash;
    return seed (e.evaluator().hash()) (range(e.positions)) (val(e.target));
  }
};

/// @internal
/// @brief Hash specialization for sdd::hom::s_imple_expression.
template <typename C>
struct hash<sdd::hom::_simple_expression<C>>
{
  std::size_t
  operator()(const sdd::hom::_simple_expression<C>& e)
  const noexcept
  {
    using namespace sdd::hash;
    return seed (e.evaluator().hash()) (range(e.positions)) (val(e.target));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
