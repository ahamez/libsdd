#ifndef _SDD_DD_ALPHA_HH_
#define _SDD_DD_ALPHA_HH_

#include <boost/container/flat_map.hpp>

#include "sdd/dd/definition_fwd.hh"
#include "sdd/util/hash.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief Represent an arc of an alpha.
template <typename C, typename Valuation>
class arc
{
private:

  const Valuation valuation_;
  const SDD<C>    successor_;

public:

  arc(Valuation&& val, SDD<C>&& succ)
    : valuation_(std::move(val))
    , successor_(std::move(succ))
  {
  }

  /// @brief Return the valuation of this arc.
  const Valuation&
  valuation()
  const noexcept
  {
    return valuation_;
  }

  /// @brief Return the successor of this arc.
  SDD<C>
  successor()
  const noexcept
  {
    return successor_;
  }
};

/// @brief Equality of two arc.
/// @related arc
template <typename C, typename Valuation>
inline
bool
operator==(const arc<C, Valuation>& lhs, const arc<C, Valuation>& rhs)
noexcept
{
  return lhs.successor() == rhs.successor() and lhs.valuation() == rhs.valuation();
}

/*------------------------------------------------------------------------------------------------*/

namespace dd {

/// @internal
/// @brief Helper class to build an alpha.
///
/// It serves two goals. First, it ensures that all alphas use the same order to store arcs.
/// Second, it helps to place the alpha directly behind the owner node.
template <typename C, typename Valuation>
class alpha_builder
{
  // Can't copy an alpha_builder
  alpha_builder(const alpha_builder&) = delete;
  alpha_builder& operator=(const alpha_builder&) = delete;

private:

  /// @brief Temporay container of arcs.
  ///
  /// Arcs are inverted because we are guaranted that the comparison of SDD is O(1), which is
  /// not the case for Valuation (though, it's likely so). Arcs are put in the correct
  /// direction in consolidate().
  boost::container::flat_map<SDD<C>, Valuation> map_;

public:

  /// @brief Default constructor.
  alpha_builder() = default;

  /// @brief Default move constructor.
  alpha_builder(alpha_builder&&) = default;

  /// @brief Request for allocation of additional memory.
  void
  reserve(std::size_t size)
  {
    map_.reserve(size);
  }

  /// @brief Tell if the builder doesn't contain any arc.
  bool
  empty()
  const noexcept
  {
    return map_.empty();
  }

  /// @brief Get the number of arcs.
  std::size_t
  size()
  const noexcept
  {
    return map_.size();
  }

  /// @brief Add an arc to the alpha.
  /// @param val shall be a non-empty element of a partition, no verification will be made.
  /// @param succ
  void
  add(Valuation&& val, const SDD<C>& succ)
  {
    map_.emplace(succ, std::move(val));
  }

  /// @brief Add an arc to the alpha.
  /// @param val shall be a non-empty element of a partition, no verification will be made.
  /// @param succ
  void
  add(const Valuation& val, const SDD<C>& succ)
  {
    map_.emplace(succ, val);
  }

  /// @brief Compute the size needed to store all the arcs contained by this builder.
  std::size_t
  size_to_allocate()
  const noexcept
  {
    return map_.size() * sizeof(arc<C, Valuation>);
  }

  /// @brief Move arcs of this builder to a given memory location.
  /// @param addr shall point to an allocated memory location of the size returned by
  /// size_to_allocate().
  ///
  /// Once performed, all subsequent calls to this instance are invalid.  
  void
  consolidate(void* addr)
  noexcept
  {
    arc<C, Valuation>* base = reinterpret_cast<arc<C, Valuation>*>(addr);
    std::size_t i = 0;
    for (auto& a : map_)
    {
      new (base + i++) arc<C, Valuation>(std::move(a.second), std::move(a.first));
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd

#endif // _SDD_DD_ALPHA_HH_
