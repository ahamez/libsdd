#ifndef _SDD_HOM_EVALUATION_HH_
#define _SDD_HOM_EVALUATION_HH_

#include <cassert>
#include <iosfwd>

#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/rewriting.hh"

/// @cond INTERNAL_DOC

namespace sdd { namespace hom {

/*-------------------------------------------------------------------------------------------*/

/// @brief Evaluate an homomorphism.
template <typename C>
struct evaluation
{
  /// @brief Used by internal::util::variant.
  typedef SDD<C> result_type;

  template <typename H>
  bool
  operator()( const H& h, const zero_terminal<C>&
            , const SDD<C>&, context<C>&, const homomorphism<C>&)
  const noexcept
  {
    assert(false);
    __builtin_unreachable();
  }

  template <typename H>
  SDD<C>
  operator()( const H& h, const one_terminal<C>&
            , const SDD<C>& x, context<C>& cxt, const homomorphism<C>&)
  const
  {
    return h(cxt, x);
  }

  /// @brief Dispatch evaluation to the concrete homomorphism.
  ///
  /// Implement a part of the automatic saturation: evaluation is propagated on successors
  /// whenever possible.
  template <typename H, typename Node>
  SDD<C>
  operator()( const H& h, const Node& node
            , const SDD<C>& x, context<C>& cxt, const homomorphism<C>& hom_proxy)
  const
  {
    if (h.skip(node.variable()))
    {
      square_union<C, typename Node::valuation_type> su;
      su.reserve(node.size());
      for (const auto& arc : node)
      {
        SDD<C> new_successor = hom_proxy(cxt, arc.successor());
        if (not new_successor.empty())
        {
          su.add(new_successor, arc.valuation());
        }
      }
      return SDD<C>(node.variable(), su(cxt.sdd_context()));
    }
    else
    {
      return apply_visitor( dispatch_eval(), rewrite(cxt, hom_proxy, node.variable())->data()
                          , cxt, x);
    }
  }

  /// @brief Dispatch to concrete evaluation.
  ///
  /// Avoid infinite recursion.
  struct dispatch_eval
  {
    typedef SDD<C> result_type;

    template <typename H>
    SDD<C>
    operator()(const H& h, context<C>& cxt, const SDD<C>& s)
    const
    {
      return h(cxt, s);
    }
  };
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Default traits for homomorphisms.
template <typename T>
struct homomorphism_traits
{
  static constexpr bool should_cache = true;
};

/*-------------------------------------------------------------------------------------------*/

/// @brief The evaluation of an homomorphism in the cache.
template <typename C>
struct cached_homomorphism
{
  /// @brief Needed by the cache.
  typedef SDD<C> result_type;

  /// @brief The evaluation context.
  context<C>& cxt_;

  /// @brief The homomorphism to evaluate.
  const homomorphism<C> h_;

  /// @brief The homomorphism's operand.
  const SDD<C> sdd_;

  /// @brief Constructor.
  cached_homomorphism(context<C>& cxt, const homomorphism<C>& h, const SDD<C>& s)
    : cxt_(cxt)
    , h_(h)
    , sdd_(s)
  {
  }

  /// @brief Launch the evaluation.
  SDD<C>
  operator()()
  const
  {
    return apply_binary_visitor(evaluation<C>(), h_->data(), sdd_->data(), sdd_, cxt_, h_);
  }
};

/*-------------------------------------------------------------------------------------------*/

/// @related cached_homomorphism
template <typename C>
inline
bool
operator==(const cached_homomorphism<C>& lhs, const cached_homomorphism<C>& rhs)
noexcept
{
  return lhs.h_ == rhs.h_ and lhs.sdd_ == rhs.sdd_;
}

/// @related cached_homomorphism
template <typename C>
std::ostream&
operator<<(std::ostream& os, const cached_homomorphism<C>& ch)
{
  return os << ch.h_;
}

/*-------------------------------------------------------------------------------------------*/

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

  /// @brief Application.
  bool
  operator()(const cached_homomorphism<C>& ch)
  const noexcept
  {
    return apply_visitor(*this, ch.h_->data());
  }
};

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

namespace std {

/*-------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::hom::cached_homomorphism
template <typename C>
struct hash<sdd::hom::cached_homomorphism<C>>
{
  std::size_t
  operator()(const sdd::hom::cached_homomorphism<C>& op)
  const noexcept
  {
    std::size_t seed = 0;
    sdd::internal::util::hash_combine(seed, op.h_);
    sdd::internal::util::hash_combine(seed, op.sdd_);
    return seed;
  }
};

/*-------------------------------------------------------------------------------------------*/

} // namespace std

/// @endcond

#endif // _SDD_HOM_EVALUATION_HH_
