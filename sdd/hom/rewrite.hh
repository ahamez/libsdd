/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <algorithm> // partition
#include <deque>
#include <tuple>

#include "sdd/hom/common_types.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/fixpoint.hh"
#include "sdd/hom/local.hh"
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
  /// @brief The type of a list of homomorphisms.
  using hom_list_type = std::deque<homomorphism<C>>;

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
      if (*begin == id<C>())
      {
        has_id = true;
      }
      else if (begin->skip(o))
      {
        F.push_back(*begin);
      }
      else if (mem::is<_local<C>>(*begin))
      {
        const _local<C>& l = mem::variant_cast<const _local<C>>(**begin);
        L.push_back(l.h);
      }
      else
      {
        G.push_back(*begin);
      }
    }
    return std::make_tuple(std::move(F), std::move(G), std::move(L), has_id);
  }

  /// @brief Rewrite sum into a Saturation sum, if possible.
  homomorphism<C>
  operator()(const _sum<C>& s, const homomorphism<C>& h, const order<C>& o)
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
      F.push_back(id<C>());
    }

    return saturation_sum( o.variable()
                         , F.size() > 0 ? rewrite(o.next(), sum(o.next(), F.begin(), F.end()))
                                        : optional_homomorphism<C>()
                         , G.begin(), G.end()
                         , L.size() > 0 ? local( o.variable()
                                               , rewrite( o.nested()
                                                        , sum(o.nested(), L.begin(), L.end())
                                                        ))
                                        : optional_homomorphism<C>()
                         );
  }

  /// @brief Rewrite intersection into a Saturation intersection, if possible.
  homomorphism<C>
  operator()(const _intersection<C>& s, const homomorphism<C>& h, const order<C>& o)
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
      F.push_back(id<C>());
    }

    return saturation_intersection( o.variable()
                           , F.size() > 0 ? rewrite( o.next()
                                                   , intersection(o.next(), F.begin(), F.end()))
                                          : optional_homomorphism<C>()
                           , G.begin(), G.end()
                           , L.size() > 0 ? local( o.variable()
                                                 , rewrite( o.nested()
                                                          , intersection( o.nested(), L.begin()
                                                                        , L.end())))
                                          : optional_homomorphism<C>()
                           );
  }

  /// @brief Rewrite a Fixpoint into a Saturation Fixpoint, if possible.
  homomorphism<C>
  operator()(const _fixpoint<C>& f, const homomorphism<C>& h, const order<C>& o)
  const
  {
    if (not mem::is<_sum<C>>(f.h))
    {
      return h;
    }

    const _sum<C>& s = mem::variant_cast<const _sum<C>>(*f.h);

    auto&& p = partition(o, s.begin(), s.end());
    auto& F = std::get<0>(p);
    auto& G = std::get<1>(p);
    auto& L = std::get<2>(p);
    const bool has_id = std::get<3>(p);

    if (not has_id)
    {
      return h;
    }

    auto rewritten_F = id<C>();
    if (not F.empty())
    {
      // Don't forget to add id to F!.
      F.push_back(id<C>());
      rewritten_F = rewrite(o.next(), fixpoint(sum(o.next(), F.begin(), F.end())));
    }

    auto rewritten_L = id<C>();
    if (not L.empty())
    {
      // Don't forget to add id to L!.
      L.push_back(id<C>());
      rewritten_L
        = local(o.variable(), rewrite(o.nested(), fixpoint(sum(o.nested(), L.begin(), L.end()))));
    }

    // Put selectors in front. It might help cut paths sooner in the Saturation Fixpoint's
    // evaluation.
    std::partition(G.begin(), G.end(), [](const homomorphism<C>& g){return g.selector();});

    return saturation_fixpoint(o.variable(), rewritten_F, G.begin(), G.end(), rewritten_L);
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
  return o.empty() ? h : visit(hom::rewriter<C>(), h, h, o);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd
