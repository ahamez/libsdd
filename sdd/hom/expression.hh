#ifndef _SDD_HOM_EXPRESSION_HH_
#define _SDD_HOM_EXPRESSION_HH_

#include <algorithm> // any_of, copy, find
#include <cassert>
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

  /// @brief An identifier type.
  using identifier_type = typename C::Identifier;

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief The type of a set of variables.
  using identifiers_type = std::vector<identifier_type>;

private:

  /// @brief Pointer to the evaluator provided by the user.
  const std::unique_ptr<expr::evaluator_base<C>> eval_ptr_;

  /// @brief The set of the expression's variables.
  const identifiers_type identifiers_;

  /// @brief The target of the assignment.
  const identifier_type target_;

  /// @brief An iterator on a set of identifiers.
  using identifiers_iterator = typename identifiers_type::const_iterator;

public:

  /// @brief Constructor.
  expression( std::unique_ptr<expr::evaluator_base<C>>&& e_ptr, identifiers_type&& identifiers
            , const identifier_type& target)
    : eval_ptr_(std::move(e_ptr))
    , identifiers_(std::move(identifiers))
    , target_(target)
  {}

  /// @brief Skip variable predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return o.identifier() != target_ and o.identifier() != *identifiers_.cbegin()
       and not o.contains(o.identifier(), *identifiers_.cbegin())
       and not o.contains(o.identifier(), target_);
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
    return visit(eval, sdd, o, app, res, identifiers_.cbegin(), identifiers_.cend());
  }

  /// @brief Get the user's evaluator.
  const expr::evaluator_base<C>&
  evaluator()
  const noexcept
  {
    return *eval_ptr_;
  }

  /// @brief Get the set of variables.
  const identifiers_type&
  identifiers()
  const noexcept
  {
    return identifiers_;
  }

  /// @brief Get the target.
  const identifier_type&
  target()
  const noexcept
  {
    return target_;
  }
};

/*------------------------------------------------------------------------------------------------*/

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

  /// @brief An identifier type.
  using identifier_type = typename C::Identifier;

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief The type of a set of variables.
  using identifiers_type = std::vector<identifier_type>;

private:

  /// @brief Pointer to the evaluator provided by the user.
  const std::unique_ptr<expr::evaluator_base<C>> eval_ptr_;

  /// @brief The set of the expression's variables.
  const identifiers_type identifiers_;

  /// @brief The target of the assignment.
  const identifier_type target_;

  /// @brief An iterator on a set of identifiers.
  using identifiers_iterator = typename identifiers_type::const_iterator;

public:

  /// @brief Constructor.
  simple_expression( std::unique_ptr<expr::evaluator_base<C>>&& e_ptr
                   , identifiers_type&& identifiers
                   , const identifier_type& target)
    : eval_ptr_(std::move(e_ptr))
    , identifiers_(std::move(identifiers))
    , target_(target)
  {}

  /// @brief Skip variable predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return o.identifier() != target_ and o.identifier() != *identifiers_.cbegin()
       and not o.contains(o.identifier(), *identifiers_.cbegin())
       and not o.contains(o.identifier(), target_);
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
    return visit(eval, sdd, o, app, res, identifiers_.cbegin(), identifiers_.cend());
  }

  /// @brief Get the user's evaluator.
  const expr::evaluator_base<C>&
  evaluator()
  const noexcept
  {
    return *eval_ptr_;
  }

  /// @brief Get the set of variables.
  const identifiers_type&
  identifiers()
  const noexcept
  {
    return identifiers_;
  }

  /// @brief Get the target.
  const identifier_type&
  target()
  const noexcept
  {
    return target_;
  }
};

/*------------------------------------------------------------------------------------------------*/

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
template <typename C, typename Evaluator, typename InputIterator>
homomorphism<C>
Expression( const order<C>& o, const Evaluator& u, InputIterator begin, InputIterator end
          , const typename C::Identifier& target)
{
  if (std::distance(begin, end) == 0)
  {
    return Id<C>();
  }

  typedef typename C::Identifier identifier_type;
  typename hom::expression<C>::identifiers_type identifiers(begin, end);
  std::sort( identifiers.begin(), identifiers.end()
           , [&](const identifier_type& lhs, const identifier_type& rhs)
                {
                  return o.compare(lhs, rhs); // use order to compare two identifiers
                });

  using derived_type = hom::expr::evaluator_derived<C, Evaluator>;
  std::unique_ptr<derived_type> evaluator_ptr(new derived_type(u));

  const auto& last_identifier = *std::prev(identifiers.end());
  if (o.compare(target, last_identifier))
  {
    return homomorphism<C>::create( mem::construct<hom::expression<C>>()
                                  , std::move(evaluator_ptr), std::move(identifiers), target);
  }
  else
  {
    // The target is below all identifiers, it's a much simpler case to handle
    return homomorphism<C>::create( mem::construct<hom::simple_expression<C>>()
                                  , std::move(evaluator_ptr), std::move(identifiers), target);
  }
}

/// @brief Create the Expression homomorphism.
/// @related homomorphism
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
