#ifndef _SDD_HOM_SATURATION_INTERSECTION_HH_
#define _SDD_HOM_SATURATION_INTERSECTION_HH_

#include <algorithm>  // all_of, copy
#include <iosfwd>
#include <stdexcept>  //invalid_argument

#include "sdd/dd/definition.hh"
#include "sdd/hom/common_types.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation_error.hh"
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
class LIBSDD_ATTRIBUTE_PACKED _saturation_intersection
{
public:

  /// @brief The variable type.
  using variable_type = typename C::variable_type;

private:

  /// @brief The variable on which this intersection works.
  const variable_type variable_;

  /// @brief The homomorphism's F part.
  const optional_homomorphism<C> F_;

  /// @brief The homomorphism's G part.
  const homomorphism_set<C> G_;

  /// @brief The homomorphism's L part.
  const optional_homomorphism<C> L_;

public:

  /// @brief Constructor.
  _saturation_intersection( variable_type var, optional_homomorphism<C>&& f, homomorphism_set<C>&& g
                          , optional_homomorphism<C>&& l)
    : variable_(var), F_(std::move(f)), G_(std::move(g)), L_(std::move(l))
  {}

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& s)
  const
  {
    dd::intersection_builder<C, SDD<C>> operands(cxt.sdd_context());
    operands.reserve(G_.size() + 2);

    try
    {
      try
      {
        if (F_)
        {
          operands.add((*F_)(cxt, o, s));
        }

        for (const auto& g : G_)
        {
          operands.add(g(cxt, o, s));
        }

        if (L_)
        {
          operands.add((*L_)(cxt, o, s));
        }
      }
      catch (interrupt<C>& i)
      {
        operands.add(i.result());
        i.result() = dd::intersection(cxt.sdd_context(), std::move(operands));
        throw;
      }
      return dd::intersection(cxt.sdd_context(), std::move(operands));
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
    return variable_ != o.variable();
  }

  /// @brief Selector predicate.
  bool
  selector()
  const noexcept
  {
    return (F_ ? F_->selector() : true)
       and (L_ ? L_->selector() : true)
       and std::all_of( G_.begin(), G_.end()
                      , [&](const homomorphism<C>& h){return h.selector();});
  }

  /// @brief Get the targeted variable.
  variable_type
  variable()
  const noexcept
  {
    return variable_;
  }

  /// @brief Get the forwardable part.
  const optional_homomorphism<C>&
  F()
  const noexcept
  {
    return F_;
  }

  /// @brief Get the global part.
  const homomorphism_set<C>&
  G()
  const noexcept
  {
    return G_;
  }

  /// @brief Get the local part.
  const optional_homomorphism<C>&
  L()
  const noexcept
  {
    return L_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related _saturation_intersection
template <typename C>
inline
bool
operator==(const _saturation_intersection<C>& lhs, const _saturation_intersection<C>& rhs)
noexcept
{
  return lhs.variable() == rhs.variable() and lhs.F() == rhs.F() and lhs.L() == rhs.L()
     and lhs.G() == rhs.G();
}

/// @internal
/// @related _saturation_intersection
template <typename C>
std::ostream&
operator<<(std::ostream& os, const _saturation_intersection<C>& s)
{
  os << "SatInter(@" << +s.variable() << ", F=";
  if (s.F())
  {
    os << *s.F();
  }
  os << ", G=";
  if (not s.G().empty())
  {
    std::copy( s.G().begin(), std::prev(s.G().end())
             , std::ostream_iterator<homomorphism<C>>(os, " & "));
    os << *std::prev(s.G().end()) << ")";
  }
  os << ", L=";
  if (s.L())
  {
    os << *s.L();
  }
  return os << ")";
}

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
    std::size_t seed = sdd::util::hash(s.variable());
    if (s.F())
    {
      sdd::util::hash_combine(seed, *s.F());
    }
    if (s.L())
    {
      sdd::util::hash_combine(seed, *s.L());
    }
    sdd::util::hash_combine(seed, s.G().begin(), s.G().end());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_SATURATION_INTERSECTION_HH_
