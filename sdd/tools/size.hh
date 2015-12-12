/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <unordered_map>
#include <unordered_set>
#include <utility> // pair

#include "sdd/dd/definition.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct size_visitor
{
  /// @brief A cache is necessary to to know if a node has already been encountered.
  ///
  /// We use the addresses of nodes as key. It's legit because nodes are unified and immutable.
  std::unordered_set<const void*> visited_;

  /// @brief |0|.
  std::size_t
  operator()(const zero_terminal<C>& n)
  {
    if (visited_.emplace(&n).second)
    {
      return sizeof(zero_terminal<C>);
    }
    else
    {
      return 0;
    }
  }

  /// @brief |1|.
  std::size_t
  operator()(const one_terminal<C>& n)
  {
    if (visited_.emplace(&n).second)
    {
      return sizeof(one_terminal<C>);
    }
    else
    {
      return 0;
    }
  }

  /// @brief Flat SDD.
  std::size_t
  operator()(const flat_node<C>& n)
  {
    if (visited_.emplace(&n).second)
    {
      std::size_t res = sizeof(typename SDD<C>::unique_type) // size of a ref_counted
                      + n.size() * sizeof(typename flat_node<C>::arc_type); // arcs
      for (const auto& arc : n)
      {
        res += visit(*this, arc.successor());
      }
      return res;
    }
    else
    {
      return 0;
    }
  }

  /// @brief Hierarchical SDD.
  std::size_t
  operator()(const hierarchical_node<C>& n)
  {
    if (visited_.emplace(&n).second)
    {
      std::size_t res = sizeof(typename SDD<C>::unique_type) // size of a ref_counted
                      + n.size() * sizeof(typename hierarchical_node<C>::arc_type); // arcs
      for (const auto& arc : n)
      {
        res += visit(*this, arc.valuation());
        res += visit(*this, arc.successor());
      }
      return res;
    }
    else
    {
      return 0;
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Get the memory usage of an SDD, in bytes.
template <typename C>
std::size_t
size(const SDD<C>& x)
{
  return visit(size_visitor<C>(), x);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools
