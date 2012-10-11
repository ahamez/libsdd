#ifndef _SDD_HOM_ID_HH_
#define _SDD_HOM_ID_HH_

#include <cassert>
#include <iosfwd>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/order/order.hh"

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
  operator()(context<C>&, const order::order<C>&, const SDD<C>&)
  const noexcept
  {
    assert(false);
    __builtin_unreachable();
  }

  /// @brief Skip predicate.
  constexpr bool
  skip(const order::order<C>&)
  const noexcept
  {
    return true;
  }

  /// @brief Selector predicate
  constexpr bool
  selector()
  const noexcept
  {
    return true;
  }
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Equality of two identity homomorphisms.
/// @related identity
template <typename C>
inline
constexpr bool
operator==(const identity<C>&, const identity<C>&)
noexcept
{
  return true;
}

/// @related identity
template <typename C>
std::ostream&
operator<<(std::ostream& os, const identity<C>&)
{
  return os << "Id";
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
