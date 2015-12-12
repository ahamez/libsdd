/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <algorithm>  // all_of, copy, equal
#include <iosfwd>
#include <stdexcept>  // invalid_argument

#include <boost/container/flat_set.hpp>

#include "sdd/internal_manager_fwd.hh"
#include "sdd/dd/definition.hh"
#include "sdd/hom/consolidate.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/identity.hh"
#include "sdd/hom/local.hh"
#include "sdd/order/order.hh"
#include "sdd/util/packed.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Saturation Fixpoint homomorphism.
template <typename C>
struct LIBSDD_ATTRIBUTE_PACKED _saturation_fixpoint
{
  /// @brief The type of a const iterator on this saturation_fixpoint's G operands.
  using const_iterator = const homomorphism<C>*;

  /// @brief The variable type.
  using variable_type = typename C::variable_type;

  /// @brief The type deduced from configuration of the number of operands.
  using operands_size_type = typename C::operands_size_type;

  /// @brief The variable on which this sum works.
  const variable_type variable;

  /// @brief The homomorphism's F part.
  const homomorphism<C> F;

  /// @brief The homomorphism's G part size.
  const operands_size_type G_size;

  /// @brief The homomorphism's L part.
  const homomorphism<C> L;

public:

  /// @brief Constructor.
  _saturation_fixpoint( variable_type var, homomorphism<C> f
                      , boost::container::flat_set<homomorphism<C>>& g
                      , homomorphism<C> l)
    : variable{var}
    , F{std::move(f)}
    , G_size{static_cast<operands_size_type>(g.size())}
    , L{std::move(l)}
  {
    // Put all homomorphisms operands right after this sum instance.
    hom::consolidate(G_operands_addr(), g.begin(), g.end());
  }

  /// @brief Destructor.
  ~_saturation_fixpoint()
  {
    for (auto& elem : *this)
    {
      elem.~homomorphism<C>();
    }
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& s)
  const
  {
    auto& sdd_context = cxt.sdd_context();

    SDD<C> s1 = s;
    SDD<C> s2 = s;

    do
    {
      s1 = s2;

      s2 = F(cxt, o, s2); // apply (F + Id)*
      s2 = L(cxt, o, s2); // apply (L + Id)*

      for (const auto& g : *this)
      {
        // chain applications of G
        s2 = dd::sum(sdd_context, dd::sum_builder<C, SDD<C>>(sdd_context, {s2, g(cxt, o, s2)}));
      }
    } while (s1 != s2);

    return s1;
  }

  /// @brief Skip predicate.
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
    return F.selector() and L.selector()
       and std::all_of(begin(), end(), [](const auto& h){return h.selector();});
  }

  /// @brief Get an iterator to the first operand of G.
  ///
  /// O(1).
  const_iterator
  begin()
  const noexcept
  {
    return reinterpret_cast<const homomorphism<C>*>(G_operands_addr());
  }

  /// @brief Get an iterator to the end of operands of G.
  ///
  /// O(1).
  const_iterator
  end()
  const noexcept
  {
    return reinterpret_cast<const homomorphism<C>*>(G_operands_addr()) + G_size;
  }

  friend
  bool
  operator==(const _saturation_fixpoint& lhs, const _saturation_fixpoint& rhs)
  noexcept
  {
    return lhs.variable == rhs.variable and lhs.F == rhs.F
       and lhs.L == rhs.L and lhs.G_size == rhs.G_size
       and std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const _saturation_fixpoint& s)
  {
    os << "Sat(@" << s.variable << ",  " << s.F << " + " << s.L;
    if (s.G_size != 0)
    {
      os << " + ";
      std::copy( s.begin(), std::prev(s.end())
               , std::ostream_iterator<homomorphism<C>>(os, " + "));
      os << *std::prev(s.end()) << ")*";
    }
    return os;
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
    if (f != id<C>() and l == id<C>()) return f;
    if (f == id<C>() and l != id<C>()) return l;
  }

  // A global flat_set to avoid reallocating a new set of operands each time.
  auto& g = global<C>().saturation_fixpoint_data;
  g.clear();
  g.insert(gbegin, gend);
  const std::size_t extra_bytes = g.size() * sizeof(homomorphism<C>);
  return hom::make_variable_size<C, _saturation_fixpoint<C>>(extra_bytes, var, f, g, l);
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
    using namespace sdd::hash;
    return seed(s.variable) (val(s.F)) (val(s.L)) (range(s));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
