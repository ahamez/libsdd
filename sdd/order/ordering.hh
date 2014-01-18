#ifndef _SDD_ORDER_ORDERING_HH_
#define _SDD_ORDER_ORDERING_HH_

#include "sdd/order/order_builder.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief Apply ordering strategies.
template <typename C, typename Strategy>
order_builder<C>
transform_order(const order_builder<C>& ob, Strategy&& strategy)
{
  return strategy(ob);
}

/// @brief Apply ordering strategies.
template <typename C, typename Strategy, typename... Strategies>
order_builder<C>
transform_order(const order_builder<C>& ob, Strategy strategy, Strategies&&... strategies)
{
  return transform_order(strategy(ob), std::forward<Strategies>(strategies)...);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_ORDER_ORDERING_HH_
