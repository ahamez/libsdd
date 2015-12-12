/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <deque>
#include <memory> // shared_ptr
#include <sstream>
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
  /// Enable fast retrieval of a vertex's address.
  std::shared_ptr<std::unordered_map<identifier_type, vertex_type*>> id_to_vertex_ptr_;

public:

  /// @brief Default constructor.
  template <typename InputIterator>
  hypergraph(InputIterator it, InputIterator end)
    : vertices_ptr_(std::make_shared<std::deque<vertex_type>>())
    , hyperedges_ptr_(std::make_shared<std::deque<hyperedge_type>>())
    , id_to_vertex_ptr_(std::make_shared<std::unordered_map<identifier_type, vertex_type*>>())
  {
    static double location = 0;
    assert(it != end);
    for (; it != end; ++it)
    {
      vertices_ptr_->emplace_back(*it, location++);
      const auto insertion = id_to_vertex_ptr_->emplace(*it, &vertices_ptr_->back());
      if (not insertion.second)
      {
        std::stringstream ss;
        ss << "Identifier " <<  *it << " appears twice.";
        throw std::runtime_error(ss.str());
      }
    }
    assert(id_to_vertex_ptr_->size() != 0);
  }

  /// @brief Add a new hyperedge with a set of identifiers.
  template <typename InputIterator>
  void
  add_hyperedge(InputIterator it, InputIterator end, double weight = 1)
  {
    if (it == end)
    {
      return;
    }

    // Create, if necessary all connected vertices.
    std::vector<vertex_type*> vertices;
    vertices.reserve(std::distance(it, end));
    for (; it != end; ++it)
    {
      const auto search = id_to_vertex_ptr_->find(*it);
      assert(search != id_to_vertex_ptr_->end());
      vertices.emplace_back(search->second);
    }
    assert(vertices.size() != 0);

    // Create the new hyperedge.
    hyperedges_ptr_->emplace_back(weight, std::move(vertices));

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
  const std::deque<vertex_type>&
  vertices()
  const noexcept
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

  /// @internal
  const std::deque<hyperedge_type>&
  hyperedges()
  const noexcept
  {
    return *hyperedges_ptr_;
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::force
