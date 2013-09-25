#ifndef _SDD_HOM_EXPRESSION_HH_
#define _SDD_HOM_EXPRESSION_HH_

#include <algorithm> // any_of, copy, find
#include <cassert>
#include <iterator>  // back_insert
#include <iosfwd>
#include <initializer_list>
#include <memory>    // make_shared, shared_ptr, unique_ptr
#include <vector>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation_error.hh"
#include "sdd/hom/expression/evaluator.hh"
#include "sdd/hom/expression/expression.hh"
#include "sdd/hom/expression/simple.hh"
#include "sdd/hom/expression/stacks.hh"
#include "sdd/hom/identity.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Expression homomorphism.
template <typename C>
class expression
{
public:

  /// @brief A variable type.
  using variable_type = typename C::Variable;

  /// @brief The absolute position of an identifier in an order.
  using position_type = typename order<C>::position_type;

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief The type of a set of variables.
  using positions_type = std::vector<position_type>;

private:

  /// @brief Pointer to the evaluator provided by the user.
  const std::unique_ptr<expr::evaluator_base<C>> eval_ptr_;

  /// @brief The set of the expression's variables.
  const positions_type positions_;

  /// @brief The target of the assignment.
  const position_type target_;

  /// @brief An iterator on a set of identifiers.
  using positions_iterator = typename positions_type::const_iterator;

public:

  /// @brief Constructor.
  expression( std::unique_ptr<expr::evaluator_base<C>>&& e_ptr, positions_type&& positions
            , position_type target)
    : eval_ptr_(std::move(e_ptr)), positions_(std::move(positions)), target_(target)
  {}

  /// @brief Skip variable predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return o.position() != target_ and o.position() != positions_.front()
       and not o.contains(o.position(), positions_.front())
       and not o.contains(o.position(), target_);
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
    expr::expression_pre<C> eval {cxt, target_, *eval_ptr_};
    return visit(eval, sdd, o, app, res, positions_.cbegin(), positions_.cend());
  }

  /// @brief Get the user's evaluator.
  const expr::evaluator_base<C>&
  evaluator()
  const noexcept
  {
    return *eval_ptr_;
  }

  /// @brief Get the set of variables.
  const positions_type&
  identifiers()
  const noexcept
  {
    return positions_;
  }

  /// @brief Get the target.
  position_type
  target()
  const noexcept
  {
    return target_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Equality of two expression homomorphisms.
/// @related expression
template <typename C>
inline
bool
operator==(const expression<C>& lhs, const expression<C>& rhs)
noexcept
{
  return lhs.target() == rhs.target() and lhs.identifiers() == rhs.identifiers()
     and lhs.evaluator() == rhs.evaluator();
}

/// @internal
/// @related expression
template <typename C>
std::ostream&
operator<<(std::ostream& os, const expression<C>& e)
{
  os << "Expression(" << e.target() << " = ";
  e.evaluator().print(os);
  return os << ")";
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Expression homomorphism.
template <typename C>
class simple_expression
{
public:

  /// @brief A variable type.
  using variable_type = typename C::Variable;

  /// @brief The absolute position of an identifier in an order.
  using position_type = typename order<C>::position_type;

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief The type of a set of variables.
  using positions_type = std::vector<position_type>;

private:

  /// @brief Pointer to the evaluator provided by the user.
  const std::unique_ptr<expr::evaluator_base<C>> eval_ptr_;

  /// @brief The set of the expression's variables.
  const positions_type positions_;

  /// @brief The target of the assignment.
  const position_type target_;

  /// @brief An iterator on a set of identifiers.
  using positions_iterator = typename positions_type::const_iterator;

public:

  /// @brief Constructor.
  simple_expression( std::unique_ptr<expr::evaluator_base<C>>&& e_ptr
                   , positions_type&& positions, position_type target)
    : eval_ptr_(std::move(e_ptr)), positions_(std::move(positions)), target_(target)
  {}

  /// @brief Skip variable predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return o.position() != target_ and o.position() != positions_.front()
       and not o.contains(o.position(), positions_.front())
       and not o.contains(o.position(), target_);
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
    expr::simple<C> eval {cxt, target_, *eval_ptr_};
    return visit(eval, sdd, o, app, res, positions_.cbegin(), positions_.cend());
  }

  /// @brief Get the user's evaluator.
  const expr::evaluator_base<C>&
  evaluator()
  const noexcept
  {
    return *eval_ptr_;
  }

  /// @brief Get the set of variables.
  const positions_type&
  identifiers()
  const noexcept
  {
    return positions_;
  }

  /// @brief Get the target.
  position_type
  target()
  const noexcept
  {
    return target_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Equality of two expression homomorphisms.
/// @related expression
template <typename C>
inline
bool
operator==(const simple_expression<C>& lhs, const simple_expression<C>& rhs)
noexcept
{
  return lhs.target() == rhs.target() and lhs.identifiers() == rhs.identifiers()
     and lhs.evaluator() == rhs.evaluator();
}

/// @internal
/// @related expression
template <typename C>
std::ostream&
operator<<(std::ostream& os, const simple_expression<C>& e)
{
  os << "SimpleExpression(" << e.target() << " = ";
  e.evaluator().print(os);
  return os << ")";
}

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the Expression homomorphism.
/// @related homomorphism
/// @todo How to handle the dumb case where there is only one identifier, which is also the target?
///
/// Elements of [begin, end) must be unique.
template <typename C, typename Evaluator, typename InputIterator>
homomorphism<C>
Expression( const order<C>& o, const Evaluator& u, InputIterator begin, InputIterator end
          , const typename C::Identifier& target)
{
  using identifier_type = typename C::Identifier;
  using derived_type = hom::expr::evaluator_derived<C, Evaluator>;
  using positions_type = std::vector<typename order<C>::position_type>;

  if (std::distance(begin, end) == 0)
  {
    return Id<C>();
  }

  const auto target_pos = o.identifier_position(target);

  positions_type positions;
  positions.reserve(std::distance(begin, end));
  std::transform( begin, end, std::back_inserter(positions)
                , [&](const identifier_type& id){return o.identifier_position(id);});
  std::sort(positions.begin(), positions.end());


  std::unique_ptr<derived_type> evaluator_ptr(new derived_type(u));

  const auto last_position = positions.back();
  if (target_pos < last_position)
  {
    return homomorphism<C>::create( mem::construct<hom::expression<C>>()
                                  , std::move(evaluator_ptr), std::move(positions), target_pos);
  }
  else
  {
    // The target is below all identifiers, it's a much simpler case to handle
    return homomorphism<C>::create( mem::construct<hom::simple_expression<C>>()
                                  , std::move(evaluator_ptr), std::move(positions), target_pos);
  }
}

/// @brief Create the Expression homomorphism.
/// @related homomorphism
///
/// Elements of ids must be unique.
template <typename C, typename Evaluator>
homomorphism<C>
Expression( const order<C>& o, const Evaluator& u, std::initializer_list<typename C::Identifier> ids
          , const typename C::Identifier& target)
{
  return Expression(o, u, ids.cbegin(), ids.cend(), target);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::expression.
template <typename C>
struct hash<sdd::hom::expression<C>>
{
  std::size_t
  operator()(const sdd::hom::expression<C>& e)
  const
  {
    std::size_t seed = e.evaluator().hash();
    for (const auto& v : e.identifiers())
    {
      sdd::util::hash_combine(seed, v);
    }
    sdd::util::hash_combine(seed, e.target());
    return seed;
  }
};

/// @internal
/// @brief Hash specialization for sdd::hom::simple_expression.
template <typename C>
struct hash<sdd::hom::simple_expression<C>>
{
  std::size_t
  operator()(const sdd::hom::simple_expression<C>& e)
  const noexcept
  {
    std::size_t seed = e.evaluator().hash();
    for (const auto& v : e.identifiers())
    {
      sdd::util::hash_combine(seed, v);
    }
    sdd::util::hash_combine(seed, e.target());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_EXPRESSION_HH_
