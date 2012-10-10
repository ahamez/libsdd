#ifndef _SDD_HOM_REWRITING_HH_
#define _SDD_HOM_REWRITING_HH_

#include <tuple>
#include <vector>

#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/sum.hh"
#include "sdd/order/order.hh"

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
  typedef std::vector<const homomorphism<C>> hom_list_type;

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

  /// @brief Get the F, G and L parts of a set of homomorphisms.
  template <typename InputIterator>
  static
  std::tuple<hom_list_type, hom_list_type, hom_list_type, bool>
  partition(const variable_type& v, InputIterator begin, InputIterator end)
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
      else if (begin->skip(v))
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
  operator()(const sum<C>& s, const homomorphism<C>& h, const order::order<C>& o)
  const
  {
    auto&& p = partition(o->variable_, s.operands().begin(), s.operands().end());
    auto& F = std::get<0>(p);
    auto& G = std::get<1>(p);
    auto& L = std::get<2>(p);
    const bool has_id = std::get<3>(p);

    if (F.size() == 0 and L.size() == 0)
    {
      return h;
    }

    if (has_id) // has_id
    {
      F.push_back(Id<C>());
    }

    typedef typename saturation_sum<C>::optional_type optional;
    return SaturationSum<C>( o->variable_
                           , F.size() > 0 ? rewrite(Sum<C>(F.begin(), F.end()), o->next_)
                                          : optional()
                           , G.begin(), G.end()
                           , L.size() > 0 ? Local( o->variable_
                                                 , rewrite( Sum<C>(L.begin(), L.end())
                                                          , o->nested_))
                                          : optional());
  }

  /// @brief General case.
  ///
  /// Any other homomorphism is not rewritten.
  template <typename T>
  homomorphism<C>
  operator()(const T& x, const homomorphism<C>& h, const order::order<C>&)
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
  if (o)
  {
    return apply_visitor(rewriter<C>(), h->data(), h, o);
  }
  else
  {
    return h;
  }
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

#endif // _SDD_HOM_REWRITING_HH_
