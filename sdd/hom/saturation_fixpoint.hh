#ifndef _SDD_HOM_SATURATION_FIXPOINT_HH_
#define _SDD_HOM_SATURATION_FIXPOINT_HH_

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
#include "sdd/order/order.hh"
#include "sdd/util/packed.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Saturation Fixpoint homomorphism.
template <typename C>
class LIBSDD_ATTRIBUTE_PACKED saturation_fixpoint
{
public:

  /// @brief The variable type.
  typedef typename C::Variable variable_type;

  /// @brief The type of the homomorphism G part.
  typedef boost::container::flat_set<homomorphism<C>> g_type;

private:

  /// @brief The variable on which this sum works.
  const variable_type variable_;

  /// @brief The homomorphism's F part.
  const homomorphism<C> F_;

  /// @brief The homomorphism's G part.
  const g_type G_;

  /// @brief The homomorphism's L part.
  const homomorphism<C> L_;

public:

  /// @brief Constructor.
  saturation_fixpoint( const variable_type& var, const homomorphism<C>& f, g_type&& g
                     , const homomorphism<C>& l)
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
    SDD<C> s1 = s;
    SDD<C> s2 = s;

    do
    {
      s1 = s2;

      s2 = F_(cxt, o, s2); // apply (F + Id)*
      s2 = L_(cxt, o, s2); // apply (L + Id)*

      for (const auto& g : G_)
      {
        try
        {
          // chain applications of G
          s2 = dd::sum(cxt.sdd_context(), {s2, g(cxt, o, s2)});
        }
        catch (top<C>& t)
        {
          evaluation_error<C> e(s);
          e.add_top(t);
          throw e;
        }
      }
    } while (s1 != s2);

    return s1;
  }

  /// @brief Skip predicate.
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
    return F_.selector()
       and L_.selector()
       and std::all_of( G_.begin(), G_.end()
                      , [](const homomorphism<C>& h){return h.selector();});
  }

  /// @brief Get the targeted variable.
  const variable_type&
  variable()
  const noexcept
  {
    return variable_;
  }

  /// @brief Get the forwardable part.
  homomorphism<C>
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
  homomorphism<C>
  L()
  const noexcept
  {
    return L_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Equality of two saturation_fixpoint.
/// @related saturation_fixpoint
template <typename C>
inline
bool
operator==(const saturation_fixpoint<C>& lhs, const saturation_fixpoint<C>& rhs)
noexcept
{
  return lhs.variable() == rhs.variable()
     and lhs.F() == rhs.F()
     and lhs.L() == rhs.L()
     and lhs.G() == rhs.G();
}

/// @internal
/// @related saturation_fixpoint
template <typename C>
std::ostream&
operator<<(std::ostream& os, const saturation_fixpoint<C>& s)
{
  os << "Sat(@" << s.variable() << ",  " << s.F() << " + " << s.L();
  if (not s.G().empty())
  {
    os << " + ";
    std::copy( s.G().begin(), std::prev(s.G().end())
             , std::ostream_iterator<homomorphism<C>>(os, " + "));
    os << *std::prev(s.G().end()) << ")*";
  }
  return os;
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Create the Saturation Fixpoint homomorphism.
/// @related homomorphism
///
/// We suppose that a saturation fixpoint is created in the rewriting process. Thus, we assume
/// that operands of the G part are already optimized (e.g. local merged and sums flatten).
template <typename C, typename InputIterator>
homomorphism<C>
SaturationFixpoint( const typename C::Variable& var
                  , const homomorphism<C>& f
                  , InputIterator gbegin, InputIterator gend
                  , const homomorphism<C>& l)
{
  const std::size_t gsize = std::distance(gbegin, gend);

  if (f != Id<C>() and gsize == 0 and l == Id<C>())
  {
    return f;
  }

  if (f == Id<C>() and gsize == 0 and l != Id<C>())
  {
    return l;
  }

  return homomorphism<C>::create( mem::construct<saturation_fixpoint<C>>()
                                , var
                                , f
                                , typename saturation_fixpoint<C>::g_type(gbegin, gend)
                                , l);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::saturation_fixpoint.
template <typename C>
struct hash<sdd::hom::saturation_fixpoint<C>>
{
  std::size_t
  operator()(const sdd::hom::saturation_fixpoint<C>& s)
  const noexcept
  {
    std::size_t seed = 0;
    sdd::util::hash_combine(seed, s.variable());
    sdd::util::hash_combine(seed, s.F());
    sdd::util::hash_combine(seed, s.L());
    for (const auto& g : s.G())
    {
      sdd::util::hash_combine(seed, g);
    }
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_SATURATION_FIXPOINT_HH_
