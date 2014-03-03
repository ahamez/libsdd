#ifndef _SDD_ORDER_STRATEGIES_FORCE_VERTEX_HH_
#define _SDD_ORDER_STRATEGIES_FORCE_VERTEX_HH_

#include <vector>

#include "sdd/order/strategies/force_hyperedge_fwd.hh"

namespace sdd { namespace force {

/*------------------------------------------------------------------------------------------------*/

/// @internal
struct vertex
{
  /// @brief The corresponding index in the order's nodes.
  const unsigned int pos;

  /// @brief This vertex's tentative location.
  double location;

  /// @brief The hyperedges this vertex is connected to.
  std::vector<hyperedge*> hyperedges;

  /// @brief Constructor.
  vertex(unsigned int p, double l)
    : pos(p), location(l), hyperedges()
  {}
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::force

#endif // _SDD_ORDER_STRATEGIES_FORCE_VERTEX_HH_
