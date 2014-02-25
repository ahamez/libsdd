#ifndef _SDD_TOOLS_ARCS_HH_
#define _SDD_TOOLS_ARCS_HH_

#include <unordered_map>
#include <unordered_set>
#include <utility> // pair

#include "sdd/dd/definition.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

using arcs_frequency_type
  = std::unordered_map< std::size_t /*number of arcs*/
                      , std::pair< std::size_t /* flat arcs frequency */
                      , std::size_t /* hierarchical arcs frequency */>>;

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct arcs_visitor
{
  /// @brief Required by mem::variant visitor mechanism.
  using result_type = void;

  /// @brief A cache is necessary to to know if a node has already been encountered.
  ///
  /// We use the addresses of nodes as key. It's legit because nodes are unified and immutable.
  mutable std::unordered_set<const char*> visited_;

  /// @brief Stores the frequency of apparition of a number of arcs.
  mutable arcs_frequency_type map_;

  /// @brief |0|.
  result_type
  operator()(const zero_terminal<C>& n)
  const
  {}

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
    if (visited_.emplace(reinterpret_cast<const char*>(&n)).second)
    {
      map_[n.size()].first += 1;
      for (const auto& arc : n)
      {
        visit(*this, arc.successor());
      }
    }
  }

  /// @brief Hierarchical SDD.
  result_type
  operator()(const hierarchical_node<C>& n)
  const
  {
    if (visited_.emplace(reinterpret_cast<const char*>(&n)).second)
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
template <typename C>
arcs_frequency_type
arcs(const SDD<C>& x)
{
  arcs_visitor<C> visitor;
  visit(visitor, x);
  return std::move(visitor.map_);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools

#endif // _SDD_TOOLS_ARCS_HH_
