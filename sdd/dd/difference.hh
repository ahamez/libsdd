#ifndef _SDD_DD_DIFFERENCE_HH_
#define _SDD_DD_DIFFERENCE_HH_

#include <cassert>
#include <iosfwd>

#include "sdd/dd/context_fwd.hh"
#include "sdd/dd/definition.hh"
#include "sdd/dd/operations_fwd.hh"
#include "sdd/dd/square_union.hh"
#include "sdd/dd/top.hh"
#include "sdd/util/hash.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

namespace dd {

/// @internal
/// @brief Implementation of the difference operation.
///
/// Called by difference_op.
template <typename C>
struct difference_visitor
{
  /// @brief Indicate to mem::variant what is the type of the result.
  typedef SDD<C> result_type;

  /// @brief The evaluation context.
  context<C>& cxt_;

  /// @brief We store the left operand in case we need to throw a top.
  const SDD<C> lhs_;

  /// @brief We store the right operand in case we need to throw a top.
  const SDD<C> rhs_;

  difference_visitor(context<C>& cxt, const SDD<C>& lhs, const SDD<C>& rhs)
  noexcept
  	: cxt_(cxt)
    , lhs_(lhs)
    , rhs_(rhs)
  {
  }

  /// @brief Perform the difference operation.
  template <typename Valuation>
  SDD<C>
  operator()(const node<C, Valuation>& lhs, const node<C, Valuation>& rhs)
  const
  {
    // Check if both operands are compatible.
    if (not (lhs.variable() == rhs.variable()))
    {
      throw top<C>(lhs_, rhs_);
    }

    // Compute union of all rhs valuations.
    sum_builder<C, Valuation> sum_builder(rhs.size());
    for (auto& rhs_arc : rhs)
    {
      sum_builder.add(rhs_arc.valuation());
    }
    const Valuation rhs_union = sum(cxt_, std::move(sum_builder));

    square_union<C, Valuation> su;

    // We iterate two times on lhs's alpha, and we possibly add each arc, modified, two times.
    // First when removing rhs_union, then when we look for all common parts).
    su.reserve(lhs.size() * 2);

    // For each valuation of lhs, remove the quantity rhs_union.
    for (auto& lhs_arc : lhs)
    {
      Valuation tmp = difference(cxt_, lhs_arc.valuation(), rhs_union);
      if (not tmp.empty())
      {
        su.add(lhs_arc.successor(), std::move(tmp));
      }
    }

    // For all common parts, propagate the difference on succcessors.
    for (auto& lhs_arc : lhs)
    {
      for (auto& rhs_arc : rhs)
      {
        intersection_builder<C, Valuation> inter_builder;
        inter_builder.add(lhs_arc.valuation());
        inter_builder.add(rhs_arc.valuation());
        Valuation tmp_val = intersection(cxt_, std::move(inter_builder));
        if (not tmp_val.empty())
        {
          SDD<C> tmp_succ = difference(cxt_, lhs_arc.successor(), rhs_arc.successor());
          if (not tmp_succ.empty())
          {
            su.add(tmp_succ, std::move(tmp_val));
          }
        }
      }
    }

    if (su.empty()) // Avoid a useless allocation when calling square_union::operator().
    {
      return zero<C>();
    }
    else
    {
      return SDD<C>(lhs.variable(), su(cxt_));
    }
  }

  /// @brief Always an error, difference with |0| as an operand is not cached.
  SDD<C>
  operator()(const zero_terminal<C>&, const zero_terminal<C>&)
  const noexcept
  {
    assert(false && "SDD difference: |0| in cache.");
    __builtin_unreachable();
  }

  /// @brief Always an error, difference with |1| as an operand is not cached.
  SDD<C>
  operator()(const one_terminal<C>&, const one_terminal<C>&)
  const noexcept
  {
    assert(false && "SDD difference: |1| in cache.");
    __builtin_unreachable();
  }

  /// @brief The difference of two different types is always an error.
  template <typename T, typename U>
  __attribute__((noreturn))
  SDD<C>
  operator()(const T&, const U&)
  const
  {
    throw top<C>(lhs_, rhs_);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The difference operation in the cache.
template <typename C>
struct difference_op
{
  /// @brief Needed by the cache to know the result of this operation.
  typedef SDD<C> result_type;

  context<C>& cxt_;
  const SDD<C> lhs_;
  const SDD<C> rhs_;

  difference_op(context<C>& cxt, const SDD<C>& lhs, const SDD<C>& rhs)
  	: cxt_(cxt)
    , lhs_(lhs)
		, rhs_(rhs)
  {
  }

  SDD<C>
  operator()()
  const
  {
    return apply_binary_visitor( difference_visitor<C>(cxt_, lhs_, rhs_)
                               , lhs_->data(), rhs_->data());
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related difference_op
template <typename C>
inline
bool
operator==(const difference_op<C>& lhs, const difference_op<C>& rhs)
noexcept
{
  return lhs.lhs_ == rhs.lhs_ and lhs.rhs_ == rhs.rhs_;
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related difference_op
template <typename C>
std::ostream&
operator<<(std::ostream& os, const difference_op<C>& x)
{
  return os << "- (" << x.lhs_ << "," << x.rhs_ << ")";
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The difference operation.
///
/// The computation is cached, except for the trivial cases (when the two operands are
/// equal or when one of the operand is |0|).
template <typename C>
inline
SDD<C>
difference(context<C>& cxt, const SDD<C>& lhs, const SDD<C>& rhs)
{
  if (lhs == rhs or lhs == zero<C>())
  {
    return zero<C>();
  }
  else if (rhs == zero<C>())
  {
    return lhs;
  }
  return cxt.difference_cache()(difference_op<C>(cxt, lhs, rhs));
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related SDD
template <typename C, typename Values>
inline
Values
difference(context<C>&, const Values& lhs, const Values& rhs)
{
  return difference(lhs, rhs);
}

} // namespace dd

/*------------------------------------------------------------------------------------------------*/

/// @brief   Perform the difference of two SDD.
/// @related SDD
template <typename C>
inline
SDD<C>
operator-(const SDD<C>& lhs, const SDD<C>& rhs)
{
  return dd::difference(dd::initial_context<C>(), lhs, rhs);
}

/// @brief   Perform the difference of two SDD.
/// @related SDD
template <typename C>
inline
SDD<C>&
operator-=(SDD<C>& lhs, const SDD<C>& rhs)
{
  SDD<C> tmp = dd::difference(dd::initial_context<C>(), lhs, rhs);
  using std::swap;
  swap(tmp, lhs);
  return lhs;
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::dd::difference_op
template <typename C>
struct hash<sdd::dd::difference_op<C>>
{
  std::size_t
  operator()(const sdd::dd::difference_op<C>& op)
  const noexcept
  {
    std::size_t seed = 0;
    sdd::util::hash_combine(seed, op.lhs_);
    sdd::util::hash_combine(seed, op.rhs_);
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_DD_DIFFERENCE_HH_
