/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <unordered_map>

#include "sdd/dd/count_combinations_fwd.hh"
#include "sdd/dd/definition.hh"
#include "sdd/values/size.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Visitor to count the number of paths in an SDD.
template <typename C>
struct count_combinations_visitor
{
  /// @brief A cache is used to speed up the computation.
  ///
  /// We use the addresses of nodes as key. It's legit because nodes are unified and immutable.
  std::unordered_map<const void*, boost::multiprecision::cpp_int> cache_;

  /// @brief Error case.
  ///
  /// We should not encounter any |0| as all SDD leading to |0| are reduced to |0| and as
  /// the |0| alone is treated in count_combinations.
  boost::multiprecision::cpp_int
  operator()(const zero_terminal<C>&)
  noexcept
  {
    assert(false && "Encountered the |0| terminal when counting paths.");
    __builtin_unreachable();
  }

  /// @brief Terminal case of the recursion.
  boost::multiprecision::cpp_int
  operator()(const one_terminal<C>&)
  const noexcept
  {
    return 1;
  }

  /// @brief The number of paths for a flat SDD.
  boost::multiprecision::cpp_int
  operator()(const flat_node<C>& n)
  {
    const auto insertion = cache_.emplace(&n, 0);
    if (insertion.second)
    {
      for (const auto& arc : n)
      {
        insertion.first->second += size(arc.valuation()) * visit(*this, arc.successor());
      }
    }
    return insertion.first->second;
  }

  /// @brief The number of paths for a hierarchical SDD.
  boost::multiprecision::cpp_int
  operator()(const hierarchical_node<C>& n)
  {
    const auto insertion = cache_.emplace(&n, 0);
    if (insertion.second)
    {
      for (const auto& arc : n)
      {
        insertion.first->second += visit(*this, arc.valuation()) * visit(*this, arc.successor());
      }
    }
    return insertion.first->second;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Compute the number of combinations in an SDD.
///
/// O(N) where N is the number of nodes in x.
template <typename C>
boost::multiprecision::cpp_int
count_combinations(const SDD<C>& x)
{
  return x.empty() ? 0 : visit(count_combinations_visitor<C>(), x);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd
