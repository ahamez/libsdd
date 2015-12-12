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
#include "sdd/hom/identity.hh"
#include "sdd/hom/local.hh"
#include "sdd/order/order.hh"

namespace sdd {

// Forward declaration needed by fixpoint_builder_helper.
template <typename C>
homomorphism<C>
fixpoint(const homomorphism<C>&);

namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Fixpoint homomorphism.
template <typename C>
struct _fixpoint
{
  /// @brief The homomorphism to apply until a fixpoint is reached.
  const homomorphism<C> h;

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& x)
  const
  {
    SDD<C> x1 = x;
    SDD<C> x2 = x1;
    do
    {
      x2 = x1;
      swap(x1, h(cxt, o, x1));
    } while (x1 != x2);
    return x1;
  }

  /// @brief Skip predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return h.skip(o);
  }

  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return h.selector();
  }

  friend
  bool
  operator==(const _fixpoint& lhs, const _fixpoint& rhs)
  noexcept
  {
    return lhs.h == rhs.h;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const _fixpoint& f)
  {
    return os << "(" << f.h << ")*";
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Concrete creation of Fixpoint.
template <typename C>
struct fixpoint_builder
{
  homomorphism<C>
  operator()(const _identity<C>&, const homomorphism<C>& h)
  const noexcept
  {
    return h;
  }

  homomorphism<C>
  operator()(const _fixpoint<C>&, const homomorphism<C>& h)
  const noexcept
  {
    return h;
  }

  template <typename T>
  homomorphism<C>
  operator()(const T&, const homomorphism<C>& h)
  const noexcept
  {
    return hom::make<C, _fixpoint<C>>(h);
  }
};

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the Fixpoint homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
fixpoint(const homomorphism<C>& h)
{
  return visit(hom::fixpoint_builder<C>{}, h, h);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::_fixpoint.
template <typename C>
struct hash<sdd::hom::_fixpoint<C>>
{
  std::size_t
  operator()(const sdd::hom::_fixpoint<C>& f)
  const
  {
    using namespace sdd::hash;
    return seed(345789) (val(f.h));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
