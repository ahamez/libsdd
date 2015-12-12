/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <initializer_list>
#include <type_traits> // enable_if, is_same
#include <unordered_map>
#include <vector>

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

#include "sdd/internal_manager_fwd.hh"
#include "sdd/dd/check_compatibility.hh"
#include "sdd/dd/context_fwd.hh"
#include "sdd/dd/definition.hh"
#include "sdd/dd/nary.hh"
#include "sdd/dd/operations_fwd.hh"
#include "sdd/dd/square_union.hh"
#include "sdd/mem/linear_alloc.hh"
#include "sdd/util/hash.hh"
#include "sdd/values/empty.hh"
#include "sdd/values/values_traits.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The sum operation in the cache.
template <typename C>
struct LIBSDD_ATTRIBUTE_PACKED sum_op_impl
{
  /// @brief The textual representation of the union operator.
  static constexpr char symbol = '+';

  /// @brief Perform the SDD union algorithm.
  ///
  /// It's a so-called 'n-ary' union in the sense that we don't create intermediary SDD.
  /// Also, a lot of tests permit to break loops as soon as possible.
  template <typename InputIterator, typename NodeType>
  static
  std::enable_if_t< std::is_same<NodeType, hierarchical_node<C>>::value
                    or not values::values_traits<typename C::Values>::fast_iterable
                  , SDD<C>>
  work(InputIterator begin, InputIterator end, context<C>& cxt)
  {
    using node_type = NodeType;
    using valuation_type = typename node_type::valuation_type;

    mem::rewinder _(cxt.arena());

    auto operands_cit = begin;
    const auto operands_end = end;

    // Get the first operand as a node, we need it to initialize the algorithm.
    const node_type& head = mem::variant_cast<node_type>(**operands_cit);

    // Type of the list of successors for a valuation, to be merged with the union operation
    // right before calling the square union.
    using sum_builder_type = sum_builder<C, SDD<C>>;

    /// @todo Use intrusive hash map to save on memory allocations?
    // List all the successors for each valuation in the final alpha.
    std::unordered_map< valuation_type
                      , sum_builder_type
                      , std::hash<valuation_type>
                      , std::equal_to<valuation_type>
                      , mem::linear_alloc<std::pair<const valuation_type, sum_builder_type>>
                      >
      res( head.size(), std::hash<valuation_type>(), std::equal_to<valuation_type>()
         , mem::linear_alloc<std::pair<const valuation_type, sum_builder_type>>(cxt.arena()));

    using pair_type = std::pair<valuation_type, sum_builder_type>;

    // Needed to temporarily store arcs erased from res and arcs from the alpha visited in
    // the loop (B).
    std::vector<pair_type, mem::linear_alloc<pair_type>>
      save(mem::linear_alloc<pair_type>(cxt.arena()));
    save.reserve(head.size());

    // Used in test (F).
    std::vector<pair_type, mem::linear_alloc<pair_type>>
      remainder(mem::linear_alloc<pair_type>(cxt.arena()));
    remainder.reserve(head.size());

    // Initialize res with the alpha of the first operand.
    for (const auto& arc : head)
    {
      sum_builder_type succs(cxt);
      succs.add(arc.successor());
      res.emplace(arc.valuation(), std::move(succs));
    }

    // (A).
    for (++operands_cit; operands_cit != operands_end; ++operands_cit)
    {
      // Throw a Top if operands are incompatible (different types or different variables).
      check_compatibility(*begin, *operands_cit);

      const auto res_end = res.end();

      const node_type& node = mem::variant_cast<node_type>(**operands_cit);
      const auto alpha_end = node.end();

      // (B). For each arc of the current operand.
      for (auto alpha_cit = node.begin(); alpha_cit != alpha_end; ++alpha_cit)
      {
        // The current valuation may be modified, we need a copy.
        auto current_val = alpha_cit->valuation();
        auto current_succ = alpha_cit->successor();

        // Initialize the start of the next search.
        auto res_cit = res.begin();

        // (C). While the current valuation is not empty, test it against arcs in res.
        while (not values::empty_values(current_val) and res_cit != res_end)
        {
          const valuation_type& res_val = res_cit->first;
          sum_builder_type& res_succs = res_cit->second;

          // (D).
          if (current_val == res_val) // Same valuations.
          {
            save.emplace_back(res_val, std::move(res_succs));
            save.back().second.add(std::move(current_succ));
            res_cit = res.erase(res_cit);
            // Avoid useless insertion or temporary variables.
            goto equality;
          }

          intersection_builder<C, valuation_type> inter_builder(cxt);
          inter_builder.add(current_val);
          inter_builder.add(res_val);
          const valuation_type inter = intersection(cxt, std::move(inter_builder));

          // (E). The current valuation and the current arc from res have a common part.
          if (not values::empty_values(inter))
          {
            save.emplace_back(inter, res_succs);
            save.back().second.add(current_succ);

            // (F).
            valuation_type diff = difference(cxt, res_val, inter);
            if (not values::empty_values(diff))
            {
              // (res_val - inter) can't be in intersection, but we need to keep it
              // for the next arcs of the current alpha. So we put it in a temporary storage.
              // It will be added back in res when we have finished with the current valuation.
              remainder.emplace_back(std::move(diff), std::move(res_succs));
            }

            // We won't need the current arc of res for the current val, we already have the
            // common part. Now, the current valuation has to be tested against the next arcs
            // of res.
            res_cit = res.erase(res_cit);

            // (G). The current valuation is completely included in the current arc of res.
            if (current_val == inter)
            {
              // We can move to the next arc of the current operand.
              break;
            }

            // Continue with what remains of val. If val is empty, the loop will stop at the
            // next iteration.
            current_val = difference(cxt, current_val, inter);
          }
          else // (H). Empty intersection, lookup for next possible common parts.
          {
            ++res_cit;
          }
        } // While we're not at the end of res and val is not empty.

        // (I). For val or a part of val (it could have been modified during the previous
        // loop), we didn't find an intersection with any arc of res.
        if (not values::empty_values(current_val))
        {
          sum_builder_type succs(cxt);
          succs.add(std::move(current_succ));
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

    square_union<C, valuation_type> su(cxt);
    su.reserve(res.size());
    for (auto& arc : res)
    {
      // construct an operand for the square union: (successors union) --> valuation
      su.add(sum(cxt, std::move(arc.second)), std::move(arc.first));
    }

    return SDD<C>(head.variable(), su());
  }

  /// @brief Linear union of flat SDDs whose valuation are "fast iterable".
  template <typename InputIterator, typename NodeType>
  static
  std::enable_if_t< std::is_same<NodeType, flat_node<C>>::value
                    and values::values_traits<typename C::Values>::fast_iterable
                  , SDD<C>>
  work(InputIterator begin, InputIterator end, context<C>& cxt)
  {
    const auto& variable = mem::variant_cast<flat_node<C>>(**begin).variable();

    mem::rewinder _(cxt.arena());

    using values_type      = typename C::Values;
    using values_builder   = typename values::values_traits<values_type>::builder;
    using value_type       = typename values_type::value_type;
    using sum_builder_type = sum_builder<C, SDD<C>>;
    boost::container::flat_map< value_type, sum_builder_type, std::less<value_type>
                              , mem::linear_alloc<std::pair<value_type, sum_builder_type>>>
      value_to_succ( std::less<value_type>()
                   , mem::linear_alloc<std::pair<value_type, sum_builder_type>>(cxt.arena()));
    value_to_succ.reserve(std::distance(begin, end) * 2);

    for (auto cit = begin; cit != end; ++cit)
    {
      check_compatibility(*begin, *cit);

      const auto& node = mem::variant_cast<flat_node<C>>(**cit);
      for (const auto& arc : node)
      {
        const SDD<C> succ = arc.successor();
        for (const auto& value : arc.valuation())
        {
          const auto search = value_to_succ.find(value);
          if (search == value_to_succ.end())
          {
            value_to_succ.emplace_hint(search, value, sum_builder_type(cxt, {succ}));
          }
          else
          {
            search->second.add(succ);
          }
        }
      }
    }

    // The following is almost like the square union except that we use a values_builder to
    // efficiently create the valuation of an arc (rather than using a union).
    boost::container::flat_map< SDD<C>, values_builder, std::less<SDD<C>>
                              , mem::linear_alloc<std::pair<SDD<C>, values_builder>>>
      succ_to_value( std::less<SDD<C>>()
                   , mem::linear_alloc<std::pair<SDD<C>, values_builder>>(cxt.arena()));
    succ_to_value.reserve(value_to_succ.size());
    for (auto& value_succs : value_to_succ)
    {
      const SDD<C> succ = sum(cxt, std::move(value_succs.second));
      const auto search = succ_to_value.find(succ);
      if (search == succ_to_value.end())
      {
        values_builder tmp;
        tmp.insert(value_succs.first);
        succ_to_value.emplace_hint(search, std::move(succ), std::move(tmp));
      }
      else
      {
        search->second.insert(std::move(value_succs.first));
      }
    }

    alpha_builder<C, values_type> alpha(cxt);
    alpha.reserve(succ_to_value.size());
    for (auto& succ_values : succ_to_value)
    {
      alpha.add(values_type(std::move(succ_values.second)), std::move(succ_values.first));
    }

    return SDD<C>(variable, std::move(alpha));
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Add an arc to the operands of the sum operation.
struct sum_builder_policy
{
  template <typename Container, typename Valuation>
  void
  add(Container& set, Valuation&& operand)
  {
    if (not values::empty_values(operand))
    {
      set.insert(std::forward<Valuation>(operand));
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The sum operation of a set of SDD.
/// @related sdd::SDD
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
  return cxt.sum_cache()({builder});
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The sum operation of a set of values.
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

} // namespace dd

/*------------------------------------------------------------------------------------------------*/

/// @brief Perform the union of two SDD.
/// @related SDD
template <typename C>
inline
SDD<C>
operator+(const SDD<C>& lhs, const SDD<C>& rhs)
{
  return dd::sum( global<C>().sdd_context
                , dd::sum_builder<C, SDD<C>>(global<C>().sdd_context, {lhs, rhs}));
}

/// @brief Perform the union of two SDD.
/// @related SDD
template <typename C>
inline
SDD<C>&
operator+=(SDD<C>& lhs, const SDD<C>& rhs)
{
  SDD<C> tmp = dd::sum( global<C>().sdd_context
                      , dd::sum_builder<C, SDD<C>>(global<C>().sdd_context, {lhs, rhs}));
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
  dd::sum_builder<C, SDD<C>> builder(global<C>().sdd_context);
  for (; begin != end; ++begin)
  {
    builder.add(*begin);
  }
  return dd::sum(global<C>().sdd_context, std::move(builder));
}

/// @brief Perform the union of an initializer list of SDD.
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
