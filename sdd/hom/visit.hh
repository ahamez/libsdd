#ifndef _SDD_HOM_VISIT_HH_
#define _SDD_HOM_VISIT_HH_

#include <utility> // forward

#include "sdd/hom/definition_fwd.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief Apply a visitor to an homomorphism.
/// @related homomorphism
template <typename C, typename Visitor, typename... Args>
inline
typename Visitor::result_type
visit(const Visitor& v, const homomorphism<C>& x, Args&&... args)
{
  return apply_visitor(v, x->data(), std::forward<Args>(args)...);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_HOM_VISIT_HH_
