#ifndef _SDD_HOM_CONS_HH_
#define _SDD_HOM_CONS_HH_

#include <iosfwd>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/order/order.hh"
#include "sdd/internal/util/packed.hh"

namespace sdd { namespace hom {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Cons homomorphism.
template <typename C, typename Valuation>
class _LIBSDD_ATTRIBUTE_PACKED cons
{
public:

  /// @brief The identifier type.
  typedef typename C::Identifier identifier_type;

private:

  /// @brief
  const order::order<C> order_;

  /// @brief The valuation of the SDD to create.
  const Valuation valuation_;

  /// @brief The homomorphism to apply on successors.
  const homomorphism<C> next_;

public:

  /// @brief Constructor.
  cons(const order::order<C>& o, const Valuation& val, const homomorphism<C>& h)
    : order_(o)
    , valuation_(val)
    , next_(h)
  {
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order::order<C>&, const SDD<C>& x)
  const
  {
    return SDD<C>(order_.variable(), valuation_, next_(cxt, order_.next(), x));
  }

  /// @brief Skip predicate.
  constexpr bool
  skip(const order::order<C>&)
  const noexcept
  {
    return false;
  }

  /// @brief Selector predicate
  constexpr bool
  selector()
  const noexcept
  {
    return false;
  }

  /// @brief Return the order of this Cons.
  const order::order<C>&
  order()
  const noexcept
  {
    return order_;
  }

  /// @brief Return the valuation of the SDD to create.
  const Valuation&
  valuation()
  const noexcept
  {
    return valuation_;
  }

  /// @brief Return the homomorphism to apply on successors.
  homomorphism<C>
  next()
  const noexcept
  {
    return next_;
  }
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Describe Cons characteristics.
template <typename C, typename Valuation>
struct homomorphism_traits<cons<C, Valuation>>
{
  static constexpr bool should_cache = false;
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Equality of two Cons homomorphisms.
/// @related cons
template <typename C, typename Valuation>
inline
bool
operator==(const cons<C, Valuation>& lhs, const cons<C, Valuation>& rhs)
noexcept
{
  return lhs.valuation() == rhs.valuation() and lhs.next() == rhs.next()
     and lhs.order() == rhs.order();
}

/// @related cons
template <typename C, typename Valuation>
std::ostream&
operator<<(std::ostream& os, const cons<C, Valuation>& c)
{
  return os << "Cons(" << c.order().identifier() << ", " << c.valuation() << ", " << c.next()
            << ")";
}

/// @endcond

/*-------------------------------------------------------------------------------------------*/

/// @brief Create the Cons homomorphism.
/// @related homomorphism
template <typename C, typename Valuation>
homomorphism<C>
Cons(const order::order<C> o, const Valuation& val, const homomorphism<C>& h)
{
  return homomorphism<C>::create(internal::mem::construct<cons<C, Valuation>>(), o, val, h);
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

namespace std {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Hash specialization for sdd::hom::cons.
template <typename C, typename Valuation>
struct hash<sdd::hom::cons<C, Valuation>>
{
  std::size_t
  operator()(const sdd::hom::cons<C, Valuation>& h)
  const noexcept
  {
    std::size_t seed = 0;
    sdd::internal::util::hash_combine(seed, h.order());
    sdd::internal::util::hash_combine(seed, h.valuation());
    sdd::internal::util::hash_combine(seed, h.next());
    return seed;
  }
};

/// @endcond

/*-------------------------------------------------------------------------------------------*/

} // namespace std


#endif // _SDD_HOM_CONS_HH_
