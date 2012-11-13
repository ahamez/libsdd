#ifndef _SDD_HOM_REWRITING_HH_
#define _SDD_HOM_REWRITING_HH_

#include <algorithm> // partition
#include <deque>
#include <tuple>

#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/fixpoint.hh"
#include "sdd/hom/local.hh"
#include "sdd/hom/saturation_sum.hh"
#include "sdd/hom/sum.hh"

/// @cond INTERNAL_DOC

namespace sdd { namespace hom {

/*-------------------------------------------------------------------------------------------*/

template <typename C>
homomorphism<C>
rewrite(const homomorphism<C>&, const order::order<C>&);

/*-------------------------------------------------------------------------------------------*/

/// @brief Concrete implementation of the rewriting process.
template <typename C>
struct rewriter
{
  /// @brief Needed by variant.
  typedef homomorphism<C> result_type;

  /// @brief A variable type.
  typedef typename C::Variable variable_type;

  /// @brief The type of a list of homomorphisms.
  typedef std::deque<homomorphism<C>> hom_list_type;

  /// @brief Tell if an homomorphism is Local.
  struct is_local
  {
    typedef bool result_type;

    constexpr bool
    operator()(const local<C>&)
    const
    {
      return true;
    }

    template <typename T>
    constexpr bool
    operator()(const T&)
    const
    {
      return false;
    }
  };

  /// @brief Tell if an homomorphism is Sum.
  struct is_sum
  {
    typedef bool result_type;

    constexpr bool
    operator()(const sum<C>&)
    const
    {
      return true;
    }

    template <typename T>
    constexpr bool
    operator()(const T&)
    const
    {
      return false;
    }
  };

  /// @brief Get the F, G and L parts of a set of homomorphisms.
  template <typename InputIterator>
  static
  std::tuple<hom_list_type, hom_list_type, hom_list_type, bool>
  partition(const order::order<C>& o, InputIterator begin, InputIterator end)
  {
    bool has_id = false;
    hom_list_type F;
    hom_list_type G;
    hom_list_type L;
    for (; begin != end; ++begin)
    {
      if (*begin == Id<C>())
      {
        has_id = true;
      }
      else if (begin->skip(o))
      {
        F.push_back(*begin);
      }
      else if (apply_visitor(is_local(), (*begin)->data()))
      {
        const local<C>& l = internal::mem::variant_cast<const local<C>>((*begin)->data());
        L.push_back(l.hom());
      }
      else
      {
        G.push_back(*begin);
      }
    }
    return std::make_tuple(F, G, L, has_id);
  }

  /// @brief Rewrite Sum into a Saturation Sum, if possible.
  homomorphism<C>
  operator()( const sum<C>& s
            , const homomorphism<C>& h, const order::order<C>& o)
  const
  {
    auto&& p = partition(o, s.operands().begin(), s.operands().end());
    auto& F = std::get<0>(p);
    auto& G = std::get<1>(p);
    auto& L = std::get<2>(p);
    const bool has_id = std::get<3>(p);

    if (F.size() == 0 and L.size() == 0)
    {
      return h;
    }

    if (has_id)
    {
      F.push_back(Id<C>());
    }

    typedef typename saturation_sum<C>::optional_type optional;
    return SaturationSum<C>( o.variable()
                           , F.size() > 0 ? rewrite(Sum<C>(o.next(), F.begin(), F.end()), o.next())
                                          : optional()
                           , G.begin(), G.end()
                           , L.size() > 0 ? Local( o.identifier()
                                                 , o
                                                 , rewrite( Sum<C>(o.nested(), L.begin(), L.end())
                                                          , o.nested()))
                                          : optional()
                           );
  }

  /// @brief Rewrite a Fixpoint into a Saturation Fixpoint, if possible.
  homomorphism<C>
  operator()( const fixpoint<C>& f
            , const homomorphism<C>& h, const order::order<C>& o)
  const
  {
    if (not apply_visitor(is_sum(), f.hom()->data()))
    {
      return h;
    }

    const sum<C>& s = internal::mem::variant_cast<const sum<C>>(f.hom()->data());

    auto&& p = partition(o, s.operands().begin(), s.operands().end());
    auto& F = std::get<0>(p);
    auto& G = std::get<1>(p);
    auto& L = std::get<2>(p);
    const bool has_id = std::get<3>(p);

    if (not has_id)
    {
      return h;
    }

    if (F.empty() and L.empty())
    {
      return h;
    }

    // Don't forget to add id to F and L parts!.
    F.push_back(Id<C>());
    L.push_back(Id<C>());

    // Put selectors in front. It might help cut paths sooner in the Saturation Fixpoint's
    // evaluation.
    std::partition(G.begin(), G.end(), [](const homomorphism<C>& g){return g.selector();});

    return SaturationFixpoint( o.variable()
                             , rewrite(Fixpoint(Sum<C>(o.next(), F.begin(), F.end())), o.next())
                             , G.begin(), G.end()
                             , Local( o.identifier()
                                    , o
                                    , rewrite(Fixpoint(Sum<C>(o.nested(), L.begin(), L.end())), o.nested())
                                    )
                             );
  }

  /// @brief General case.
  ///
  /// Any other homomorphism is not rewritten.
  template <typename T>
  homomorphism<C>
  operator()(const T&, const homomorphism<C>& h, const order::order<C>&)
  const
  {
    return h;
  }
};

/*-------------------------------------------------------------------------------------------*/

template <typename C>
homomorphism<C>
rewrite(const homomorphism<C>& h, const order::order<C>& o)
{
  if (o.empty())
  {
    return h;
  }
  else
  {
    return apply_visitor(rewriter<C>(), h->data(), h, o);
  }
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

/// @endcond

#endif // _SDD_HOM_REWRITING_HH_
