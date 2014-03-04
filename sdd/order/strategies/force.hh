#ifndef _SDD_ORDER_STRATEGIES_FORCE_HH_
#define _SDD_ORDER_STRATEGIES_FORCE_HH_

#include <cassert>
#include <functional> // reference_wrapper
#include <limits>
#include <numeric>    // accumulate
#include <stdexcept>  // runtime_error
#include <vector>

#include <boost/container/flat_set.hpp>

#include <fstream>

#include "sdd/hom/definition.hh"
#include "sdd/order/order.hh"
#include "sdd/order/strategies/force_hyperedge.hh"
#include "sdd/order/strategies/force_hypergraph.hh"
#include "sdd/order/strategies/force_vertex.hh"

namespace sdd { namespace force {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
class worker
{
private:

  using id_type = typename C::Identifier;
  using vertex_type = vertex<id_type>;
  using hyperedge_type = hyperedge<id_type>;

  /// @brief
  std::deque<vertex_type>& vertices_;

  /// @brief The hyperedges that link together vertices.
  std::deque<hyperedge_type>& hyperedges_;

public:

  /// @brief Constructor.
  worker(std::deque<vertex_type>& vertices, std::deque<hyperedge_type>& hyperedges)
    : vertices_(vertices), hyperedges_(hyperedges)
  {}

  /// @brief Effectively apply the FORCE ordering strategy.
  order<C>
  operator()()
  {
    std::vector<std::reference_wrapper<vertex_type>>
      sorted_vertices(vertices_.begin(), vertices_.end());

    double span = std::numeric_limits<double>::max();
    double old_span = 0;

    do
    {
      old_span = span;

      // Compute the new center of gravity for every hyperedge.
      for (auto& edge : hyperedges_)
      {
        edge.compute_center_of_gravity();
      }

      // Compute the tentative new location of every vertex.
      for (auto& vertex : vertices_)
      {
        assert(not vertex.hyperedges().empty());
        vertex.location() = std::accumulate( vertex.hyperedges().cbegin()
                                           , vertex.hyperedges().cend()
                                           , 0
                                           , [](double acc, const hyperedge_type* e)
                                               {return acc + e->center_of_gravity();}
                                           ) / vertex.hyperedges().size();
      }

      // Sort tentative vertex locations.
      std::sort( sorted_vertices.begin(), sorted_vertices.end()
               , [](vertex_type& lhs, vertex_type& rhs){return lhs.location() < rhs.location();});

      // Assign integer indices to the vertices.
      unsigned int pos = 0;;
      std::for_each( sorted_vertices.begin(), sorted_vertices.end()
                   , [&pos](vertex_type& v){v.location() = pos++;});

      span = get_total_span();
    } while (old_span > span);

    order_builder<C> ob;
//    for (auto rcit = sorted_vertices.rbegin(); rcit != sorted_vertices.rend(); ++rcit)
//    {
//      ob.push(vertex.get().id);
//    }
    for (const auto& vertex : sorted_vertices)
    {
      ob.push(vertex.get().id());
    }

    return order<C>(ob);
  }

private:

  /// @brief Add the span of all hyperedges.
  double
  get_total_span()
  const noexcept
  {
    return std::accumulate( hyperedges_.cbegin(), hyperedges_.cend(), 0
                          , [](double acc, const hyperedge_type& h){return acc + h.span();});
  }
};

} // namespace force

/*------------------------------------------------------------------------------------------------*/

/// @brief The FORCE ordering strategy.
///
/// See http://dx.doi.org/10.1145/764808.764839 for the details.
template <typename C>
order<C>
force_ordering(force::hypergraph<C>& graph)
{
  return force::worker<C>(graph.vertices(), graph.hyperedges())();
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_ORDER_STRATEGIES_FORCE_HH_
