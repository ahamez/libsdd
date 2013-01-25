#ifndef _SDD_HOM_EVALUATION_HH_
#define _SDD_HOM_EVALUATION_HH_

#include <cassert>
#include <iosfwd>

#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/rewrite.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Evaluate an homomorphism.
template <typename C>
struct evaluation
{
  /// @brief Used by util::variant.
  typedef SDD<C> result_type;

  template <typename H>
  bool
  operator()( const H& h, const zero_terminal<C>&
            , const SDD<C>&, context<C>&, const order<C>& o, const homomorphism<C>&)
  const noexcept
  {
    assert(false);
    __builtin_unreachable();
  }

  template <typename H>
  SDD<C>
  operator()( const H& h, const one_terminal<C>&
            , const SDD<C>& x, context<C>& cxt, const order<C>& o
            , const homomorphism<C>&)
  const
  {
    return h(cxt, o, x);
  }

  /// @brief Dispatch evaluation to the concrete homomorphism.
  ///
  /// Implement a part of the automatic saturation: evaluation is propagated on successors
  /// whenever possible.
  template <typename H, typename Node>
  SDD<C>
  operator()( const H& h, const Node& node
            , const SDD<C>& x, context<C>& cxt, const order<C>& o
            , const homomorphism<C>& hom_proxy)
  const
  {
    assert(not o.empty() && "Empty order.");
    assert(o.variable() == node.variable() && "Different variables in order and SDD.");

    if (h.skip(o))
    {
      // The evaluated homomorphism skips the current level. We can thus forward its application
      // to the following levels.
      dd::square_union<C, typename Node::valuation_type> su;
      su.reserve(node.size());
      for (const auto& arc : node)
      {
        SDD<C> new_successor = hom_proxy(cxt, o.next(), arc.successor());
        if (not new_successor.empty())
        {
          su.add(new_successor, arc.valuation());
        }
      }
      return SDD<C>(node.variable(), su(cxt.sdd_context()));
    }
    else
    {
      return h(cxt, o, x);
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Default traits for homomorphisms.
template <typename T>
struct homomorphism_traits
{
  static constexpr bool should_cache = true;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The evaluation of an homomorphism in the cache.
template <typename C>
struct cached_homomorphism
{
  /// @brief Needed by the cache.
  typedef SDD<C> result_type;

  /// @brief The current order position.
  const order<C>& ord;

  /// @brief The homomorphism to evaluate.
  const homomorphism<C> hom;

  /// @brief The homomorphism's operand.
  const SDD<C> sdd;

  /// @brief Constructor.
  cached_homomorphism(context<C>& cxt, const order<C>& o, const homomorphism<C>& h, const SDD<C>& s)
    : ord(o)
    , hom(h)
    , sdd(s)
  {
  }

  /// @brief Launch the evaluation.
  SDD<C>
  operator()(context<C>& cxt)
  const
  {
    return apply_binary_visitor(evaluation<C>(), hom->data(), sdd->data(), sdd, cxt, ord, hom);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related cached_homomorphism
template <typename C>
inline
bool
operator==(const cached_homomorphism<C>& lhs, const cached_homomorphism<C>& rhs)
noexcept
{
  return lhs.hom == rhs.hom and lhs.sdd == rhs.sdd;
}

/// @internal
/// @related cached_homomorphism
template <typename C>
std::ostream&
operator<<(std::ostream& os, const cached_homomorphism<C>& ch)
{
  return os << ch.hom;
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Used by the cache as a filter to know if an homomorphism should be cached.
template <typename C>
struct should_cache
{
  /// @brief Needed by variant.
  typedef bool result_type;

  /// @brief Dispatch to each homomorphism's trait.
  template <typename T>
  constexpr bool
  operator()(const T&)
  const noexcept
  {
    return homomorphism_traits<T>::should_cache;
  }

  /// @brief Application of should_cache.
  bool
  operator()(const cached_homomorphism<C>& ch)
  const noexcept
  {
    return apply_visitor(*this, ch.hom->data());
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::cached_homomorphism
template <typename C>
struct hash<sdd::hom::cached_homomorphism<C>>
{
  std::size_t
  operator()(const sdd::hom::cached_homomorphism<C>& ch)
  const noexcept
  {
    std::size_t seed = 0;
    sdd::util::hash_combine(seed, ch.hom);
    sdd::util::hash_combine(seed, ch.sdd);
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_EVALUATION_HH_
