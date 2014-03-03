#ifndef _SDD_ORDER_STRATEGIES_FORCE_VERTEX_HH_
#define _SDD_ORDER_STRATEGIES_FORCE_VERTEX_HH_

#include <vector>

#include "sdd/order/strategies/force_hyperedge_fwd.hh"

namespace sdd { namespace force {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename Identifier>
struct vertex
{
  using identifier_type = Identifier;

  /// @brief The corresponding identifier.
  const identifier_type id;

  /// @brief This vertex's tentative location.
  double location;

  /// @brief The hyperedges this vertex is connected to.
  std::vector<hyperedge<Identifier>*> hyperedges;

  /// @brief Constructor.
  vertex(const Identifier& id, double l)
    : id(id), location(l), hyperedges()
  {}
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::force

#endif // _SDD_ORDER_STRATEGIES_FORCE_VERTEX_HH_
