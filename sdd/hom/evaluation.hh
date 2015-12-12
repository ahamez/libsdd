/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <cassert>
#include <iosfwd>

#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/traits.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Evaluate an homomorphism.
template <typename C>
struct evaluation
{
  /// @brief Terminal |0| case.
  ///
  /// It shall never be called as the |0| case is handled by the evaluation of
  /// homomorphism::operator().
  template <typename H>
  SDD<C>
  operator()( const H&, const zero_terminal<C>&
            , const homomorphism<C>&, const SDD<C>&
            , context<C>&, const order<C>&)
  const noexcept
  {
    assert(false);
    __builtin_unreachable();
  }

  /// @brief Terminal |1| case.
  template <typename H>
  SDD<C>
  operator()( const H& h, const one_terminal<C>&
            , const homomorphism<C>&, const SDD<C>& x
            , context<C>& cxt, const order<C>& o)
  const
  {
    return h(cxt, o, x);
  }

  /// @brief Dispatch evaluation to the concrete homomorphism.
  ///
  /// Implement a part of the automatic saturation: evaluation is propagated on successors
  /// whenever possible.
  template <typename H, typename Node>
  SDD<C>
  operator()( const H& h, const Node& node
            , const homomorphism<C>& hom, const SDD<C>& x
            , context<C>& cxt, const order<C>& o)
  const
  {
    assert(not o.empty() && "Empty order.");
    assert(o.variable() == node.variable() && "Different variables in order and SDD.");

    if (h.skip(o))
    {
      // The evaluated homomorphism skips the current level. We can thus forward its application
      // to the following levels.
      mem::rewinder _(cxt.sdd_context().arena());
      dd::square_union<C, typename Node::valuation_type> su(cxt.sdd_context());
      su.reserve(node.size());
      for (const auto& arc : node)
      {
        SDD<C> new_successor = hom(cxt, o.next(), arc.successor());
        if (not new_successor.empty())
        {
          su.add(std::move(new_successor), arc.valuation());
        }
      }
      return {node.variable(), su()};
    }
    else
    {
      return h(cxt, o, x);
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The evaluation of an homomorphism in the cache.
template <typename C>
struct cached_homomorphism
{
  /// @brief The current order position.
  const order<C> ord;

  /// @brief The homomorphism to evaluate.
  const homomorphism<C> hom;

  /// @brief The homomorphism's operand.
  const SDD<C> sdd;

  /// @brief Launch the evaluation.
  ///
  /// Called by the cache.
  SDD<C>
  operator()(context<C>& cxt)
  const
  {
    return binary_visit(evaluation<C>{}, hom, sdd, hom, sdd, cxt, ord);
  }

  friend
  bool
  operator==(const cached_homomorphism& lhs, const cached_homomorphism& rhs)
  noexcept
  {
    return lhs.hom == rhs.hom and lhs.sdd == rhs.sdd;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Used by the cache as a filter to know if an homomorphism should be cached.
template <typename C>
struct should_cache
{
  /// @brief Dispatch to each homomorphism's trait.
  template <typename T>
  constexpr bool
  operator()(const T&)
  const noexcept
  {
    return homomorphism_traits<T>::should_cache;
  }

  /// @brief Application of should_cache.
  bool
  operator()(const cached_homomorphism<C>& ch)
  const noexcept
  {
    return visit(*this, ch.hom);
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::cached_homomorphism
///
/// We don't need to hash order as the SDD operand brings the same information.
template <typename C>
struct hash<sdd::hom::cached_homomorphism<C>>
{
  std::size_t
  operator()(const sdd::hom::cached_homomorphism<C>& ch)
  const
  {
    using namespace sdd::hash;
    return seed(ch.hom) (val(ch.sdd));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
