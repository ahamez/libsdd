/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <iosfwd>
#include <stdexcept> // invalid_argument

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/identity.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief if then else homomorphism.
template <typename C>
struct _if_then_else
{
  /// @brief The predicate (acts as filter).
  const homomorphism<C> h_if;

  /// @brief The true branch (works on the accepted part).
  const homomorphism<C> h_then;

  /// @brief The false branch (works on the rejected part).
  const homomorphism<C> h_else;

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& s)
  const
  {
    // Apply predicate.
    const auto tmp = h_if(cxt, o, s);

    dd::sum_builder<C, SDD<C>> sum_operands(cxt.sdd_context());
    sum_operands.reserve(2);

    // Apply "then" on the part accepted by the predicate.
    sum_operands.add(h_then(cxt, o, tmp));

    // Apply "else" on the part rejected by the predicate.
    sum_operands.add(h_else(cxt, o, dd::difference(cxt.sdd_context(), s, std::move(tmp))));

    return dd::sum(cxt.sdd_context(), std::move(sum_operands));
  }

  /// @brief Skip predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return h_if.skip(o) and h_else.skip(o) and h_then.skip(o);
  }

  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    // h_if is always a selector.
    return h_else.selector() and h_then.selector();
  }

  /// @brief Equality.
  friend
  bool
  operator==(const _if_then_else& lhs, const _if_then_else& rhs)
  noexcept
  {
    return lhs.h_if == rhs.h_if and lhs.h_then == rhs.h_then and lhs.h_else == rhs.h_else;
  }

  /// @brief Textual output.
  friend
  std::ostream&
  operator<<(std::ostream& os, const _if_then_else<C>& ite)
  {
    return os << "ite(" << ite.h_if << ", " << ite.h_then << "," << ite.h_else << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the "if then else" homomorphism.
/// @related homomorphism
///
/// ite(pred, then, else)(x) == then(pred(x)) + else(x - pred(x))
template <typename C>
homomorphism<C>
if_then_else( const homomorphism<C>& h_if, const homomorphism<C>& h_then
            , const homomorphism<C>& h_else)
{
  if (not h_if.selector())
  {
    throw std::invalid_argument("Predicate for 'if then else' must be a selector.");
  }

  // The else branch can never be applied if no paths are removed by the predicate.
  if (h_if == id<C>())
  {
    return h_then;
  }

  // If both branches apply the same operation, then the predicate is useless.
  if (h_then == h_else)
  {
    return h_then;
  }

  return hom::make<C, hom::_if_then_else<C>>(h_if, h_then, h_else);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::_if_then_else.
template <typename C>
struct hash<sdd::hom::_if_then_else<C>>
{
  std::size_t
  operator()(const sdd::hom::_if_then_else<C>& ite)
  const
  {
    using namespace sdd::hash;
    return seed(ite.h_if) (val(ite.h_then)) (val(ite.h_else));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
