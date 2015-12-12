/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/local.hh"
#include "sdd/order/order.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief Get the succession of local that apply h on target.
/// @param o The actual order.
/// @param target Must belong to o.
/// @param h The homomorphism to apply on target.
template <typename C>
homomorphism<C>
carrier(const order<C>& o, const typename C::Identifier& target, homomorphism<C> h)
{
  const auto& path = o.node(target).path();
  for (auto cit = path.rbegin(); cit != path.rend(); ++cit)
  {
    h = local(o.node_from_position(*cit).variable(), h);
  }
  return h;
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd
