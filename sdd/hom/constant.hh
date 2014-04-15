#ifndef _SDD_HOM_CONSTANT_HH_
#define _SDD_HOM_CONSTANT_HH_

#include <iosfwd>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/traits.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief constant homomorphism.
template <typename C>
struct _constant
{
private:

  /// @brief The SDD to return.
  const SDD<C> sdd_;

public:

  /// @brief Constructor.
  _constant(const SDD<C>& s)
    : sdd_(s)
  {}

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>&, const order<C>&, const SDD<C>&)
  const noexcept
  {
    return sdd_;
  }

  /// @brief Skip variable predicate.
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

  SDD<C>
  sdd()
  const noexcept
  {
    return sdd_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Describe _constant characteristics.
template <typename C>
struct homomorphism_traits<_constant<C>>
{
  static constexpr bool should_cache = false;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related _constant
template <typename C>
inline
bool
operator==(const _constant<C>& lhs, const _constant<C>& rhs)
noexcept
{
  return lhs.sdd() == rhs.sdd();
}

/// @internal
/// @related _constant
template <typename C>
std::ostream&
operator<<(std::ostream& os, const _constant<C>& c)
{
  return os << "const(" << c.sdd() << ")";
}

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the constant homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
constant(const SDD<C>& s)
{
  return homomorphism<C>::create(mem::construct<hom::_constant<C>>(), s);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::constant.
template <typename C>
struct hash<sdd::hom::_constant<C>>
{
  constexpr
  std::size_t
  operator()(const sdd::hom::_constant<C>& c)
  const
  {
    return sdd::util::hash(c.sdd());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_CONSTANT_HH_
