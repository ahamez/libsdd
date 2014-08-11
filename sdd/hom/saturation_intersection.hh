#ifndef _SDD_HOM_SATURATION_INTERSECTION_HH_
#define _SDD_HOM_SATURATION_INTERSECTION_HH_

#include <algorithm>  // all_of, copy
#include <iosfwd>
#include <stdexcept>  //invalid_argument

#include "sdd/dd/definition.hh"
#include "sdd/hom/common_types.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/identity.hh"
#include "sdd/hom/intersection.hh"
#include "sdd/hom/interrupt.hh"
#include "sdd/hom/local.hh"
#include "sdd/order/order.hh"
#include "sdd/util/packed.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Saturation intersection homomorphism.
template <typename C>
struct LIBSDD_ATTRIBUTE_PACKED _saturation_intersection
{
  /// @brief The variable type.
  using variable_type = typename C::variable_type;

  /// @brief The variable on which this intersection works.
  const variable_type variable;

  /// @brief The homomorphism's F part.
  const optional_homomorphism<C> F;

  /// @brief The homomorphism's G part.
  const homomorphism_set<C> G;

  /// @brief The homomorphism's L part.
  const optional_homomorphism<C> L;

  /// @brief Constructor.
  _saturation_intersection( variable_type var, optional_homomorphism<C>&& f, homomorphism_set<C>&& g
                          , optional_homomorphism<C>&& l)
    : variable(var), F(std::move(f)), G(std::move(g)), L(std::move(l))
  {}

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& s)
  const
  {
    dd::intersection_builder<C, SDD<C>> operands(cxt.sdd_context());
    operands.reserve(G.size() + 2);

    if (F)
    {
      operands.add((*F)(cxt, o, s));
    }

    for (const auto& g : G)
    {
      operands.add(g(cxt, o, s));
    }

    if (L)
    {
      operands.add((*L)(cxt, o, s));
    }
    return dd::intersection(cxt.sdd_context(), std::move(operands));
  }

  /// @brief Skip variable predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return variable != o.variable();
  }

  /// @brief Selector predicate.
  bool
  selector()
  const noexcept
  {
    return (F ? F->selector() : true) and (L ? L->selector() : true)
       and std::all_of( G.begin(), G.end()
                      , [&](const homomorphism<C>& h){return h.selector();});
  }

  friend
  bool
  operator==(const _saturation_intersection& lhs, const _saturation_intersection& rhs)
  noexcept
  {
    return lhs.variable == rhs.variable and lhs.F == rhs.F and lhs.L == rhs.L
       and lhs.G == rhs.G;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const _saturation_intersection& s)
  {
    os << "SatInter(@" << +s.variable << ", F=";
    if (s.F)
    {
      os << *s.F;
    }
    os << ", G=";
    if (not s.G.empty())
    {
      std::copy( s.G.begin(), std::prev(s.G.end())
               , std::ostream_iterator<homomorphism<C>>(os, " & "));
      os << *std::prev(s.G.end()) << ")";
    }
    os << ", L=";
    if (s.L)
    {
      os << *s.L;
    }
    return os << ")";
  }

};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Create the Saturation intersection homomorphism.
/// @related sdd::homomorphism
///
/// We suppose that a saturation intersection is created in the rewriting process. Thus, we assume
/// that operands are already optimized (local merged, etc.).
template <typename C, typename InputIterator>
homomorphism<C>
saturation_intersection( typename C::variable_type var
                       , optional_homomorphism<C>&& f
                       , InputIterator gbegin, InputIterator gend
                       , optional_homomorphism<C>&& l)
{
  if (std::distance(gbegin, gend) == 0)
  {
    if (f and not l)
    {
      return *f;
    }
    if (not f and l)
    {
      return *l;
    }
  }

  return homomorphism<C>::create( mem::construct<_saturation_intersection<C>>()
                                , var
                                , std::move(f)
                                , homomorphism_set<C>(gbegin, gend)
                                , std::move(l));
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::_saturation_intersection.
template <typename C>
struct hash<sdd::hom::_saturation_intersection<C>>
{
  std::size_t
  operator()(const sdd::hom::_saturation_intersection<C>& s)
  const
  {
    std::size_t seed = sdd::util::hash(s.variable);
    if (s.F)
    {
      sdd::util::hash_combine(seed, *s.F);
    }
    if (s.L)
    {
      sdd::util::hash_combine(seed, *s.L);
    }
    sdd::util::hash_combine(seed, s.G.begin(), s.G.end());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_SATURATION_INTERSECTION_HH_
