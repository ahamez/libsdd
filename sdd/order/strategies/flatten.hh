#ifndef _SDD_ORDER_STRATEGIES_FLATTEN_HH_
#define _SDD_ORDER_STRATEGIES_FLATTEN_HH_

#include "sdd/order/order_builder.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief Flatten an order_builder.
///
/// Flatten an order_builder by removing all hierarchy variables.
template <typename C>
struct flatten
{
  order_builder<C>
  operator()(const order_builder<C>& ob)
  const
  {
    if (ob.empty())
    {
      return ob;
    }
    else if (ob.nested().empty())
    {
      return order_builder<C>(ob.identifier().user()) << operator()(ob.next());
    }
    else
    {
      return operator()(ob.nested()) << operator()(ob.next());
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_ORDER_STRATEGIES_FLATTEN_HH_
