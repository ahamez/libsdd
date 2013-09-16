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
/// @brief Local homomorphism.
template <typename C>
class LIBSDD_ATTRIBUTE_PACKED local
{
private:

  /// @brief The identifier type.
  typedef typename C::Identifier identifier_type;

  ///
  const identifier_type identifier_;

  /// @brief The nested homomorphism to apply in a nested level.
  const homomorphism<C> h_;

public:

  /// @brief Constructor.
  local(const identifier_type& id, const order<C>&, const homomorphism<C>& h)
    : identifier_(id)
    , h_(h)
  {
  }

  /// @internal
  /// @brief Local's evaluation implementation.
  struct evaluation
  {
    /// @brief Used by variant.
    typedef SDD<C> result_type;

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
          dd::square_union<C, SDD<C>> su;
          su.reserve(node.size());
          for (const auto& arc : node)
          {
            const SDD<C> new_valuation = h_(cxt_, order_.nested(), arc.valuation());
            if (not new_valuation.empty())
            {
              su.add(arc.successor(), new_valuation);
            }
          }
          return {node.variable(), su(cxt_.sdd_context())};
        }
        else // partition will change
        {
          dd::sum_builder<C, SDD<C>> sum_operands(node.size());
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

    /// @brief Error case: Local only applies on hierarchical nodes.
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
    return o.identifier() != identifier_;
  }

  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return h_.selector();
  }

  /// @brief Return the target.
  const identifier_type&
  identifier()
  const noexcept
  {
    return identifier_;
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
/// @brief Equality of two Local homomorphisms.
/// @related local
template <typename C>
inline
bool
operator==(const local<C>& lhs, const local<C>& rhs)
noexcept
{
  return lhs.identifier() == rhs.identifier() and lhs.hom() == rhs.hom();
}

/// @internal
/// @related local
template <typename C>
std::ostream&
operator<<(std::ostream& os, const local<C>& l)
{
  return os << "@(" << l.identifier() << ", " << l.hom() << ")";
}

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the Local homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
Local(const typename C::Identifier& id, const order<C>& o, const homomorphism<C>& h)
{
  if (h == Id<C>())
  {
    return h;
  }
  else
  {
    return homomorphism<C>::create(mem::construct<hom::local<C>>(), id, o, h);
  }
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::local.
template <typename C>
struct hash<sdd::hom::local<C>>
{
  std::size_t
  operator()(const sdd::hom::local<C>& l)
  const
  {
    std::size_t seed = 0;
    sdd::util::hash_combine(seed, l.identifier());
    sdd::util::hash_combine(seed, l.hom());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_LOCAL_HH_
