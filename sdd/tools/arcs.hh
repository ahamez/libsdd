/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <map>
#include <unordered_set>
#include <utility> // pair

#include "sdd/dd/definition.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

using arcs_frequency_type
  = std::map< unsigned int /*number of arcs*/
            , std::pair< unsigned int /* flat arcs frequency */
                       , unsigned int /* hierarchical arcs frequency */>>;

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct arcs_visitor
{
  /// @brief A cache is necessary to to know if a node has already been encountered.
  ///
  /// We use the addresses of nodes as key. It's legit because nodes are unified and immutable.
  std::unordered_set<const void*> visited_;

  /// @brief Stores the frequency of apparition of a number of arcs.
  arcs_frequency_type map_;

  /// @brief |0|.
  void
  operator()(const zero_terminal<C>&)
  const
  {}

  /// @brief |1|.
  void
  operator()(const one_terminal<C>&)
  const
  {}

  /// @brief Flat SDD.
  void
  operator()(const flat_node<C>& n)
  {
    if (visited_.emplace(&n).second)
    {
      map_[n.size()].first += 1;
      for (const auto& arc : n)
      {
        visit(*this, arc.successor());
      }
    }
  }

  /// @brief Hierarchical SDD.
  void
  operator()(const hierarchical_node<C>& n)
  {
    if (visited_.emplace(&n).second)
    {
      map_[n.size()].second += 1;
      for (const auto& arc : n)
      {
        visit(*this, arc.valuation());
        visit(*this, arc.successor());
      }
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Get the arc frequency map of an SDD.
///
/// An arc frequency map indicates the number of nodes with 1, 2, etc. arcs.
template <typename C>
arcs_frequency_type
arcs(const SDD<C>& x)
{
  arcs_visitor<C> visitor;
  visit(visitor, x);
  return std::move(visitor.map_);
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Get the total number of arcs from an arc frequency map.
inline
std::pair<unsigned int /* flat arcs */, unsigned int /* hierarchical arcs */>
number_of_arcs(const arcs_frequency_type& freq)
noexcept
{
  std::pair<unsigned int, unsigned int> res {0, 0};
  for (const auto& kv : freq)
  {
    res.first += kv.first * kv.second.first;
    res.second += kv.first * kv.second.second;
  }
  return res;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools
