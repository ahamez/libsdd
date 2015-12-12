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
/// @brief cons homomorphism.
template <typename C, typename Valuation>
struct _cons
{
  /// @brief
  const order<C> o;

  /// @brief The valuation of the SDD to create.
  const Valuation valuation;

  /// @brief The homomorphism to apply on successors.
  const homomorphism<C> next;

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>&, const SDD<C>& x)
  const
  {
    return {o.variable(), valuation, next(cxt, o.next(), x)};
  }

  /// @brief Skip predicate.
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
  operator==(const _cons& lhs, const _cons& rhs)
  noexcept
  {
    return lhs.valuation == rhs.valuation and lhs.next == rhs.next and lhs.o == rhs.o;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const _cons& c)
  {
    return os << "cons(" << c.o.identifier() << ", " << c.valuation << ", " << c.next
    << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Describe cons characteristics.
template <typename C, typename Valuation>
struct homomorphism_traits<_cons<C, Valuation>>
{
  static constexpr bool should_cache = false;
};

/*------------------------------------------------------------------------------------------------*/

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the cons homomorphism.
/// @related homomorphism
template <typename C, typename Valuation>
homomorphism<C>
cons(const order<C> o, const Valuation& val, const homomorphism<C>& h)
{
  return hom::make<C, hom::_cons<C, Valuation>>(o, val, h);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::cons.
template <typename C, typename Valuation>
struct hash<sdd::hom::_cons<C, Valuation>>
{
  std::size_t
  operator()(const sdd::hom::_cons<C, Valuation>& h)
  const
  {
    using namespace sdd::hash;
    return seed(h.o) (val(h.valuation)) (val(h.next));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
