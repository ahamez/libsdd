#ifndef _SDD_HOM_REWRITING_HH_
#define _SDD_HOM_REWRITING_HH_

#include <algorithm> // partition
#include <deque>
#include <tuple>

#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/fixpoint.hh"
#include "sdd/hom/local.hh"
#include "sdd/hom/optional_homomorphism.hh"
#include "sdd/hom/saturation_fixpoint.hh"
#include "sdd/hom/saturation_intersection.hh"
#include "sdd/hom/saturation_sum.hh"
#include "sdd/hom/intersection.hh"
#include "sdd/hom/sum.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

// Forward declaration for recursive call by rewriter.
template <typename C>
homomorphism<C>
rewrite(const order<C>&, const homomorphism<C>&);

/*-------------------------------------------------------------------------------------------*/

namespace hom {

/*-------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Concrete implementation of the rewriting process.
template <typename C>
struct rewriter
{
  /// @brief Needed by mem::variant.
  using result_type = homomorphism<C>;

  /// @brief The type of a list of homomorphisms.
  using hom_list_type = std::deque<homomorphism<C>>;

  /// @internal
  /// @brief Tell if an homomorphism is Local.
  struct is_local
  {
    /// @brief Needed by mem::variant.
    using result_type = bool;

    constexpr bool
    operator()(const local<C>&)
    const noexcept
    {
      return true;
    }

    template <typename T>
    constexpr bool
    operator()(const T&)
    const noexcept
    {
      return false;
    }
  };

  /// @internal
  /// @brief Tell if an homomorphism is Sum.
  struct is_sum
  {
    using result_type = bool;

    constexpr bool
    operator()(const sum<C>&)
    const noexcept
    {
      return true;
    }

    template <typename T>
    constexpr bool
    operator()(const T&)
    const noexcept
    {
      return false;
    }
  };

  /// @brief Get the F, G and L parts of a set of homomorphisms.
  template <typename InputIterator>
  static
  std::tuple<hom_list_type, hom_list_type, hom_list_type, bool>
  partition(const order<C>& o, InputIterator begin, InputIterator end)
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
      else if (visit(is_local{}, *begin))
      {
        const local<C>& l = mem::variant_cast<const local<C>>(**begin);
        L.push_back(l.hom());
      }
      else
      {
        G.push_back(*begin);
      }
    }
    return std::make_tuple(std::move(F), std::move(G), std::move(L), has_id);
  }

  /// @brief Rewrite Sum into a Saturation Sum, if possible.
  homomorphism<C>
  operator()(const sum<C>& s, const homomorphism<C>& h, const order<C>& o)
  const
  {
    auto&& p = partition(o, s.begin(), s.end());
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

    return SaturationSum<C>( o.variable()
                           , F.size() > 0 ? rewrite(o.next(), Sum<C>(o.next(), F.begin(), F.end()))
                                          : optional_homomorphism<C>()
                           , G.begin(), G.end()
                           , L.size() > 0 ? Local( o.position()
                                                 , rewrite( o.nested()
                                                          , Sum<C>(o.nested(), L.begin(), L.end())
                                                          ))
                                          : optional_homomorphism<C>()
                           );
  }

  /// @brief Rewrite Intersection into a Saturation Intersection, if possible.
  homomorphism<C>
  operator()(const intersection<C>& s, const homomorphism<C>& h, const order<C>& o)
  const
  {
    auto&& p = partition(o, s.begin(), s.end());
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

    return SaturationIntersection<C>( o.variable()
                           , F.size() > 0 ? rewrite( o.next()
                                                   , Intersection<C>(o.next(), F.begin(), F.end()))
                                          : optional_homomorphism<C>()
                           , G.begin(), G.end()
                           , L.size() > 0 ? Local( o.position()
                                                 , rewrite( o.nested()
                                                          , Intersection<C>( o.nested(), L.begin()
                                                                           , L.end())))
                                          : optional_homomorphism<C>()
                           );
  }

  /// @brief Rewrite a Fixpoint into a Saturation Fixpoint, if possible.
  homomorphism<C>
  operator()(const fixpoint<C>& f, const homomorphism<C>& h, const order<C>& o)
  const
  {
    if (not visit(is_sum{}, f.hom()))
    {
      return h;
    }

    const sum<C>& s = mem::variant_cast<const sum<C>>(*f.hom());

    auto&& p = partition(o, s.begin(), s.end());
    auto& F = std::get<0>(p);
    auto& G = std::get<1>(p);
    auto& L = std::get<2>(p);
    const bool has_id = std::get<3>(p);

    if ((not has_id) or (F.empty() and L.empty()))
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
                             , rewrite(o.next(), Fixpoint(Sum<C>(o.next(), F.begin(), F.end())))
                             , G.begin(), G.end()
                             , Local( o.position()
                                    , rewrite( o.nested()
                                             , Fixpoint(Sum<C>(o.nested(), L.begin(), L.end())))
                                    )
                             );
  }

  /// @brief General case.
  ///
  /// Any other homomorphism is not rewritten.
  template <typename T>
  homomorphism<C>
  operator()(const T&, const homomorphism<C>& h, const order<C>&)
  const
  {
    return h;
  }
};

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Rewrite an homomorphism to enable saturation.
template <typename C>
homomorphism<C>
rewrite(const order<C>& o, const homomorphism<C>& h)
{
  return o.empty()
       ? h
       : visit_self(hom::rewriter<C>(), h, o);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_HOM_REWRITING_HH_
