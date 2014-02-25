#ifndef _SDD_TOOLS_SIZE_HH_
#define _SDD_TOOLS_SIZE_HH_

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
  /// @brief Required by mem::variant visitor mechanism.
  using result_type = std::size_t;

  /// @brief A cache is necessary to to know if a node has already been encountered.
  ///
  /// We use the addresses of nodes as key. It's legit because nodes are unified and immutable.
  mutable std::unordered_set<const char*> visited_;

  /// @brief |0|.
  result_type
  operator()(const zero_terminal<C>& n)
  const
  {
    if (visited_.emplace(reinterpret_cast<const char*>(&n)).second)
    {
      return sizeof(zero_terminal<C>);
    }
    else
    {
      return 0;
    }
  }

  /// @brief |1|.
  result_type
  operator()(const one_terminal<C>& n)
  const
  {
    if (visited_.emplace(reinterpret_cast<const char*>(&n)).second)
    {
      return sizeof(one_terminal<C>);
    }
    else
    {
      return 0;
    }
  }

  /// @brief Flat SDD.
  result_type
  operator()(const flat_node<C>& n)
  const
  {
    if (visited_.emplace(reinterpret_cast<const char*>(&n)).second)
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
  result_type
  operator()(const hierarchical_node<C>& n)
  const
  {
    if (visited_.emplace(reinterpret_cast<const char*>(&n)).second)
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

/// @internal
template <typename C>
std::size_t
size(const SDD<C>& x)
{
  return visit(size_visitor<C>(), x);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools

#endif // _SDD_TOOLS_SIZE_HH_
