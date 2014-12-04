#pragma once

#include <map>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include "sdd/dd/definition.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

/// @internal
using sequences_frequency_type
  = std::map< unsigned int    /* sequence length */
            , unsigned int >; /* frequency */

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct parents_visitor
{
  /// @brief The number of parents for a node.
  std::unordered_map<const void*, unsigned int> parents;

  /// @brief |0|.
  void
  operator()(const zero_terminal<C>&)
  noexcept
  {
    assert(false);
  }

  /// @brief |1|.
  void
  operator()(const one_terminal<C>&)
  const
  {}

  /// @brief Flat SDD.
  void
  operator()(const flat_node<C>& n)
  {
    auto insertion = parents.emplace(&n, 1);
    if (insertion.second)
    {
      for (auto&& arc : n)
      {
        visit(*this, arc.successor());
      }
    }
    else
    {
      insertion.first->second += 1;
    }
  }

  /// @brief Hierarchical SDD.
  void
  operator()(const hierarchical_node<C>&)
  const
  {
    using namespace std::string_literals;
    throw std::runtime_error(__PRETTY_FUNCTION__ + ": TODO"s);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct sequences_visitor
{
  /// @brief Tells if a node has already been encoutered.
  std::unordered_set<const void*> visited;

  /// @brief Stores the result.
  sequences_frequency_type map;

  /// @brief The number of parents for a node.
  const std::unordered_map<const void*, unsigned int>& parents;

  sequences_visitor(const std::unordered_map<const void*, unsigned int>& par)
    : parents(par)
  {}

  /// @brief |0|.
  void
  operator()(const zero_terminal<C>&, unsigned int)
  const noexcept
  {
    assert(false);
  }

  /// @brief |1|.
  void
  operator()(const one_terminal<C>&, unsigned int depth)
  noexcept
  {
    map[depth] += 1;
  }

  /// @brief Flat SDD.
  void
  operator()(const flat_node<C>& n, unsigned int depth)
  {
    const auto addr = &n;
    if (visited.emplace(addr).second)
    {
      assert(parents.find(addr) != parents.cend());
      if (parents.find(addr)->second > 1) // More than one parent
      {
        map[depth] += 1;
        depth = 0; // Reset sequence even if there is only one arc.
      }

      if (n.size() == 1)
      {
        visit(*this, n.begin()->successor(), depth + 1);
      }
      else
      {
        map[depth] += 1;
        for (auto&& arc : n)
        {
          visit(*this, arc.successor(), 0);
        }
      }
    }
    else
    {
      map[depth] += 1;
    }
  }

  /// @brief Hierarchical SDD.
  void
  operator()(const hierarchical_node<C>&, unsigned int)
  const noexcept
  {
    using namespace std::string_literals;
    throw std::runtime_error(__PRETTY_FUNCTION__ + ": TODO"s);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Returns the frequency of sequences.
///
/// A sequence is defined by a succession of nodes with only one successor and only one predecessor.
/// Helps identify if sharing is efficient or not.
template <typename C>
sequences_frequency_type
sequences(const SDD<C>& x)
{
  parents_visitor<C> v1;
  visit(v1, x);
  sequences_visitor<C> v(v1.parents);
  visit(v, x, 0);
  v.map.erase(0);
  return v.map;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools