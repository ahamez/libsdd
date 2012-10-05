#ifndef _SDD_HOM_ID_HH_
#define _SDD_HOM_ID_HH_

#include <cassert>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"

namespace sdd { namespace hom {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Return its operand.
template <typename C>
struct identity
{
  /// @brief Evaluation.
  ///
  /// This is an error to arrive here as the identity is computed before calling the cache.
  SDD<C>
  operator()(context<C>&, const SDD<C>&)
  const noexcept
  {
    assert(false);
    __builtin_unreachable();
  }

  /// @brief Skip variable predicate.
  constexpr bool
  skip(const typename C::Variable& v)
  const noexcept
  {
    return true;
  }
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Equality of two identity homomorphisms.
template <typename C>
constexpr
bool
operator==(const identity<C>&, const identity<C>&)
noexcept
{
  return true;
}

/// @endcond

/*-------------------------------------------------------------------------------------------*/

/// @brief Create the Identity homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
Id()
{
  static homomorphism<C> id = homomorphism<C>::create(internal::mem::construct<identity<C>>());
  return id;
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

namespace std {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Hash specialization for sdd::hom::identity.
template <typename C>
struct hash<sdd::hom::identity<C>>
{
  constexpr
  std::size_t
  operator()(const sdd::hom::identity<C>&)
  const noexcept
  {
    return 607769;
  }
};

/// @endcond

/*-------------------------------------------------------------------------------------------*/

} // namespace std


#endif // _SDD_HOM_ID_HH_
