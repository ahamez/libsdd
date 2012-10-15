#ifndef _SDD_HOM_CONSTANT_HH_
#define _SDD_HOM_CONSTANT_HH_

#include <iosfwd>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"

namespace sdd { namespace hom {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

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
  {
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>&, const SDD<C>&)
  const noexcept
  {
    return sdd_;
  }

  /// @brief Skip variable predicate.
  constexpr bool
  skip(const typename C::Variable&)
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

/*-------------------------------------------------------------------------------------------*/

/// @brief Describe Constant characteristics.
template <typename C>
struct homomorphism_traits<constant<C>>
{
  static constexpr bool should_cache = false;
};

/*-------------------------------------------------------------------------------------------*/

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

/// @related constant
template <typename C>
std::ostream&
operator<<(std::ostream& os, const constant<C>& c)
{
  return os << c.sdd();
}

/// @endcond

/*-------------------------------------------------------------------------------------------*/

/// @brief Create the Constant homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
Constant(const SDD<C>& s)
{
  return homomorphism<C>::create(internal::mem::construct<constant<C>>(), s);
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

namespace std {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Hash specialization for sdd::hom::constant.
template <typename C>
struct hash<sdd::hom::constant<C>>
{
  constexpr
  std::size_t
  operator()(const sdd::hom::constant<C>& c)
  const noexcept
  {
    return hash<sdd::SDD<C>>()(c.sdd());
  }
};

/// @endcond

/*-------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_CONSTANT_HH_
