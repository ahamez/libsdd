#pragma once

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
  /// @brief Required by mem::variant visitor mechanism.
  using result_type = void;

  /// @brief The number of parents for a node.
  mutable std::unordered_map<const char*, unsigned int> parents;

  /// @brief |0|.
  result_type
  operator()(const zero_terminal<C>&)
  const noexcept
  {
    assert(false);
  }

  /// @brief |1|.
  result_type
  operator()(const one_terminal<C>& n)
  const
  {}

  /// @brief Flat SDD.
  result_type
  operator()(const flat_node<C>& n)
  const
  {
    auto insertion = parents.emplace(reinterpret_cast<const char*>(&n), 1);
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
  result_type
  operator()(const hierarchical_node<C>&)
  const
  {
    assert(false);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct sequences_visitor
{
  /// @brief Required by mem::variant visitor mechanism.
  using result_type = void;

  /// @brief Tells if a node has already been encoutered.
  mutable std::unordered_set<const char*> visited;

  /// @brief Stores the result.
  mutable sequences_frequency_type map;

  /// @brief The number of parents for a node.
  const std::unordered_map<const char*, unsigned int>& parents;

  sequences_visitor(const std::unordered_map<const char*, unsigned int>& par)
    : parents(par)
  {}

  /// @brief |0|.
  result_type
  operator()(const zero_terminal<C>&, unsigned int)
  const noexcept
  {
    assert(false);
  }

  /// @brief |1|.
  result_type
  operator()(const one_terminal<C>&, unsigned int depth)
  const noexcept
  {
    if (depth != 0)
    {
      map[depth] += 1;
    }
  }

  /// @brief Flat SDD.
  result_type
  operator()(const flat_node<C>& n, unsigned int depth)
  const
  {
    const auto addr = reinterpret_cast<const char*>(&n);
    if (visited.emplace(addr).second)
    {
      assert(parents.find(addr) != parents.cend());
      if (parents.find(addr)->second > 1) // More than one parent
      {
        if (depth != 0)
        {
          map[depth] += 1;
        }
        depth = 0;
      }

      if (n.size() == 1)
      {
        visit(*this, n.begin()->successor(), depth + 1);
      }
      else
      {
        for (auto&& arc : n)
        {
          visit(*this, arc.successor(), 0);
        }
      }
    }
    else
    {
      if (depth != 0)
      {
        map[depth] += 1;
      }
    }
  }

  /// @brief Hierarchical SDD.
  result_type
  operator()(const hierarchical_node<C>&, unsigned int)
  const
  {
    assert(false);
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
  return v.map;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools
