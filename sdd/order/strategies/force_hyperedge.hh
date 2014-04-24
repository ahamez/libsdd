#ifndef _SDD_ORDER_STRATEGIES_FORCE_HYPEREDGE_HH_
#define _SDD_ORDER_STRATEGIES_FORCE_HYPEREDGE_HH_

#include <algorithm>  // minmax_element
#include <numeric>    // accumulate
#include <vector>

#include "sdd/order/strategies/force_hyperedge_fwd.hh"
#include "sdd/order/strategies/force_vertex.hh"

namespace sdd { namespace force {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename Identifier, typename EdgeId>
class hyperedge
{
public:

  using identifier_type = Identifier;

private:

  /// @brief The center of gravity.
  double cog_;

  /// @brief The user identifier associated to this hyperdge.
  const EdgeId id_;

  using vertex_type = vertex<identifier_type, EdgeId>;

  /// @brief Vertices connected to this hyperedge.
  std::vector<vertex_type*> vertices_;

public:

  /// @brief Constructor with an already existing container of vertices.
  hyperedge(const EdgeId& id, std::vector<vertex_type*>&& v)
    : id_(id), cog_(0), vertices_(std::move(v))
  {}

  /// @brief Return the computed center of gravity.
  double
  center_of_gravity()
  const noexcept
  {
    return cog_;
  }

  /// @brief Return the user identifier associated to this hyperdge.
  const EdgeId&
  id()
  const noexcept
  {
    return id_;
  }

  std::vector<vertex_type*>&
  vertices()
  noexcept
  {
    return vertices_;
  }

  const std::vector<vertex_type*>&
  vertices()
  const noexcept
  {
    return vertices_;
  }

  /// @brief Compute the center of gravity.
  void
  compute_center_of_gravity()
  noexcept
  {
    assert(not vertices_.empty());
    cog_ = std::accumulate( vertices_.cbegin(), vertices_.cend(), 0
                          , [](double acc, const vertex_type* v)
                              {return acc + v->location();}
                          ) / vertices_.size();
  }

  /// @brief Compute the span of all vertices.
  double
  span()
  const noexcept
  {
    assert(not vertices_.empty());
    const auto minmax
      = std::minmax_element( vertices_.cbegin(), vertices_.cend()
                           , []( const vertex_type* lhs
                               , const vertex_type* rhs)
                               {return lhs->location() < rhs->location();});
    return (*minmax.second)->location() - (*minmax.first)->location();
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::force

#endif // _SDD_ORDER_STRATEGIES_FORCE_HYPEREDGE_HH_
