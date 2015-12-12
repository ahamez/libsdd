/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <cassert>

#include <boost/container/flat_map.hpp>

#include "sdd/dd/alpha.hh"
#include "sdd/dd/context_fwd.hh"
#include "sdd/dd/definition.hh"
#include "sdd/dd/operations_fwd.hh"
#include "sdd/mem/linear_alloc.hh"

namespace sdd { namespace dd {
  
/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The square union operation.
template <typename C, typename Valuation>
class square_union
{
private:

  /// @brief Store the a list of valuation to be merged with a union operation.
  using sum_builder_type = sum_builder<C, Valuation>;

  /// @brief The SDD evaluation context.
  context<C>& cxt_;

  /// @brief Map successors to a list of valuations going to this same successor.
  boost::container::flat_map< SDD<C>, sum_builder_type, std::less<SDD<C>>
                            , mem::linear_alloc<std::pair<SDD<C>, sum_builder_type>>> map_;

public:

  /// @brief Constructor.
  square_union(context<C>& cxt)
    : cxt_(cxt)
    , map_( std::less<SDD<C>>()
          , mem::linear_alloc<std::pair<SDD<C>, sum_builder_type>>(cxt.arena()))
  {}

  /// @brief Add a new (reversed) arc to the operands.
  ///
  /// @pre not succ.empty() and not val.empty()
  template <typename SDD_, typename Valuation_>
  void
  add(SDD_&& succ, Valuation_&& val)
  {
    assert(not succ.empty() and not val.empty());

    const auto lb = map_.lower_bound(succ);
    if (lb != map_.end() and not map_.key_comp()(succ, lb->first))
    {
      lb->second.add(std::forward<Valuation_>(val));
    }
    else
    {
      auto ins = map_.emplace_hint(lb, std::forward<SDD_>(succ), sum_builder_type(cxt_));
      ins->second.add(std::forward<Valuation_>(val));
    }
  }

  /// @brief Request for allocation of additional memory.
  void
  reserve(std::size_t size)
  {
    map_.reserve(size);
  }

  /// @brief Remove all operands.
  ///
  /// Mostly needed by the intersection operation to avoid the creation of multiple
  /// square_union.
  void
  clear()
  noexcept
  {
    map_.clear();
  }

  /// @brief Tell if no operands have been added yet.
  bool
  empty()
  const noexcept
  {
    return map_.empty();
  }

  /// @brief  Perform the square union operation on operands added with add().
  /// @return An alpha (builder) suitable to construct an SDD.
  ///
  /// Once performed, all subsequent calls to this instance are invalid, except clear().
  /// Once clear() has been called, it's OK to use this square_union again.
  alpha_builder<C, Valuation>
  operator()()
  {
    alpha_builder<C, Valuation> builder(cxt_);
    builder.reserve(map_.size());
    for (auto& reversed_arc : map_)
    {
      builder.add(sum(cxt_, std::move(reversed_arc.second)), std::move(reversed_arc.first));
    }
    return builder;
  }
};

/*------------------------------------------------------------------------------------------------*/
  
}} // namespace sdd::dd
