#ifndef _SDD_HOM_SATURATION_SUM_HH_
#define _SDD_HOM_SATURATION_SUM_HH_

#include <algorithm>  // all_of, copy
#include <iosfwd>
#include <stdexcept>  //invalid_argument

#include "sdd/dd/definition.hh"
#include "sdd/hom/common_types.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation_error.hh"
#include "sdd/hom/identity.hh"
#include "sdd/hom/interrupt.hh"
#include "sdd/hom/local.hh"
#include "sdd/hom/sum.hh"
#include "sdd/order/order.hh"
#include "sdd/util/packed.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Saturation sum homomorphism.
template <typename C>
struct LIBSDD_ATTRIBUTE_PACKED _saturation_sum
{
  /// @brief The variable type.
  using variable_type = typename C::variable_type;

  /// @brief The variable on which this sum works.
  const variable_type variable;

  /// @brief The homomorphism's F part.
  const optional_homomorphism<C> F;

  /// @brief The homomorphism's G part.
  const homomorphism_set<C> G;

  /// @brief The homomorphism's L part.
  const optional_homomorphism<C> L;

  /// @brief Constructor.
  _saturation_sum( variable_type var, optional_homomorphism<C>&& f, homomorphism_set<C>&& g
                 , optional_homomorphism<C>&& l)
    : variable(var), F(std::move(f)), G(std::move(g)), L(std::move(l))
  {}

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& s)
  const
  {
    dd::sum_builder<C, SDD<C>> operands(cxt.sdd_context());
    operands.reserve(G.size() + 2);

    try
    {
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
      return dd::sum(cxt.sdd_context(), std::move(operands));
    }
    catch (top<C>& t)
    {
      evaluation_error<C> e(s);
      e.add_top(t);
      throw e;
    }
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
  operator==(const _saturation_sum& lhs, const _saturation_sum& rhs)
  noexcept
  {
    return lhs.variable == rhs.variable and lhs.F == rhs.F and lhs.L == rhs.L and lhs.G == rhs.G;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const _saturation_sum& s)
  {
    os << "SatSum(@" << +s.variable << ", F=";
    if (s.F)
    {
      os << *s.F;
    }
    os << ", G=";
    if (not s.G.empty())
    {
      std::copy( s.G.begin(), std::prev(s.G.end())
               , std::ostream_iterator<homomorphism<C>>(os, " + "));
      os << *std::prev(s.G.end()) << ")";
    }
    os << ", L=";
    if (s.L)
    {
      os << *s.L;
    }
    return os;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Create the Saturation sum homomorphism.
/// @related sdd::homomorphism
///
/// We suppose that a saturation sum is created in the rewriting process. Thus, we assume that
/// operands are already optimized (local merged, etc.).
template <typename C, typename InputIterator>
homomorphism<C>
saturation_sum( typename C::variable_type var, optional_homomorphism<C>&& f
              , InputIterator gbegin, InputIterator gend, optional_homomorphism<C>&& l)
{
  const std::size_t g_size = std::distance(gbegin, gend);

  if (g_size == 0)
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

  return homomorphism<C>::create( mem::construct<_saturation_sum<C>>()
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
/// @brief Hash specialization for sdd::hom::_saturation_sum.
template <typename C>
struct hash<sdd::hom::_saturation_sum<C>>
{
  std::size_t
  operator()(const sdd::hom::_saturation_sum<C>& s)
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

#endif // _SDD_HOM_SATURATION_SUM_HH_
