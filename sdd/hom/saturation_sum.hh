#ifndef _SDD_HOM_SATURATION_SUM_HH_
#define _SDD_HOM_SATURATION_SUM_HH_

#include <algorithm>  // all_of, copy
#include <iosfwd>
#include <stdexcept>  //invalid_argument

#include <boost/container/flat_set.hpp>
#include <boost/optional.hpp>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation_error.hh"
#include "sdd/hom/identity.hh"
#include "sdd/hom/local.hh"
#include "sdd/hom/sum.hh"
#include "sdd/order/order.hh"
#include "sdd/util/packed.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Saturation Sum homomorphism.
template <typename C>
class _LIBSDD_ATTRIBUTE_PACKED saturation_sum
{
public:

  /// @brief The type of an optional homomorphism.
  typedef boost::optional<homomorphism<C>> optional_type;

  /// @brief The variable type.
  typedef typename C::Variable variable_type;

  /// @brief The type of the homomorphism G part.
  typedef boost::container::flat_set<homomorphism<C>> g_type;

private:

  /// @brief The variable on which this sum works.
  const variable_type variable_;

  /// @brief The homomorphism's F part.
  const optional_type F_;

  /// @brief The homomorphism's G part.
  const g_type G_;

  /// @brief The homomorphism's L part.
  const optional_type L_;

public:

  /// @brief Constructor.
  saturation_sum( const variable_type& var, const optional_type& f, g_type&& g
                , const optional_type& l)
    : variable_(var)
    , F_(f)
    , G_(std::move(g))
    , L_(l)
  {
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order::order<C>& o, const SDD<C>& s)
  const
  {
    dd::sum_builder<C, SDD<C>> sum_operands(G_.size() + 2);

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
  skip(const order::order<C>& o)
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
  const variable_type&
  variable()
  const noexcept
  {
    return variable_;
  }

  /// @brief Get the forwardable part.
  const optional_type&
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
  const optional_type&
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
/// @related sum
template <typename C>
std::ostream&
operator<<(std::ostream& os, const saturation_sum<C>& s)
{
  os << "Sat(@" << (int)s.variable() << ",  ";
  if (s.F())
  {
    os << *s.F();
  }
  os << " + ";
  if (s.L())
  {
    os << *s.L();
  }
  if (not s.G().empty())
  {
    os << " + ";
    std::copy( s.G().begin(), std::prev(s.G().end())
             , std::ostream_iterator<homomorphism<C>>(os, " + "));
    os << *std::prev(s.G().end()) << ")";
  }
  return os;
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Create the Saturation Sum homomorphism.
/// @related homomorphism
///
/// We suppose that a saturation sum is created in the rewriting process. Thus, we assume that
/// operands of the G part are already optimized (local merged and sums flatten).
template <typename C, typename InputIterator>
homomorphism<C>
SaturationSum( const typename C::Variable& var
             , const typename saturation_sum<C>::optional_type& f
             , InputIterator gbegin, InputIterator gend
             , const typename saturation_sum<C>::optional_type& l)
{
  const std::size_t g_size = std::distance(gbegin, gend);

  if (f and g_size == 0 and not l)
  {
    return *f;
  }

  if (not f and g_size == 0 and l)
  {
    return *l;
  }

  return homomorphism<C>::create( mem::construct<saturation_sum<C>>()
                                , var
                                , f
                                , typename saturation_sum<C>::g_type(gbegin, gend)
                                , l);
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
  const noexcept
  {
    std::size_t seed = 0;
    sdd::util::hash_combine(seed, s.variable());
    if (s.F())
    {
      sdd::util::hash_combine(seed, *s.F());
    }
    if (s.L())
    {
      sdd::util::hash_combine(seed, *s.L());
    }
    for (const auto& g : s.G())
    {
      sdd::util::hash_combine(seed, g);
    }
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_SATURATION_SUM_HH_
