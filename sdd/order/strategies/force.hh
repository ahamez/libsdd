#ifndef _SDD_ORDER_STRATEGIES_FORCE_HH_
#define _SDD_ORDER_STRATEGIES_FORCE_HH_

#include <functional> // reference_wrapper
#include <limits>
#include <numeric>    // accumulate
#include <vector>

#include "sdd/order/order_builder.hh"
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

  /// @brief Keep all computed total spans for statistics.
  std::deque<double> spans_;

public:

  /// @brief Constructor.
  worker(hypergraph<C>& graph)
    : vertices_(graph.vertices()), hyperedges_(graph.hyperedges())
  {}

  /// @brief Effectively apply the FORCE ordering strategy.
  order_builder<C>
  operator()(unsigned int iterations = 200)
  {
    std::vector<std::reference_wrapper<vertex_type>>
      sorted_vertices(vertices_.begin(), vertices_.end());

    // Keep a copy of the order with the smallest span.
    std::vector<std::reference_wrapper<vertex_type>> best_order(sorted_vertices);
    double smallest_span = std::numeric_limits<double>::max();

    while (iterations-- != 0)
    {
      // Compute the new center of gravity for every hyperedge.
      for (auto& edge : hyperedges_)
      {
        edge.compute_center_of_gravity();
      }

      // Compute the tentative new location of every vertex.
      for (auto& vertex : vertices_)
      {
        if (vertex.hyperedges().empty())
        {
          continue;
        }
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

      const double span = get_total_span();
      spans_.push_back(span);
      if (span < smallest_span)
      {
        // We keep the order that minimizes the span.
        best_order = sorted_vertices;
      }
    }

    order_builder<C> ob;
    for (const auto& vertex : best_order)
    {
      ob.push(vertex.get().id());
    }
    return ob;
  }

  const std::deque<double>&
  spans()
  const noexcept
  {
    return spans_;
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

} // namespace sdd

#endif // _SDD_ORDER_STRATEGIES_FORCE_HH_
