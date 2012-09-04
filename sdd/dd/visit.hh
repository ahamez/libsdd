#ifndef _SDD_DD_VISIT_HH_
#define _SDD_DD_VISIT_HH_

#include "sdd/dd/definition.hh"

namespace sdd { namespace dd {

/*-------------------------------------------------------------------------------------------*/

/// @brief Apply a visitor to an SDD.
template <typename C, typename Visitor>
inline
typename Visitor::result_type
visit(const Visitor& v, const SDD<C>& x)
{
  return apply_visitor(v, x->data());
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd

#endif // SDD_DD_VISIT_HH_
