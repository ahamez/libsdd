#ifndef _SDD_ORDER_CARRIER_HH_
#define _SDD_ORDER_CARRIER_HH_

#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/local.hh"
#include "sdd/order/order.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

  /// @brief Get the succession of Local that apply h on target.
template <typename C>
homomorphism<C>
carrier(const order<C>& o, const typename C::Identifier& target, homomorphism<C> h)
{
  const auto& identifiers = o.identifiers();
  const auto search = identifiers.find(target);
  if (search == identifiers.end())
  {
    std::stringstream ss;
    ss << "Identifier " << target << " not found";
    throw std::runtime_error(ss.str());
  }
  const auto& path = *(search->path_ptr);
  for (auto cit = path.rbegin(); cit != path.rend(); ++cit)
  {
    h = Local(*cit, o, h);
  }
  return h;
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_ORDER_CARRIER_HH_
