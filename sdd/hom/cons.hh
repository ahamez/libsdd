#ifndef _SDD_HOM_CONS_HH_
#define _SDD_HOM_CONS_HH_

#include <iosfwd>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/order/order.hh"
#include "sdd/util/packed.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief cons homomorphism.
template <typename C, typename Valuation>
class LIBSDD_ATTRIBUTE_PACKED _cons
{
private:

  /// @brief
  const order<C> order_;

  /// @brief The valuation of the SDD to create.
  const Valuation valuation_;

  /// @brief The homomorphism to apply on successors.
  const homomorphism<C> next_;

public:

  /// @brief Constructor.
  _cons(const order<C>& o, const Valuation& val, const homomorphism<C>& h)
    : order_(o), valuation_(val), next_(h)
  {}

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>&, const SDD<C>& x)
  const
  {
    return {order_.variable(), valuation_, next_(cxt, order_.next(), x)};
  }

  /// @brief Skip predicate.
  constexpr bool
  skip(const order<C>&)
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

  /// @brief Return the order of this cons.
  const order<C>&
  get_order()
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

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Describe cons characteristics.
template <typename C, typename Valuation>
struct homomorphism_traits<_cons<C, Valuation>>
{
  static constexpr bool should_cache = false;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Equality of two _cons homomorphisms.
/// @related _cons
template <typename C, typename Valuation>
inline
bool
operator==(const _cons<C, Valuation>& lhs, const _cons<C, Valuation>& rhs)
noexcept
{
  return lhs.valuation() == rhs.valuation() and lhs.next() == rhs.next()
     and lhs.get_order() == rhs.get_order();
}

/// @internal
/// @related _cons
template <typename C, typename Valuation>
std::ostream&
operator<<(std::ostream& os, const _cons<C, Valuation>& c)
{
  return os << "cons(" << c.get_order().identifier() << ", " << c.valuation() << ", " << c.next()
            << ")";
}

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the cons homomorphism.
/// @related homomorphism
template <typename C, typename Valuation>
homomorphism<C>
cons(const order<C> o, const Valuation& val, const homomorphism<C>& h)
{
  return homomorphism<C>::create(mem::construct<hom::_cons<C, Valuation>>(), o, val, h);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::cons.
template <typename C, typename Valuation>
struct hash<sdd::hom::_cons<C, Valuation>>
{
  std::size_t
  operator()(const sdd::hom::_cons<C, Valuation>& h)
  const
  {
    std::size_t seed = sdd::util::hash(h.get_order());
    sdd::util::hash_combine(seed, h.valuation());
    sdd::util::hash_combine(seed, h.next());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_CONS_HH_
