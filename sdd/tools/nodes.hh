/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <unordered_set>

#include "sdd/dd/definition.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct nb_nodes_visitor
{
  using result_type = std::pair<unsigned int, unsigned int>;

  /// @brief A cache is necessary to to know if a node has already been encountered.
  ///
  /// We use the addresses of nodes as key. It's legit because nodes are unified and immutable.
  std::unordered_set<const void*> visited_;

  /// @brief |0|.
  result_type
  operator()(const zero_terminal<C>&)
  const
  {
    return std::make_pair(0, 0);
  }

  /// @brief |1|.
  result_type
  operator()(const one_terminal<C>&)
  const
  {
    return std::make_pair(0, 0);
  }

  /// @brief Flat SDD.
  result_type
  operator()(const flat_node<C>& n)
  {
    if (visited_.emplace(&n).second)
    {
      result_type res {1, 0};
      for (const auto& arc : n)
      {
        accumulate_pair(res, visit(*this, arc.successor()));
      }
      return res;
    }
    else
    {
      return std::make_pair(0, 0);
    }
  }

  /// @brief Hierarchical SDD.
  result_type
  operator()(const hierarchical_node<C>& n)
  {
    if (visited_.emplace(&n).second)
    {
      result_type res {0, 1};
      for (const auto& arc : n)
      {
        accumulate_pair(res, visit(*this, arc.valuation()));
        accumulate_pair(res, visit(*this, arc.successor()));
      }
      return res;
    }
    else
    {
      return std::make_pair(0, 0);
    }
  }

private:

  static
  void
  accumulate_pair(result_type& lhs, result_type&& rhs)
  {
    lhs.first += rhs.first;
    lhs.second += rhs.second;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Get the number of unique nodes of an SDD.
///
/// Each node is counted only once.
template <typename C>
std::pair<unsigned int, unsigned int>
nodes(const SDD<C>& x)
{
  return visit(nb_nodes_visitor<C>(), x);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools
