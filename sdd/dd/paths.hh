#ifndef _SDD_DD_PATHS_HH_
#define _SDD_DD_PATHS_HH_

#include <unordered_map>

#include "sdd/dd/definition.hh"
#include "sdd/dd/visit.hh"

namespace sdd { namespace dd {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Visitor to count the number of paths in an SDD.
template <typename C>
struct count_paths_visitor
{

  typedef std::size_t result_type;

  /// @brief A cache is used to speedup computation.
  ///
  /// We use the addresses of nodes as key. It's legit because nodes are unified.
  mutable std::unordered_map<const void*, result_type> cache_;

  /// @brief Error case.
  ///
  /// We should not encouter any |0| as all SDD leading to |0| are reduced to |0| and as
  /// the |0| alone is treated in count_paths.
  result_type
  operator()(const zero_terminal<C>&)
  const noexcept
  {
    assert(false && "Encounterd a |0| when counting paths.");
    __builtin_unreachable();
  }

  /// @brief Terminal case of the visitation.
  constexpr
  result_type
  operator()(const one_terminal<C>&)
  const noexcept
  {
    return 1;
  }

  /// @brief The number of paths for a flat SDD.
  result_type
  operator()(const flat_node<C>& n)
  const noexcept
  {
    const auto insertion = cache_.emplace(&n, 0);
    if (insertion.second)
    {
      for (const auto& arc : n)
      {
        insertion.first->second += arc.valuation().size()
                                 * visit(*this, arc.successor());
      }
    }
    return insertion.first->second;
  }

  /// @brief The number of paths for a hierarchical SDD.
  result_type
  operator()(const hierarchical_node<C>& n)
  const noexcept
  {
    const auto insertion = cache_.emplace(&n, 0);
    if (insertion.second)
    {
      for (const auto& arc : n)
      {
        insertion.first->second += visit(*this, arc.valuation())
                                 * visit(*this, arc.successor());
      }
    }
    return insertion.first->second;
  }
};

/// @endcond

/*-------------------------------------------------------------------------------------------*/

/// @brief Compute the number of paths in an SDD.
///
/// O(N) where N is the number of nodes in x.
template <typename C>
inline
std::size_t
count_paths(const SDD<C>& x)
noexcept
{
  return x.empty() ? 0 : visit(count_paths_visitor<C>(), x);
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd

#endif // _SDD_DD_PATHS_HH_
