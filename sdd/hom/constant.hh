/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <iosfwd>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/traits.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief constant homomorphism.
template <typename C>
struct _constant
{
  /// @brief The SDD to return.
  const SDD<C> operand;

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>&, const order<C>&, const SDD<C>&)
  const noexcept
  {
    return operand;
  }

  /// @brief Skip variable predicate.
  constexpr bool
  skip(const order<C>&)
  const noexcept
  {
    return false;
  }

  /// @brief Selector predicate
  constexpr bool
  selector()
  const noexcept
  {
    return false;
  }

  friend
  bool
  operator==(const _constant& lhs, const _constant& rhs)
  noexcept
  {
    return lhs.operand == rhs.operand;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const _constant& c)
  {
    return os << "const(" << c.operand << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Describe _constant characteristics.
template <typename C>
struct homomorphism_traits<_constant<C>>
{
  static constexpr bool should_cache = false;
};

/*------------------------------------------------------------------------------------------------*/

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the constant homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
constant(const SDD<C>& s)
{
  return hom::make<C, hom::_constant<C>>(s);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::constant.
template <typename C>
struct hash<sdd::hom::_constant<C>>
{
  constexpr
  std::size_t
  operator()(const sdd::hom::_constant<C>& c)
  const
  {
    return sdd::hash::seed(c.operand);
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
