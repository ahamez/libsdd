#ifndef _SDD_ORDER_UTILITY_HH_
#define _SDD_ORDER_UTILITY_HH_

#include "sdd/dd/definition.hh"
#include "sdd/order/order.hh"

namespace sdd {

/*-------------------------------------------------------------------------------------------*/

/// @brief Construct an SDD from an order.
/// @param init should be a functor returning the initial set of values for an identifier.
template <typename C, typename Initializer>
SDD<C>
sdd_from_order(const order<C>& o, const Initializer& init)
{
  if (o.empty())
  {
    return one<C>();
  }
  // flat
  else if (o.nested().empty())
  {
    return SDD<C>(o.variable(), init(o.identifier()), sdd_from_order(o.next(), init));
  }
  // hierarchical
  else
  {
    return SDD<C>(o.variable(), sdd_from_order(o.nested(), init), sdd_from_order(o.next(), init));
  }
}

/*-------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_ORDER_UTILITY_HH_
