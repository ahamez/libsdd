#ifndef _SDD_DD_VISIT_HH_
#define _SDD_DD_VISIT_HH_

#include <utility> // forward

#include "sdd/dd/definition.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief Apply a visitor to an SDD.
/// @related SDD
template <typename C, typename Visitor, typename... Args>
inline
typename Visitor::result_type
visit(const Visitor& v, const SDD<C>& x, Args&&... args)
{
  return apply_visitor(v, x->data(), std::forward<Args>(args)...);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // SDD_DD_VISIT_HH_
