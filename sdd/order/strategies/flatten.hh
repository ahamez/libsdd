/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

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
      return order_builder<C>(ob.identifier().user()) << (*this)(ob.next());
    }
    else
    {
      return (*this)(ob.nested()) << (*this)(ob.next());
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd
