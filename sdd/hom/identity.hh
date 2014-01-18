#ifndef _SDD_HOM_ID_HH_
#define _SDD_HOM_ID_HH_

#include <cassert>
#include <iosfwd>

#include "sdd/internal_manager_fwd.hh"
#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The Identity homomorphism which returns its operand.
///
/// Its evaluation is done in homomorphism<C>::operator().
template <typename C>
struct identity
{
  /// @brief Evaluation.
  ///
  /// This is an error to call this function, as the identity is computed before calling the cache.
  SDD<C>
  operator()(context<C>&, const order<C>&, const SDD<C>&)
  const noexcept
  {
    assert(false);
    __builtin_unreachable();
  }

  /// @brief Skip predicate.
  constexpr bool
  skip(const order<C>&)
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

/*------------------------------------------------------------------------------------------------*/

/// @internal
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

/// @internal
/// @related identity
template <typename C>
std::ostream&
operator<<(std::ostream& os, const identity<C>&)
{
  return os << "Id";
}

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the Identity homomorphism.
/// @related homomorphism
template <typename C>
inline
homomorphism<C>
Id()
noexcept
{
  return global<C>().id;
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::identity.
template <typename C>
struct hash<sdd::hom::identity<C>>
{
  constexpr
  std::size_t
  operator()(const sdd::hom::identity<C>&)
  const noexcept
  {
    return sdd::util::hash(607769);
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_ID_HH_
