/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <list>

#include "sdd/order/order_builder.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief Creates an order with a maximum number of identifiers per hierarchy.
template <typename C>
class identifiers_per_hierarchy
{
private:

  const unsigned int nb_variables;

public:

  identifiers_per_hierarchy(unsigned int nb)
  noexcept
    : nb_variables(nb)
  {}

  order_builder<C>
  operator()(order_builder<C> ob)
  const
  {
    if (ob.empty() or nb_variables == 1)
    {
      return ob;
    }

    while (ob.height() > nb_variables)
    {
      const auto packets = packetize(ob);
      order_builder<C> tmp;
      for (auto rcit = packets.rbegin(); rcit != packets.rend(); ++rcit)
      {
        tmp.push(order_identifier<C>(), *rcit);
      }
      ob = tmp;
    }

    return ob;
  }

private:

  std::list<order_builder<C>>
  packetize(const order_builder<C>& ob)
  const
  {
    std::list<order_builder<C>> packets;
    packetize_impl(ob, packets);
    return packets;
  }

  unsigned int
  packetize_impl(const order_builder<C>& ob, std::list<order_builder<C>>& packets)
  const
  {
    if (ob.empty())
    {
      packets.emplace_front();
      return 0;
    }
    else
    {
      const auto nb = packetize_impl(ob.next(), packets);
      if (nb == nb_variables)
      {
        packets.emplace_front(ob.identifier(), ob.nested());
        return 1;
      }
      else
      {
        packets.front().push(ob.identifier(), ob.nested());
        return nb + 1;
      }
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd
