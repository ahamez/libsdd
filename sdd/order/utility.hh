#ifndef _SDD_ORDER_UTILITY_HH_
#define _SDD_ORDER_UTILITY_HH_

#include <cassert>

#include "sdd/dd/definition.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace order {

/*-------------------------------------------------------------------------------------------*/

/// @brief Construct an SDD from an order.
/// @param init should be a functor returning the initial set of values for an identifier.
template <typename C, typename Initializer>
dd::SDD<C>
sdd(const order<C>& o, const Initializer& init)
{
  if (not o)
  {
    return dd::one<C>();
  }
  // hierarchical
  else if (o->nested_)
  {
    return dd::SDD<C>(o->variable_, sdd(o->nested_, init), sdd(o->next_, init));
  }
  // flat
  else
  {
    assert(o->identifier_ && "Artificial flat node.");
    return dd::SDD<C>(o->variable_, init(*o->identifier_), sdd(o->next_, init));
  }
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::order

#endif // _SDD_ORDER_UTILITY_HH_
