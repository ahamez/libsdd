#ifndef _SDD_HOM_LOCAL_HH_
#define _SDD_HOM_LOCAL_HH_

#include <iosfwd>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation_error.hh"
#include "sdd/hom/identity.hh"
#include "sdd/order/order.hh"
#include "sdd/util/packed.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief local homomorphism.
template <typename C>
class LIBSDD_ATTRIBUTE_PACKED _local
{
private:

  /// @brief The target of this homomorphism.
  const order_position_type target_;

  /// @brief The nested homomorphism to apply in a nested level.
  const homomorphism<C> h_;

public:

  /// @brief Constructor.
  _local(order_position_type target, const homomorphism<C>& h)
    : target_(target), h_(h)
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
    const SDD<C> sdd_; // needed if an evaluation_error is throwed

    evaluation(context<C>& cxt, const order<C>& o, const homomorphism<C>& h, const SDD<C>& s)
      : cxt_(cxt), order_(o), h_(h), sdd_(s)
    {}

    /// @brief Hierarchical nodes case.
    SDD<C>
    operator()(const hierarchical_node<C>& node)
    const
    {
      try
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
      catch (top<C>& t)
      {
        evaluation_error<C> e(sdd_);
        e.add_top(t);
        throw e;
      }
    }

    /// @brief Error case: local only applies on hierarchical nodes.
    template <typename T>
    SDD<C>
    operator()(const T&)
    const
    {
      throw evaluation_error<C>(sdd_);
    }
  };

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& s)
  const
  {
    return visit(evaluation{cxt, o, h_, s}, s);
  }

  /// @brief Skip predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return o.position() != target_;
  }

  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return h_.selector();
  }

  /// @brief Return the target.
  order_position_type
  target()
  const noexcept
  {
    return target_;
  }

  /// @brief Return the carried homomorphism.
  homomorphism<C>
  hom()
  const noexcept
  {
    return h_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related _local
template <typename C>
inline
bool
operator==(const _local<C>& lhs, const _local<C>& rhs)
noexcept
{
  return lhs.target() == rhs.target() and lhs.hom() == rhs.hom();
}

/// @internal
/// @related _local
template <typename C>
std::ostream&
operator<<(std::ostream& os, const _local<C>& l)
{
  return os << "@(" << l.target() << ", " << l.hom() << ")";
}

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
    std::size_t seed = sdd::util::hash(l.target());
    sdd::util::hash_combine(seed, l.hom());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_LOCAL_HH_
