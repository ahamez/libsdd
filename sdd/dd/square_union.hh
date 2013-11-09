#ifndef _SDD_DD_SQUARE_UNION_HH_
#define _SDD_DD_SQUARE_UNION_HH_

#include <cassert>

#include "sdd/dd/alpha.hh"
#include "sdd/dd/context_fwd.hh"
#include "sdd/dd/definition.hh"
#include "sdd/dd/operations_fwd.hh"
#include "sdd/util/boost_flat_map_no_warnings.hh"

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

  /// @brief Map successors to a list of valuations going to this same successor.
  boost::container::flat_map<SDD<C>, sum_builder_type> map_;

public:

  /// @brief Add a new (reversed) arc to the operands.
  ///
  /// @pre not succ.empty() and not val.empty()
  template <typename Val>
  void
  add(const SDD<C>& succ, Val&& val)
  {
    assert(not succ.empty() and not val.empty());

    auto lb = map_.lower_bound(succ);
    if (lb != map_.end() and not map_.key_comp()(succ, lb->first))
    {
      lb->second.add(std::forward<Val>(val));
    }
    else
    {
      auto ins = map_.emplace_hint(lb, succ, sum_builder_type());
      ins->second.add(std::forward<Val>(val));
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
  operator()(context<C>& cxt)
  {
    alpha_builder<C, Valuation> builder;
    builder.reserve(map_.size());
    for (auto& reversed_arc : map_)
    {
      builder.add(sum(cxt, std::move(reversed_arc.second)), std::move(reversed_arc.first));
    }
    return builder;
  }
};

/*------------------------------------------------------------------------------------------------*/
  
}} // namespace sdd::dd

#endif // _SDD_DD_SQUARE_UNION_HH_
