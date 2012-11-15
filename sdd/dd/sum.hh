#ifndef _SDD_DD_SUM_HH_
#define _SDD_DD_SUM_HH_

#include <unordered_map>
#include <vector>

#include <boost/container/flat_set.hpp>

#include "sdd/dd/context_fwd.hh"
#include "sdd/dd/definition.hh"
#include "sdd/dd/nary.hh"
#include "sdd/dd/operations_fwd.hh"
#include "sdd/dd/square_union.hh"
#include "sdd/internal/util/hash.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The sum operation in the cache.
template <typename C>
struct _LIBSDD_ATTRIBUTE_PACKED sum_op
  : nary_base<C, sum_op<C>>
{
  typedef nary_base<C, sum_op<C>> base_type;

  template <typename... Args>
  sum_op(Args&&... args)
    : base_type(std::forward<Args>(args)...)
  {
  }

  /// @brief Get the textual representation of the union operator.
  ///
  /// Called by top to export a textual description.
  static
  char
  symbol()
  noexcept
  {
    return '+';
  }

  /// @brief Perform the union algorithm.
  ///
  /// It's a so-called 'n-ary' union in the sense that we don't create intermediary SDD.
  /// Also, a lot of tests permit to break loops as soon as possible.
  template <enum node_tag tag>
  SDD<C>
  work()
  const
  {
    typedef typename node_for_tag<C, tag>::type node_type;
    typedef typename node_type::valuation_type valuation_type;

    auto operands_cit = base_type::begin();
    const auto operands_end = base_type::end();

    // Get the first operand as a node, we need it to initialize the algorithm.
    const node_type& head = internal::mem::variant_cast<node_type>((*operands_cit)->data());

    // Type of the list of successors for a valuation, to be merged with the union operation
    // right before calling the square union.
    typedef sum_builder<C, SDD<C>> sum_builder_type;

    /// TODO Use Boost.Intrusive to save on memory allocations?
    // List all the successors for each valuation in the final alpha.
    std::unordered_map<valuation_type, sum_builder_type> res(head.size());

    // Needed to temporarily store arcs erased from res and arcs from the alpha visited in
    // the loop (B).
    std::vector<std::pair<valuation_type, sum_builder_type>> save;
    save.reserve(head.size());

    // Used in test (F).
    std::vector<std::pair<valuation_type, sum_builder_type>> remainder;
    remainder.reserve(head.size());

    // Initialize res with the alpha of the first operand.
    for (auto& arc : head)
    {
      sum_builder_type succs;
      succs.add(arc.successor());
      res.emplace(arc.valuation(), std::move(succs));
    }

    // (A).
    for (++operands_cit; operands_cit != operands_end; ++operands_cit)
    {
      const auto res_end = res.end();

      const node_type& node = internal::mem::variant_cast<node_type>((*operands_cit)->data());
      const auto alpha_end = node.end();

      // (B). For each arc of the current operand.
      for (auto alpha_cit = node.begin(); alpha_cit != alpha_end; ++alpha_cit)
      {
        // The current valuation may be modified, we need a copy.
        valuation_type current_val = alpha_cit->valuation();
        const SDD<C> current_succ = alpha_cit->successor();

        // Initialize the start of the next search.
        auto res_cit = res.begin();

        // (C). While the current valuation is not empty, test it against arcs in res.
        while (not current_val.empty() and res_cit != res_end)
        {
          const valuation_type& res_val = res_cit->first;
          sum_builder_type& res_succs = res_cit->second;

          // (D).
          if (current_val == res_val) // Same valuations.
          {
            save.emplace_back(res_val, std::move(res_succs));
            save.back().second.add(current_succ);
            const auto to_erase = res_cit;
            ++res_cit;
            res.erase(to_erase);
            // Avoid useless insertion or temporary variables.
            goto equality;
          }

          intersection_builder<C, valuation_type> inter_builder;
          inter_builder.add(current_val);
          inter_builder.add(res_val);
          const valuation_type inter = intersection(base_type::cxt_, std::move(inter_builder));

          // (E). The current valuation and the current arc from res have a common part.
          if (not inter.empty())
          {
            save.emplace_back(inter, res_succs);
            save.back().second.add(current_succ);

            // (F).
            valuation_type diff = difference(base_type::cxt_, res_val, inter);
            if (not diff.empty())
            {
              // (res_val - inter) can't be in intersection, but we need to keep it
              // for the next arcs of the current alpha. So we put in a temporary storage.
              // It will be added back in res when we have finished with the current valuation.
              remainder.emplace_back(std::move(diff), std::move(res_succs));
            }

            // We won't need the current arc of res for the current val, we already have the
            // common part. Now, the current valuation has to be tested against the next arcs
            // of res.
            const auto to_erase = res_cit;
            ++res_cit;
            res.erase(to_erase);

            // (G). The current valuation is completely included in the current arc of res.
            if (current_val == inter)
            {
              // We can move to the next arc of the current operand.
              break;
            }

            // Continue with what remains of val. if val is empy, the loop will stop at the
            // next iteration.
            current_val = difference(base_type::cxt_, current_val, inter);
          }
          else // (H). Empty intersection, lookup for next possible common parts.
          {
            ++res_cit;
          }
        } // While we're not at the end of res and val is not empty.

        // (I). For val or a part of val (it could have been modified during the previous
        // loop), we didn't find an intersection with any arc of res.
        if (not current_val.empty())
        {
          sum_builder_type succs;
          succs.add(current_succ);
          save.emplace_back(std::move(current_val), std::move(succs));
        }

        // Both arcs had the same valuation.
        equality:;

        // Reinject all parts that were removed in (F).
        for (auto& rem : remainder)
        {
          res.emplace(std::move(rem.first), std::move(rem.second));
        }
        remainder.clear();

      } // For each arc of the current operand.

      // Reinject all parts that were removed from res (all parts that have a non-empty
      // intersection with the current alpha) and all parts of the current alpha that have an
      // empty intersection with all the parts of res.
      res.insert(save.begin(), save.end());

      // We still need save.
      save.clear();
    } // End of iteration on operands.

    square_union<C, valuation_type> su;
    su.reserve(res.size());
    for (auto& arc : res)
    {
      // construct an operand for the square union: (successors union) --> valuation
      su.add(sum(base_type::cxt_, std::move(arc.second)), arc.first);
    }

    return SDD<C>(head.variable(), su(base_type::cxt_));
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Add an arc to the operands of the sum operation.
///
/// This implementation is meant to be used as a policy by nary_builder which doesn't know how
/// to add an arc.
template <typename C, typename Valuation>
struct _LIBSDD_ATTRIBUTE_PACKED sum_builder_impl
{
  void
  add(boost::container::flat_set<Valuation>& set, Valuation&& operand)
  {
    if (not operand.empty())
    {
      set.insert(std::move(operand));
    }
  }

  void
  add(boost::container::flat_set<Valuation>& set, const Valuation& operand)
  {
    if (not operand.empty())
    {
      set.insert(operand);
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief   The sum operation of a set of SDD.
/// @related SDD
template <typename C>
inline
SDD<C>
sum(context<C>& cxt, sum_builder<C, SDD<C>>&& builder)
{
  if (builder.empty())
  {
    return zero<C>();
  }
  else if (builder.size() == 1)
  {
    return *builder.begin();
  }
  return cxt.sum_cache()(sum_op<C>(cxt, builder));
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief   The sum operation of a set of values.
/// @details A wrapper around the implementation of sum provided by Values.
template <typename C, typename Values>
inline
Values
sum(context<C>&, sum_builder<C, Values>&& builder)
{
  if (builder.empty())
  {
    return Values();
  }
  else if (builder.size() == 1)
  {
    return *builder.begin();
  }
  else
  {
    auto cit = builder.begin();
    const auto end = builder.end();
    Values result = *cit;
    for (++cit; cit != end; ++cit)
    {
      typename C::Values tmp = sum(result, *cit);
      using std::swap;
      swap(tmp, result);
    }
    return result;
  }
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Perform the union of two SDD.
/// @related SDD
template <typename C>
inline
SDD<C>
operator+(const SDD<C>& lhs, const SDD<C>& rhs)
{
  return sum(initial_context<C>(), {lhs, rhs});
}

/// @brief Perform the union of two SDD.
/// @related SDD
template <typename C>
inline
SDD<C>&
operator+=(SDD<C>& lhs, const SDD<C>& rhs)
{
  SDD<C> tmp = sum(initial_context<C>(), {lhs, rhs});
  using std::swap;
  swap(tmp, lhs);
  return lhs;
}

/// @brief Perform the union of an iterable container of SDD.
/// @related SDD
template <typename C, typename InputIterator>
SDD<C>
inline
sum(InputIterator begin, InputIterator end)
{
  sum_builder<C, SDD<C>> builder;
  for (; begin != end; ++begin)
  {
    builder.add(*begin);
  }
  return sum(initial_context<C>(), std::move(builder));
}

/// @brief   Perform the union of an initializer list of SDD.
/// @related SDD
template <typename C>
SDD<C>
inline
sum(std::initializer_list<SDD<C>> operands)
{
  return sum<C>(std::begin(operands), std::end(operands));
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::dd::sum_op
template <typename C>
struct hash<sdd::sum_op<C>>
{
  std::size_t
  operator()(const sdd::sum_op<C>& sum)
  const noexcept
  {
    std::size_t seed = 0;
    for (const auto& operand : sum)
    {
      sdd::internal::util::hash_combine(seed, operand);
    }
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_DD_SUM_HH_
