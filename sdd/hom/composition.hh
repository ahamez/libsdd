#ifndef _SDD_HOM_COMPOSITON_HH_
#define _SDD_HOM_COMPOSITON_HH_

#include <iosfwd>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/identity.hh"
#include "sdd/util/packed.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Composition homomorphism.
template <typename C>
class _LIBSDD_ATTRIBUTE_PACKED composition
{
public:

  /// @brief The variable type.
  typedef typename C::Variable variable_type;

private:

  /// @brief The left homomorphism to apply.
  const homomorphism<C> left_;

  /// @brief The right homomorphism to apply.
  const homomorphism<C> right_;

public:

  /// @brief Constructor.
  composition(const homomorphism<C>& left, const homomorphism<C>& right)
    : left_(left)
    , right_(right)
  {
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const SDD<C>& x)
  const
  {
    return left_(cxt, right_(cxt, x));
  }

  /// @brief Skip variable predicate.
  bool
  skip(const variable_type& var)
  const noexcept
  {
    return left_.skip(var) and right_.skip(var);
  }

  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return left_.selector() and right_.selector();
  }

  /// @brief Return the left homomorphism to apply.
  homomorphism<C>
  left()
  const noexcept
  {
    return left_;
  }

  /// @brief Return the right homomorphism to apply.
  homomorphism<C>
  right()
  const noexcept
  {
    return right_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Equality of two composition homomorphisms.
/// @related composition
template <typename C>
inline
bool
operator==(const composition<C>& lhs, const composition<C>& rhs)
noexcept
{
  return lhs.left() == rhs.left() and lhs.right() == rhs.right();
}

/// @internal
/// @related composition
template <typename C>
std::ostream&
operator<<(std::ostream& os, const composition<C>& c)
{
  return os << c.left() << " o " << c.right();
}

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the composition homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
Composition(const homomorphism<C>& left, const homomorphism<C>& right)
{
  if (left == Id<C>())
  {
    return right;
  }
  else if (right == Id<C>())
  {
    return left;
  }
  return homomorphism<C>::create(mem::construct<hom::composition<C>>(), left, right);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::composition.
template <typename C>
struct hash<sdd::hom::composition<C>>
{
  std::size_t
  operator()(const sdd::hom::composition<C>& c)
  const noexcept
  {
    std::size_t seed = 0;
    sdd::util::hash_combine(seed, c.left());
    sdd::util::hash_combine(seed, c.right());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_COMPOSITON_HH_
