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
class _LIBSDD_ATTRIBUTE_PACKED local
{
private:

  /// @brief The variable type.
  typedef typename C::Variable variable_type;

  /// @brief The identifier type.
  typedef typename C::Identifier identifier_type;

  /// @brief The hierarchical node where the nested homomorphism will be carried to.
  const variable_type variable_;

  ///
  const identifier_type identifier_;

  /// @brief The nested homomorphism to apply in a nested level.
  const homomorphism<C> h_;

public:

  /// @brief Constructor.
  local(const identifier_type& id, const order::order<C>& o, const homomorphism<C>& h)
    : variable_(o.identifier_variable(id))
    , identifier_(id)
    , h_(h)
  {
  }

  /// @internal
  /// @brief Local's evaluation implementation.
  struct evaluation
  {
    /// @brief Used by variant.
    typedef SDD<C> result_type;

    SDD<C>
    operator()( const hierarchical_node<C>& node
              , context<C>& cxt, const order::order<C>& o, const variable_type& var
              , const homomorphism<C>& h, const SDD<C>& s)
    const
    {
      try
      {
        if (h.selector()) // partition won't change
        {
          dd::square_union<C, SDD<C>> su;
          su.reserve(node.size());
          for (const auto& arc : node)
          {
            SDD<C> new_valuation = h(cxt, o.nested(), arc.valuation());
            if (not new_valuation.empty())
            {
              su.add(arc.successor(), new_valuation);
            }
          }
          return SDD<C>(node.variable(), su(cxt.sdd_context()));
        }
        else
        {
          dd::sum_builder<C, SDD<C>> sum_operands(node.size());
          for (const auto& arc : node)
          {
            sum_operands.add(SDD<C>(var, h(cxt, o.nested(), arc.valuation()), arc.successor()));
          }
          return dd::sum(cxt.sdd_context(), std::move(sum_operands));
        }
      }
      catch (top<C>& t)
      {
        evaluation_error<C> e(s);
        e.add_top(t);
        throw e;
      }
    }

    template <typename T>
    SDD<C>
    operator()( const T&, context<C>&, const order::order<C>&, const variable_type&
              , const homomorphism<C>&, const SDD<C> s)
    const
    {
      throw evaluation_error<C>(s);
    }
  };

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order::order<C>& o, const SDD<C>& s)
  const
  {
    return apply_visitor(evaluation(), s->data(), cxt, o, variable_, h_, s);
  }

  /// @brief Skip predicate.
  bool
  skip(const order::order<C>& o)
  const noexcept
  {
    return o.variable() != variable_;
  }

  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return h_.selector();
  }

  /// @brief Return the target.
  const variable_type&
  variable()
  const noexcept
  {
    return variable_;
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
  return lhs.variable() == rhs.variable() and lhs.hom() == rhs.hom();
}

/// @internal
/// @related local
template <typename C>
std::ostream&
operator<<(std::ostream& os, const local<C>& l)
{
  return os << "@(" << l.variable() << ", " << l.hom() << ")";
}

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the Local homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
Local(const typename C::Identifier& id, const order::order<C>& o, const homomorphism<C>& h)
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
  const noexcept
  {
    std::size_t seed = 0;
    sdd::util::hash_combine(seed, l.variable());
    sdd::util::hash_combine(seed, l.hom());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_LOCAL_HH_
