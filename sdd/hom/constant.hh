#ifndef _SDD_HOM_CONSTANT_HH_
#define _SDD_HOM_CONSTANT_HH_

#include <iosfwd>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Constant homomorphism.
template <typename C>
struct constant
{
private:

  /// @brief The SDD to return.
  const SDD<C> sdd_;

public:

  /// @brief Constructor.
  constant(const SDD<C>& s)
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
/// @brief Describe Constant characteristics.
template <typename C>
struct homomorphism_traits<constant<C>>
{
  static constexpr bool should_cache = false;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Equality of two constant.
/// @related constant
template <typename C>
inline
bool
operator==(const constant<C>& lhs, const constant<C>& rhs)
noexcept
{
  return lhs.sdd() == rhs.sdd();
}

/// @internal
/// @related constant
template <typename C>
std::ostream&
operator<<(std::ostream& os, const constant<C>& c)
{
  return os << "Constant(" << c.sdd() << ")";
}

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the Constant homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
Constant(const SDD<C>& s)
{
  return homomorphism<C>::create(mem::construct<hom::constant<C>>(), s);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::constant.
template <typename C>
struct hash<sdd::hom::constant<C>>
{
  constexpr
  std::size_t
  operator()(const sdd::hom::constant<C>& c)
  const
  {
    return sdd::util::hash(c.sdd());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_CONSTANT_HH_
