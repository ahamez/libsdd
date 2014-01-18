#ifndef _SDD_ORDER_STRATEGIES_VARIABLES_PER_LEVEL_HH_
#define _SDD_ORDER_STRATEGIES_VARIABLES_PER_LEVEL_HH_

#include <tuple>

#include "sdd/order/order_builder.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief
template <typename C>
struct variables_per_level
{
  const unsigned int nb_variables;

  variables_per_level(unsigned int nb)
  noexcept
    : nb_variables(nb)
  {}

  order_builder<C>
  operator()(const order_builder<C>& ob)
  const
  {
    if (ob.empty() or nb_variables == 1)
    {
      return ob;
    }
    order_builder<C> current = ob;
    while (current.height() > nb_variables)
    {
      const auto tmp = impl(current, current.height() % nb_variables);
      current = std::get<0>(tmp) << std::get<1>(tmp);
    }
    return current.height() == 1 ? current.nested() : current;
  }

  std::tuple<order_builder<C>/*current*/, order_builder<C>/*next*/, unsigned int/*counter*/>
  impl(const order_builder<C>& ob, unsigned int remainder)
  const
  {
    if (ob.next().empty())
    {
      return std::make_tuple( order_builder<C>(ob.identifier(), ob.nested())
                            , order_builder<C>()
                            , nb_variables + remainder - 2);
    }
    else
    {
      const auto tmp = impl(ob.next(), remainder);
      if (std::get<2>(tmp) == 0) // current hierarchy is full
      {
        return std::make_tuple( order_builder<C>()
                              , order_builder<C>( order_identifier<C>()
                                                , order_builder<C>(ob.identifier(), ob.nested())
                                                    << std::get<0>(tmp))
                                  << std::get<1>(tmp)
                              , nb_variables - 1);
      }
      else
      {
        return std::make_tuple( order_builder<C>(ob.identifier(), ob.nested()) << std::get<0>(tmp)
                              , std::get<1>(tmp)
                              , std::get<2>(tmp) - 1);
      }
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_ORDER_STRATEGIES_VARIABLES_PER_LEVEL_HH_
