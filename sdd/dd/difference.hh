/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <cassert>
#include <iosfwd>

#include "sdd/internal_manager_fwd.hh"
#include "sdd/dd/context_fwd.hh"
#include "sdd/dd/definition.hh"
#include "sdd/dd/operations_fwd.hh"
#include "sdd/dd/square_union.hh"
#include "sdd/dd/top.hh"
#include "sdd/util/hash.hh"
#include "sdd/values/empty.hh"

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
  /// @brief The evaluation context.
  context<C>& cxt_;

  /// @brief Perform the difference operation.
  template <typename Valuation>
  SDD<C>
  operator()( const node<C, Valuation>& lhs, const node<C, Valuation>& rhs
            , const SDD<C>& lhs_orig, const SDD<C>& rhs_orig)
  const
  {
    // Check if both operands are compatible.
    if (not (lhs.variable() == rhs.variable()))
    {
      throw top<C>(lhs_orig, rhs_orig);
    }

    mem::rewinder _(cxt_.arena());

    // Compute union of all rhs valuations.
    sum_builder<C, Valuation> sum_builder(cxt_);
    sum_builder.reserve(rhs.size());
    for (auto& rhs_arc : rhs)
    {
      sum_builder.add(rhs_arc.valuation());
    }
    auto rhs_union = sum(cxt_, std::move(sum_builder));

    square_union<C, Valuation> su(cxt_);

    // We iterate two times on lhs's alpha, and we possibly add each arc, modified, two times.
    // First when removing rhs_union, then when we look for all common parts.
    su.reserve(lhs.size() * 2);

    // For each valuation of lhs, remove the quantity rhs_union.
    for (auto& lhs_arc : lhs)
    {
      auto tmp = difference(cxt_, lhs_arc.valuation(), rhs_union);
      if (not values::empty_values(tmp))
      {
        su.add(lhs_arc.successor(), std::move(tmp));
      }
    }

    // For all common parts, propagate the difference on succcessors.
    for (auto& lhs_arc : lhs)
    {
      for (auto& rhs_arc : rhs)
      {
        intersection_builder<C, Valuation> inter_builder(cxt_);
        inter_builder.add(lhs_arc.valuation());
        inter_builder.add(rhs_arc.valuation());
        Valuation tmp_val = intersection(cxt_, std::move(inter_builder));
        if (not values::empty_values(tmp_val))
        {
          SDD<C> tmp_succ = difference(cxt_, lhs_arc.successor(), rhs_arc.successor());
          if (not values::empty_values(tmp_succ))
          {
            su.add(std::move(tmp_succ), std::move(tmp_val));
          }
        }
      }
    }

    return su.empty()
         ? zero<C>() // avoid a useless allocation when calling square_union::operator()
         : SDD<C>(lhs.variable(), su());
  }

  /// @brief Always an error, difference with |0| is not cached.
  SDD<C>
  operator()(const zero_terminal<C>&, const zero_terminal<C>&, const SDD<C>&, const SDD<C>&)
  const noexcept
  {
    assert(false && "SDD difference: |0| in cache.");
    __builtin_unreachable();
  }

  /// @brief Always an error, difference with |1| is not cached.
  SDD<C>
  operator()(const one_terminal<C>&, const one_terminal<C>&, const SDD<C>&, const SDD<C>&)
  const noexcept
  {
    assert(false && "SDD difference: |1| in cache.");
    __builtin_unreachable();
  }

  /// @brief The difference of two different types is always an error.
  template <typename T, typename U>
  __attribute__((noreturn))
  SDD<C>
  operator()(const T&, const U&, const SDD<C>& lhs_orig, const SDD<C>& rhs_orig)
  const
  {
    throw top<C>(lhs_orig, rhs_orig);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The difference operation in the cache.
template <typename C>
struct difference_op
{
  /// @brief The left operand of this difference operation.
  const SDD<C> left;

  /// @brief The right operand of this difference operation.
  const SDD<C> right;

  /// @brief Apply this operation.
  ///
  /// Called by the cache.
  SDD<C>
  operator()(context<C>& cxt)
  const
  {
    return binary_visit(difference_visitor<C>{cxt}, left, right, left, right);
  }

  friend
  bool
  operator==(const difference_op& lhs, const difference_op& rhs)
  noexcept
  {
    return lhs.left == rhs.left and lhs.right == rhs.right;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const difference_op& x)
  {
    return os << "- (" << x.left << "," << x.right << ")";
  }

};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related SDD
/// @brief The difference operation.
///
/// The computation is cached, except for the trivial cases (when the two operands are
/// equal or when one of the operand is |0|).
template <typename C>
inline
SDD<C>
difference(context<C>& cxt, SDD<C> lhs, SDD<C> rhs)
{
  if (lhs == rhs or lhs == zero<C>())
  {
    return zero<C>();
  }
  else if (rhs == zero<C>())
  {
    return lhs;
  }
  return cxt.difference_cache()({std::move(lhs), std::move(rhs)});
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C, typename Values>
inline
Values
difference(context<C>&, const Values& lhs, const Values& rhs)
{
  return difference(lhs, rhs);
}

} // namespace dd

/*------------------------------------------------------------------------------------------------*/

/// @brief Perform the difference of two SDD.
/// @related SDD
template <typename C>
inline
SDD<C>
operator-(const SDD<C>& lhs, const SDD<C>& rhs)
{
  return dd::difference(global<C>().sdd_context, lhs, rhs);
}

/// @brief Perform the difference of two SDD.
/// @related SDD
template <typename C>
inline
SDD<C>&
operator-=(SDD<C>& lhs, const SDD<C>& rhs)
{
  SDD<C> tmp = dd::difference(global<C>().sdd_context, lhs, rhs);
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
  const
  {
    using namespace sdd::hash;
    return seed(op.left) (val(op.right));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
