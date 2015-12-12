/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <iosfwd>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/identity.hh"
#include "sdd/order/order.hh"
#include "sdd/util/packed.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief local homomorphism.
template <typename C>
struct LIBSDD_ATTRIBUTE_PACKED _local
{
  /// @brief The identifier on which the user function is applied.
  const typename C::variable_type target;

  /// @brief The nested homomorphism to apply in a nested level.
  const homomorphism<C> h;

  /// @internal
  /// @brief Local's evaluation implementation.
  struct evaluation
  {
    context<C>& cxt_;
    const order<C>& order_;
    const homomorphism<C> h_;

    /// @brief Hierarchical nodes case.
    SDD<C>
    operator()(const hierarchical_node<C>& node)
    const
    {
      if (h_.selector()) // partition won't change
      {
        dd::square_union<C, SDD<C>> su(cxt_.sdd_context());
        su.reserve(node.size());
        for (const auto& arc : node)
        {
          auto new_valuation = h_(cxt_, order_.nested(), arc.valuation());
          if (not new_valuation.empty())
          {
            su.add(arc.successor(), std::move(new_valuation));
          }
        }
        return {node.variable(), su()};
      }
      else // partition will change
      {
        dd::sum_builder<C, SDD<C>> sum_operands(cxt_.sdd_context());
        sum_operands.reserve(node.size());
        for (const auto& arc : node)
        {
          auto new_valuation = h_(cxt_, order_.nested(), arc.valuation());
          sum_operands.add(SDD<C>(node.variable(), std::move(new_valuation), arc.successor()));
        }
        return dd::sum(cxt_.sdd_context(), std::move(sum_operands));
      }
    }

    /// @brief Error case: local only applies on hierarchical nodes.
    template <typename T>
    SDD<C>
    operator()(const T&)
    const
    {
      assert(false && "Local applied on a non-hierarchical node");
      __builtin_unreachable();
    }
  };

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& s)
  const
  {
    return visit(evaluation{cxt, o, h}, s);
  }

  /// @brief Skip predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return o.variable() != target;
  }

  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return h.selector();
  }

  friend
  bool
  operator==(const _local& lhs, const _local& rhs)
  noexcept
  {
    return lhs.target == rhs.target and lhs.h == rhs.h;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const _local& l)
  {
    return os << "@(" << l.target << ", " << l.h << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related homomorphism
template <typename C>
homomorphism<C>
local(typename C::variable_type var, const homomorphism<C>& h)
{
  return h == id<C>() ? h : hom::make<C, hom::_local<C>>(var, h);
}

/// @brief Create the local homomorphism.
/// @related homomorphism
///
/// One should not directly use this function, but preferably the carrier function. Also, note that
/// function automatically creates the succession of local necessary to apply on its target.
template <typename C>
homomorphism<C>
local(const typename C::Identifier& id, const order<C>& o, const homomorphism<C>& h)
{
  /// @todo Check that id is a hierarchical identifier.
  return local(o.node(id).variable(), h);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::_local.
template <typename C>
struct hash<sdd::hom::_local<C>>
{
  std::size_t
  operator()(const sdd::hom::_local<C>& l)
  const
  {
    using namespace sdd::hash;
    return seed(l.target) (val(l.h));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
