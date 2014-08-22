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
  operator()(const one_terminal<C>&)
  const noexcept
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
struct length_by_node_visitor
{
  /// @brief Required by mem::variant visitor mechanism.
  using result_type = std::pair<unsigned int, bool /* has_parent */>;

  /// @brief The length of the sequence starting at a node.
  mutable std::unordered_map<const char*, unsigned int> lengths;

  /// @brief The number of parents for a node.
  const std::unordered_map<const char*, unsigned int>& parents;

  length_by_node_visitor(const std::unordered_map<const char*, unsigned int>& par)
    : parents(par)
  {}

  /// @brief |0|.
  result_type
  operator()(const zero_terminal<C>&)
  const noexcept
  {
    assert(false);
    return {0, false};
  }

  /// @brief |1|.
  result_type
  operator()(const one_terminal<C>&)
  const noexcept
  {
    return {1, true};
  }

  /// @brief Flat SDD.
  result_type
  operator()(const flat_node<C>& n)
  const
  {
    auto insertion = lengths.emplace(reinterpret_cast<const char*>(&n), 0);
    if (insertion.second)
    {
      if (n.size() == 1)
      {
        const auto rec = visit(*this, n.begin()->successor());
        if (rec.second) // successor has other parents
        {
          insertion.first->second = 1;
        }
        else
        {
          insertion.first->second = rec.first + 1;
        }
      }
      else
      {
        for (const auto& arc : n)
        {
          visit(*this, arc.successor());
        }
        insertion.first->second = 0;
      }
    }

//    std::cout << reinterpret_cast<const void*>(&n) << " ";
//    const auto p =  std::make_pair( insertion.first->second
//                                  , parents.find(reinterpret_cast<const char*>(&n))->second > 1);
//    std::cout << p.first << " | " << p.second << std::endl;;
//    return p;
    return { insertion.first->second
           , parents.find(reinterpret_cast<const char*>(&n))->second > 1};
//
//
//    if (parents.find(reinterpret_cast<const char*>(&n))->second > 1) // more than one parent
//    {
//      std::cout << "=> 0\n";
//      return 0;
//    }
//    else
//    {
//      std::cout << insertion.first->second << std::endl;
//      return insertion.first->second;
//    }
  }

  /// @brief Hierarchical SDD.
  result_type
  operator()(const hierarchical_node<C>&)
  const
  {
    assert(false);
    return {0, false};
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

  /// @brief Stores the length computed by length_by_node_visitor.
  const std::unordered_map<const char*, unsigned int>& lengths;

  /// @brief The number of parents for a node.
  const std::unordered_map<const char*, unsigned int>& parents;

  sequences_visitor( const std::unordered_map<const char*, unsigned int>& cache
                   , const std::unordered_map<const char*, unsigned int>& par)
    : lengths(cache), parents(par)
  {}

  /// @brief |0|.
  result_type
  operator()(const zero_terminal<C>&, bool)
  const noexcept
  {
    assert(false);
  }

  /// @brief |1|.
  result_type
  operator()(const one_terminal<C>&, bool)
  const noexcept
  {}

  /// @brief Flat SDD.
  result_type
  operator()(const flat_node<C>& n, bool in_sequence)
  const
  {
    if (visited.emplace(reinterpret_cast<const char*>(&n)).second)
    {
      if (n.size() == 1)
      {
        if (not in_sequence) // a new sequence starts here
        {
          const auto length = lengths.find(reinterpret_cast<const char*>(&n))->second;
          auto insertion = map.emplace(length, 0);
          insertion.first->second += 1;
//          std::cout << reinterpret_cast<const void*>(&n) << " => " << insertion.first->second << std::endl;
          visit(*this, n.begin()->successor(), true);
        }
        else
        {
          if (parents.find(reinterpret_cast<const char*>(&n))->second > 1)
          {
            visit(*this, n.begin()->successor(), false);
          }
          else
          {
            visit(*this, n.begin()->successor(), true);
          }
        }
      }
      else
      {
        for (const auto& arc : n)
        {
          visit(*this, arc.successor(), false);
        }
      }
    }
  }

  /// @brief Hierarchical SDD.
  result_type
  operator()(const hierarchical_node<C>&, bool)
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
  length_by_node_visitor<C> v2(v1.parents);
  visit(v2, x);
  sequences_visitor<C> v3(v2.lengths, v1.parents);
  visit(v3, x, false);
  return v3.map;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools
