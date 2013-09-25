#ifndef _SDD_ORDER_CARRIER_HH_
#define _SDD_ORDER_CARRIER_HH_

#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/local.hh"
#include "sdd/order/order.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief Get the succession of Local that apply h on target.
/// @param o The actual order.
/// @param target Must belong to o.
/// @param h The homomorphism to apply on target.
template <typename C>
homomorphism<C>
carrier(const order<C>& o, const typename C::Identifier& target, homomorphism<C> h)
{
  const auto& path = *(o.identifiers().find(target)->path_ptr);
  for (auto cit = path.rbegin(); cit != path.rend(); ++cit)
  {
    h = Local(*cit, h);
  }
  return h;
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_ORDER_CARRIER_HH_
