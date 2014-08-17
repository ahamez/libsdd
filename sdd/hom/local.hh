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
  /// @brief The target of this homomorphism.
  const order_position_type target;

  /// @brief The nested homomorphism to apply in a nested level.
  const homomorphism<C> h;

  /// @brief Constructor.
  _local(order_position_type t, const homomorphism<C>& hm)
    : target(t), h(hm)
  {}

  /// @internal
  /// @brief Local's evaluation implementation.
  struct evaluation
  {
    /// @brief Used by variant.
    using result_type = SDD<C>;

    context<C>& cxt_;
    const order<C>& order_;
    const homomorphism<C> h_;

    evaluation(context<C>& cxt, const order<C>& o, const homomorphism<C>& op)
      : cxt_(cxt), order_(o), h_(op)
    {}

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
          const SDD<C> new_valuation = h_(cxt_, order_.nested(), arc.valuation());
          if (not new_valuation.empty())
          {
            su.add(arc.successor(), new_valuation);
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
          const SDD<C> new_valuation = h_(cxt_, order_.nested(), arc.valuation());
          sum_operands.add(SDD<C>(node.variable(), new_valuation, arc.successor()));
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
      assert(false && "Local applied on a flat node");
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
    return o.position() != target;
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
local(order_position_type pos, const homomorphism<C>& h)
{
  if (h == id<C>())
  {
    return h;
  }
  else
  {
    return homomorphism<C>::create(mem::construct<hom::_local<C>>(), pos, h);
  }
}

/// @brief Create the local homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
local(const typename C::Identifier& id, const order<C>& o, const homomorphism<C>& h)
{
  /// @todo Check that id is a hierarchical identifier.
  return local(o.node(id).position(), h);
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
    std::size_t seed = sdd::util::hash(l.target);
    sdd::util::hash_combine(seed, l.h);
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
