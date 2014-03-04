#ifndef _SDD_HOM_SATURATION_FIXPOINT_HH_
#define _SDD_HOM_SATURATION_FIXPOINT_HH_

#include <algorithm>  // all_of, copy, equal
#include <iosfwd>
#include <stdexcept>  // invalid_argument

#include <boost/container/flat_set.hpp>

#include "sdd/internal_manager_fwd.hh"
#include "sdd/dd/definition.hh"
#include "sdd/hom/consolidate.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation_error.hh"
#include "sdd/hom/identity.hh"
#include "sdd/hom/interrupt.hh"
#include "sdd/hom/local.hh"
#include "sdd/order/order.hh"
#include "sdd/util/packed.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Saturation Fixpoint homomorphism.
template <typename C>
class LIBSDD_ATTRIBUTE_PACKED _saturation_fixpoint
{
public:

  /// @brief The type of a const iterator on this saturation_fixpoint's G operands.
  using const_iterator = const homomorphism<C>*;

  /// @brief The variable type.
  using variable_type = typename C::variable_type;

private:

  /// @brief The type deduced from configuration of the number of operands.
  using operands_size_type = typename C::operands_size_type;

  /// @brief The variable on which this sum works.
  const variable_type variable_;

  /// @brief The homomorphism's F part.
  const homomorphism<C> F_;

  /// @brief The homomorphism's G part size.
  const operands_size_type G_size_;

  /// @brief The homomorphism's L part.
  const homomorphism<C> L_;

public:

  /// @brief Constructor.
  _saturation_fixpoint( variable_type var, const homomorphism<C>& f
                      , boost::container::flat_set<homomorphism<C>>& g
                      , const homomorphism<C>& l)
    : variable_(var)
    , F_(f)
    , G_size_(static_cast<operands_size_type>(g.size()))
    , L_(l)
  {
    // Put all homomorphisms operands right after this sum instance.
    hom::consolidate(G_operands_addr(), g.begin(), g.end());
  }

  /// @brief Destructor.
  ~_saturation_fixpoint()
  {
    for (auto it = G_begin(); it != G_end(); ++it)
    {
      it->~homomorphism<C>();
    }
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& s)
  const
  {
    SDD<C> s1 = s;
    SDD<C> s2 = s;

    try
    {
      do
      {
        s1 = s2;

        try
        {
          s2 = F_(cxt, o, s2); // apply (F + Id)*
          s2 = L_(cxt, o, s2); // apply (L + Id)*
        }
        catch (interrupt<C>& i)
        {
          i.result() = s2;
          throw;
        }

        for (auto cit = G_begin(); cit != G_end(); ++cit)
        {
          const auto& g = *cit;
          try
          {
            // chain applications of G
            s2 = dd::sum(cxt.sdd_context(), {s2, g(cxt, o, s2)});
          }
          catch(interrupt<C>& i)
          {
            i.result() = dd::sum(cxt.sdd_context(), {s2, i.result()});
            throw;
          }
        }
      } while (s1 != s2);
    }
    catch (top<C>& t)
    {
      evaluation_error<C> e(s);
      e.add_top(t);
      throw e;
    }

    return s1;
  }

  /// @brief Skip predicate.
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
    return F_.selector()
       and L_.selector()
       and std::all_of( G_begin(), G_end()
                      , [](const homomorphism<C>& h){return h.selector();});
  }

  /// @brief Get the targeted variable.
  variable_type
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

  /// @brief Get the number of G operands.
  std::size_t
  G_size()
  const noexcept
  {
    return G_size_;
  }

  /// @brief Get an iterator to the first operand of G.
  ///
  /// O(1).
  const_iterator
  G_begin()
  const noexcept
  {
    return reinterpret_cast<const homomorphism<C>*>(G_operands_addr());
  }

  /// @brief Get an iterator to the end of operands of G.
  ///
  /// O(1).
  const_iterator
  G_end()
  const noexcept
  {
    return reinterpret_cast<const homomorphism<C>*>(G_operands_addr()) + G_size_;
  }

  /// @brief Get the local part.
  homomorphism<C>
  L()
  const noexcept
  {
    return L_;
  }

private:

  /// @brief Return the address of the beginning of the operands of G.
  char*
  G_operands_addr()
  const noexcept
  {
    return reinterpret_cast<char*>(const_cast<_saturation_fixpoint*>(this))
         + sizeof(_saturation_fixpoint);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related saturation_fixpoint
template <typename C>
inline
bool
operator==(const _saturation_fixpoint<C>& lhs, const _saturation_fixpoint<C>& rhs)
noexcept
{
  return lhs.variable() == rhs.variable()
     and lhs.F() == rhs.F()
     and lhs.L() == rhs.L()
     and lhs.G_size() == rhs.G_size()
     and std::equal(lhs.G_begin(), lhs.G_end(), rhs.G_begin());
}

/// @internal
/// @related _saturation_fixpoint
template <typename C>
std::ostream&
operator<<(std::ostream& os, const _saturation_fixpoint<C>& s)
{
  os << "Sat(@" << s.variable() << ",  " << s.F() << " + " << s.L();
  if (not (s.G_size() == 0))
  {
    os << " + ";
    std::copy( s.G_begin(), std::prev(s.G_end())
             , std::ostream_iterator<homomorphism<C>>(os, " + "));
    os << *std::prev(s.G_end()) << ")*";
  }
  return os;
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Create the Saturation Fixpoint homomorphism.
/// @related sdd::homomorphism
///
/// We suppose that a saturation fixpoint is created in the rewriting process. Thus, we assume
/// that operands of the G part are already optimized (e.g. local merged and sums flatten).
template <typename C, typename InputIterator>
homomorphism<C>
saturation_fixpoint( typename C::variable_type var
                  , const homomorphism<C>& f
                  , InputIterator gbegin, InputIterator gend
                  , const homomorphism<C>& l)
{
  const std::size_t gsize = std::distance(gbegin, gend);

  if (gsize == 0)
  {
    if (f != id<C>() and l == id<C>())
    {
      return f;
    }
    if (f == id<C>() and l != id<C>())
    {
      return l;
    }
  }

  // A global flat_set to avoid reallocating a new set of operands each time.
  auto& g = global<C>().saturation_fixpoint_data;
  g.clear();
  g.insert(gbegin, gend);
  const std::size_t extra_bytes = g.size() * sizeof(homomorphism<C>);
  return homomorphism<C>::create_variable_size( mem::construct<_saturation_fixpoint<C>>()
                                              , extra_bytes, var, f, g, l);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::_saturation_fixpoint.
template <typename C>
struct hash<sdd::hom::_saturation_fixpoint<C>>
{
  std::size_t
  operator()(const sdd::hom::_saturation_fixpoint<C>& s)
  const
  {
    std::size_t seed = sdd::util::hash(s.variable());
    sdd::util::hash_combine(seed, s.F());
    sdd::util::hash_combine(seed, s.L());
    sdd::util::hash_combine(seed, s.G_begin(), s.G_end());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_SATURATION_FIXPOINT_HH_
