#ifndef _SDD_HOM_SATURATION_SUM_HH_
#define _SDD_HOM_SATURATION_SUM_HH_

#include <algorithm>  // all_of, copy
#include <iosfwd>
#include <stdexcept>  //invalid_argument

#include <boost/container/flat_set.hpp>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation_error.hh"
#include "sdd/hom/identity.hh"
#include "sdd/hom/local.hh"
#include "sdd/hom/optional_homomorphism.hh"
#include "sdd/hom/sum.hh"
#include "sdd/order/order.hh"
#include "sdd/util/packed.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Saturation Sum homomorphism.
template <typename C>
class LIBSDD_ATTRIBUTE_PACKED saturation_sum
{
public:

  /// @brief The variable type.
  using variable_type = typename C::variable_type;

  /// @brief The type of the homomorphism G part.
  using g_type = boost::container::flat_set<homomorphism<C>>;

private:

  /// @brief The variable on which this sum works.
  const variable_type variable_;

  /// @brief The homomorphism's F part.
  const optional_homomorphism<C> F_;

  /// @brief The homomorphism's G part.
  const g_type G_;

  /// @brief The homomorphism's L part.
  const optional_homomorphism<C> L_;

public:

  /// @brief Constructor.
  saturation_sum( variable_type var, optional_homomorphism<C>&& f, g_type&& g
                , optional_homomorphism<C>&& l)
    : variable_(var), F_(std::move(f)), G_(std::move(g)), L_(std::move(l))
  {}

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& s)
  const
  {
    dd::sum_builder<C, SDD<C>> sum_operands(cxt.sdd_context());
    sum_operands.reserve(G_.size() + 2);

    if (F_)
    {
      sum_operands.add((*F_)(cxt, o, s));
    }

    for (const auto& g : G_)
    {
      sum_operands.add(g(cxt, o, s));
    }

    if (L_)
    {
      sum_operands.add((*L_)(cxt, o, s));
    }

    try
    {
      return dd::sum(cxt.sdd_context(), std::move(sum_operands));
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
  const g_type&
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
/// @brief Equality of two saturation_sum.
/// @related saturation_sum
template <typename C>
inline
bool
operator==(const saturation_sum<C>& lhs, const saturation_sum<C>& rhs)
noexcept
{
  return lhs.variable() == rhs.variable()
     and lhs.F() == rhs.F()
     and lhs.L() == rhs.L()
     and lhs.G() == rhs.G();
}

/// @internal
/// @related saturation_sum
template <typename C>
std::ostream&
operator<<(std::ostream& os, const saturation_sum<C>& s)
{
  os << "SatSum(@" << +s.variable() << ", F=";
  if (s.F())
  {
    os << *s.F();
  }
  os << ", G=";
  if (not s.G().empty())
  {
    std::copy( s.G().begin(), std::prev(s.G().end())
             , std::ostream_iterator<homomorphism<C>>(os, " + "));
    os << *std::prev(s.G().end()) << ")";
  }
  os << ", L=";
  if (s.L())
  {
    os << *s.L();
  }
  return os;
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Create the Saturation Sum homomorphism.
/// @related sdd::homomorphism
///
/// We suppose that a saturation sum is created in the rewriting process. Thus, we assume that
/// operands are already optimized (local merged, etc.).
template <typename C, typename InputIterator>
homomorphism<C>
SaturationSum( typename C::variable_type var
             , optional_homomorphism<C>&& f
             , InputIterator gbegin, InputIterator gend
             , optional_homomorphism<C>&& l)
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

  return homomorphism<C>::create( mem::construct<saturation_sum<C>>()
                                , var
                                , std::move(f)
                                , typename saturation_sum<C>::g_type(gbegin, gend)
                                , std::move(l));
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::saturation_sum.
template <typename C>
struct hash<sdd::hom::saturation_sum<C>>
{
  std::size_t
  operator()(const sdd::hom::saturation_sum<C>& s)
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

#endif // _SDD_HOM_SATURATION_SUM_HH_
