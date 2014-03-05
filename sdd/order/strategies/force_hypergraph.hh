#ifndef _SDD_ORDER_STRATEGIES_FORCE_HYPERGRAPH_HH_
#define _SDD_ORDER_STRATEGIES_FORCE_HYPERGRAPH_HH_

#include <deque>
#include <memory> // shared_ptr
#include <unordered_map>
#include <vector>

#include "sdd/order/strategies/force_hyperedge.hh"
#include "sdd/order/strategies/force_vertex.hh"

namespace sdd { namespace force {

/*------------------------------------------------------------------------------------------------*/

/// @brief Represent the connections between the identifiers.
template <typename C>
class hypergraph
{
public:

  /// @brief The user identifier type.
  using identifier_type = typename C::Identifier;

private:

  /// @brief An hypergraph's vertex.
  using vertex_type = vertex<identifier_type>;

  /// @brief An hypergraph's hyperedge.
  using hyperedge_type = hyperedge<identifier_type>;

  /// @brief All the vertices of this hypergraph.
  ///
  /// A deque is used because insertions at the end don't invalid references.
  std::shared_ptr<std::deque<vertex_type>> vertices_ptr_;

  /// @brief All the hyperedges of this hypergraph.
  ///
  /// A deque is used because insertions at the end don't invalid references.
  std::shared_ptr<std::deque<hyperedge_type>> hyperedges_ptr_;

  /// @brief Map identifiers to vertices.
  ///
  /// Enable fast retrieving of a vertex's address.
  std::shared_ptr<std::unordered_map<identifier_type, vertex_type*>> id_to_vertex_ptr_;

public:

  /// @brief Default constructor.
  hypergraph()
    : vertices_ptr_(std::make_shared<std::deque<vertex_type>>())
    , hyperedges_ptr_(std::make_shared<std::deque<hyperedge_type>>())
    , id_to_vertex_ptr_(std::make_shared<std::unordered_map<identifier_type, vertex_type*>>())
  {}

  /// @brief Add a new hyperedge with a set of identifiers.
  template <typename InputIterator>
  void
  add_hyperedge(InputIterator it, InputIterator end)
  {
    if (it == end)
    {
      return;
    }

    // Create, if necessary all connected vertices.
    std::vector<vertex_type*> vertices;
    for (; it != end; ++it)
    {
      auto insertion = id_to_vertex_ptr_->emplace(*it, nullptr);
      if (insertion.second) // new vertex
      {
        vertices_ptr_->emplace_back(*it, next_location());
        insertion.first->second = &vertices_ptr_->back();
      }
      vertices.emplace_back(insertion.first->second);
    }
    assert(vertices.size() != 0);

    // Create the new hyperedge.
    hyperedges_ptr_->emplace_back(std::move(vertices));

    // Update connected vertices.
    assert(hyperedges_ptr_->back().vertices().size() != 0);
    for (auto vertex_ptr : hyperedges_ptr_->back().vertices())
    {
      assert(vertex_ptr != nullptr);
      vertex_ptr->hyperedges().emplace_back(&hyperedges_ptr_->back());
    }
  }

  /// @internal
  std::deque<vertex_type>&
  vertices()
  noexcept
  {
    return *vertices_ptr_;
  }

  /// @internal
  std::deque<hyperedge_type>&
  hyperedges()
  noexcept
  {
    return *hyperedges_ptr_;
  }

private:

  /// @brief Get the next initial location for a new vertex.
  static
  double
  next_location()
  noexcept
  {
    static double location = 0;
    return location++;
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::force

#endif // _SDD_ORDER_STRATEGIES_FORCE_HYPERGRAPH_HH_
