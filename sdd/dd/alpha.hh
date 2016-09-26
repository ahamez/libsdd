/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <boost/container/flat_map.hpp>

#include "sdd/dd/context_fwd.hh"
#include "sdd/dd/definition_fwd.hh"
#include "sdd/mem/linear_alloc.hh"
#include "sdd/util/hash.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief Represent an arc of an alpha.
template <typename C, typename Valuation>
class arc final
{
private:

  /// @brief This arc's valuation, either an SDD or a set of values.
  const Valuation valuation_;

  /// @brief This arcs's SDD successor.
  const SDD<C> successor_;

public:

  /// @brief The valuation type.
  using valuation_type = Valuation;

  /// @internal
  /// @brief Constructor.
  arc(Valuation&& val, SDD<C>&& succ)
    : valuation_(std::move(val))
    , successor_(std::move(succ))
  {}

  /// @brief Get the valuation of this arc.
  const Valuation&
  valuation()
  const noexcept
  {
    return valuation_;
  }

  /// @brief Get the successor of this arc.
  SDD<C>
  successor()
  const noexcept
  {
    return successor_;
  }

  /// @brief Equality of two arcs.
  friend
  bool
  operator==(const arc& lhs, const arc& rhs)
  noexcept
  {
    return lhs.successor_ == rhs.successor_ and lhs.valuation_ == rhs.valuation_;
  }
};

/*------------------------------------------------------------------------------------------------*/

namespace dd {

/// @internal
/// @brief Helper class to build an alpha.
///
/// It serves two goals. First, it ensures that all alphas use the same order to store arcs.
/// Second, it helps to place the alpha directly behind the owner node to avoid an indirection.
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
  boost::container::flat_map< SDD<C>, Valuation, std::less<SDD<C>>
                            , mem::linear_alloc<std::pair<SDD<C>, Valuation>>> map_;

public:

  /// @brief Default constructor.
  alpha_builder(context<C>& cxt)
    : map_(std::less<SDD<C>>(), mem::linear_alloc<std::pair<SDD<C>, Valuation>>(cxt.arena()))
  {}

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
  template <typename Valuation_, typename SDD_>
  void
  add(Valuation_&& val, SDD_&& succ)
  {
    map_.emplace(std::forward<SDD_>(succ), std::forward<Valuation_>(val));
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
  consolidate(char* addr)
  noexcept
  {
    arc<C, Valuation>* base = reinterpret_cast<arc<C, Valuation>*>(addr);
    for (auto& a : map_)
    {
      new (base++) arc<C, Valuation>(std::move(a.second), std::move(a.first));
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::arc
template <typename C, typename Valuation>
struct hash<sdd::arc<C, Valuation>>
{
  std::size_t
  operator()(const sdd::arc<C, Valuation>& arc)
  const
  {
    using namespace sdd::hash;
    return seed(arc.valuation()) (val(arc.successor()));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
