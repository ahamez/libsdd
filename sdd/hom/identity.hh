/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

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
template <typename C>
struct _identity
{
  /// @brief Evaluation.
  ///
  /// This is an error to call this function, as the identity is computed before calling the cache
  /// (in homomorphism<C>::operator()).
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

  friend
  constexpr bool
  operator==(const _identity&, const _identity&)
  noexcept
  {
    return true;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const _identity&)
  {
    return os << "Id";
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the identity homomorphism.
/// @related homomorphism
template <typename C>
inline
homomorphism<C>
id()
noexcept
{
  return global<C>().id;
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::_identity.
template <typename C>
struct hash<sdd::hom::_identity<C>>
{
  constexpr
  std::size_t
  operator()(const sdd::hom::_identity<C>&)
  const noexcept
  {
    return 607769;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
