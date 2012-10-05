#ifndef _SDD_HOM_CONS_HH_
#define _SDD_HOM_CONS_HH_

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"

namespace sdd { namespace hom {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Cons homomorphism.
template <typename C, typename Valuation>
class cons
{
public:

  /// @brief The variable type.
  typedef typename C::Variable variable_type;

private:

  /// @brief The variable of the SDD to create.
  const variable_type variable_;

  /// @brief The valuation of the SDD to create.
  const Valuation     valuation_;

public:

  /// @brief Constructor.
  cons(const variable_type& var, const Valuation& val)
    : variable_(var)
    , valuation_(val)
  {
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const SDD<C>& x)
  const
  {
    return SDD<C>(variable_, valuation_, x);
  }

  /// @brief Skip variable predicate.
  constexpr bool
  skip(const variable_type&)
  const noexcept
  {
    return false;
  }

  /// @brief Return the variable of the SDD to create.
  const variable_type&
  variable()
  const noexcept
  {
    return variable_;
  }

  /// @brief Return the valuation of the SDD to create.
  const Valuation&
  valuation()
  const noexcept
  {
    return valuation_;
  }
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Describe Cons characteristics.
template <typename C, typename Valuation>
struct homomorphism_traits<cons<C, Valuation>>
{
  static constexpr bool should_cache = false;
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Equality of two Cons homomorphisms.
/// @related cons
template <typename C, typename Valuation>
inline
bool
operator==(const cons<C, Valuation>& lhs, const cons<C, Valuation>& rhs)
noexcept
{
  return lhs.variable() == rhs.variable() and lhs.valuation() == rhs.valuation();
}

/// @endcond

/*-------------------------------------------------------------------------------------------*/

/// @brief Create the Cons homomorphism.
/// @related homomorphism
template <typename C, typename Valuation>
homomorphism<C>
Cons(const typename C::Variable& var, const Valuation& val)
{
  return homomorphism<C>::create(internal::mem::construct<cons<C, Valuation>>(), var, val);
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

namespace std {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Hash specialization for sdd::hom::cons.
template <typename C, typename Valuation>
struct hash<sdd::hom::cons<C, Valuation>>
{
  std::size_t
  operator()(const sdd::hom::cons<C, Valuation>& h)
  const noexcept
  {
    std::size_t seed = 0;
    sdd::internal::util::hash_combine(seed, h.variable());
    sdd::internal::util::hash_combine(seed, h.valuation());
    return seed;
  }
};

/// @endcond

/*-------------------------------------------------------------------------------------------*/

} // namespace std


#endif // _SDD_HOM_CONS_HH_
