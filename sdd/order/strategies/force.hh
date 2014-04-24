#ifndef _SDD_ORDER_STRATEGIES_FORCE_HH_
#define _SDD_ORDER_STRATEGIES_FORCE_HH_

#include <algorithm>  // stable_sort
#include <functional> // function, reference_wrapper
#include <limits>
#include <numeric>    // accumulate
#include <vector>

#include "sdd/order/order_builder.hh"
#include "sdd/order/strategies/force_hyperedge.hh"
#include "sdd/order/strategies/force_hypergraph.hh"
#include "sdd/order/strategies/force_vertex.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief Apply the FORCE heuristic.
template <typename C, typename EdgeId>
class apply_force
{
private:

  using id_type = typename C::Identifier;
  using vertex_type = force::vertex<id_type, EdgeId>;
  using hyperedge_type = force::hyperedge<id_type, EdgeId>;

  /// @brief
  std::deque<vertex_type>& vertices_;

  /// @brief The hyperedges that link together vertices.
  std::deque<hyperedge_type>& hyperedges_;

  /// @brief Keep all computed total spans for statistics.
  std::deque<double> spans_;

  /// @brief The number of time the heuristic will be applied.
  const unsigned int iterations_;

  /// @brief Reverse the computed order.
  const bool reverse_;

  /// @brief The type of the function used to sort identifiers at each iteration.
  using sort_identifiers_type = std::function<bool(const EdgeId&, const id_type&, const id_type&)>;

  /// @brief Sort identifiers.
  sort_identifiers_type sort_identifiers_;

public:

  /// @brief Constructor.
  apply_force( force_hypergraph<C, EdgeId>& graph, unsigned int iterations, bool reverse
             , sort_identifiers_type sort)
    : vertices_(graph.vertices()), hyperedges_(graph.hyperedges()), spans_()
    , iterations_(iterations), reverse_(reverse), sort_identifiers_(sort)
  {}

  /// @brief Constructor.
  apply_force(force_hypergraph<C, EdgeId>& graph, unsigned int iterations, bool reverse)
    : apply_force(graph, iterations, reverse, sort_identifiers_type())
  {}

  /// @brief Effectively apply the FORCE ordering strategy.
  order_builder<C>
  operator()()
  {
    auto iterations = iterations_;

    std::vector<std::reference_wrapper<vertex_type>>
      sorted_vertices(vertices_.begin(), vertices_.end());

    // Keep a copy of the order with the smallest span.
    std::vector<std::reference_wrapper<vertex_type>> best_order(sorted_vertices);
    double smallest_span = std::numeric_limits<double>::max();

    while (iterations-- != 0)
    {
      if (sort_identifiers_)
      {
        for (auto& edge : hyperedges_)
        {
          std::stable_sort( edge.vertices().begin(), edge.vertices().end()
                          , [](const vertex_type* lhs, const vertex_type* rhs)
                              {return lhs->location() < rhs->location();});

          std::vector<double> locations;
          locations.reserve(edge.vertices().size());
          std::transform( edge.vertices().cbegin(), edge.vertices().cend()
                        , std::back_inserter(locations)
                        , [](const vertex_type* v){return v->location();});

          std::stable_sort( edge.vertices().begin(), edge.vertices().end()
                          , [&, this](const vertex_type* lhs, const vertex_type* rhs)
                            {return sort_identifiers_(edge.id(), lhs->id(), rhs->id());});

          auto location_cit = locations.begin();
          std::for_each( edge.vertices().begin(), edge.vertices().end()
                       , [&](vertex_type* v){v->location() = *location_cit++;});
        }
      }

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
    if (reverse_)
    {
      for (auto rcit = best_order.rbegin(); rcit != best_order.rend(); ++rcit)
      {
        ob.push(rcit->get().id());
      }
    }
    else
    {
      for (const auto& vertex : best_order)
      {
        ob.push(vertex.get().id());
      }
    }
    return ob;
  }

  /// @internal
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

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_ORDER_STRATEGIES_FORCE_HH_
