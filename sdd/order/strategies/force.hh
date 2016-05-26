/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

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
/// @brief An implementation of the FORCE ordering strategy
/// @see http://dx.doi.org/10.1145/764808.764839
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

  /// @brief Reverse order.
  bool reverse_;

public:

  /// @brief Constructor.
  worker(hypergraph<C>& graph, bool reverse = false)
    : vertices_(graph.vertices()), hyperedges_(graph.hyperedges()), spans_(), reverse_(reverse)
  {}

  /// @brief Effectively apply the FORCE ordering strategy.
  order_builder<C>
  operator()(unsigned int iterations)
  {
    auto sorted_vertices
      = std::vector<std::reference_wrapper<vertex_type>>{vertices_.begin(), vertices_.end()};

    // Keep a copy of the order with the smallest span.
    auto best_order = std::vector<std::reference_wrapper<vertex_type>>{sorted_vertices};
    const auto smallest_span = std::numeric_limits<double>::max();

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
                                               {return acc + e->center_of_gravity() * e->weight();}
                                           ) / vertex.hyperedges().size();
      }

      // Sort tentative vertex locations.
      std::sort( sorted_vertices.begin(), sorted_vertices.end()
               , [](vertex_type& lhs, vertex_type& rhs){return lhs.location() < rhs.location();});

      // Assign integer indices to the vertices.
      auto pos = 0ul;
      std::for_each( sorted_vertices.begin(), sorted_vertices.end()
                   , [&pos](vertex_type& v){v.location() = pos++;});

      const auto span = get_total_span();
      spans_.push_back(span);
      if (span < smallest_span)
      {
        // We keep the order that minimizes the span.
        best_order = sorted_vertices;
      }
    }

    auto ob = order_builder<C>{};
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
