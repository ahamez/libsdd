#ifndef _SDD_HOM_COMPOSITON_HH_
#define _SDD_HOM_COMPOSITON_HH_

#include <iosfwd>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/identity.hh"
#include "sdd/mem/interrupt.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief composition homomorphism.
template <typename C>
class _composition
{
private:

  /// @brief The left homomorphism to apply.
  const homomorphism<C> left_;

  /// @brief The right homomorphism to apply.
  const homomorphism<C> right_;

public:

  /// @brief Constructor.
  _composition(const homomorphism<C>& left, const homomorphism<C>& right)
    : left_(left)
    , right_(right)
  {}

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& x)
  const
  {
    SDD<C> tmp = x;
    try
    {
      tmp = right_(cxt, o, x);
      tmp = left_(cxt, o, tmp);
      return tmp;
    }
    catch (interrupt<SDD<C>>& i)
    {
      i.result() = tmp;
      throw;
    }
  }

  /// @brief Skip predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return left_.skip(o) and right_.skip(o);
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
/// @brief Equality of two _composition homomorphisms.
/// @related _composition
template <typename C>
inline
bool
operator==(const _composition<C>& lhs, const _composition<C>& rhs)
noexcept
{
  return lhs.left() == rhs.left() and lhs.right() == rhs.right();
}

/// @internal
/// @related _composition
template <typename C>
std::ostream&
operator<<(std::ostream& os, const _composition<C>& c)
{
  return os << c.left() << " o " << c.right();
}

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the composition homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
composition(const homomorphism<C>& left, const homomorphism<C>& right)
{
  if (left == id<C>())
  {
    return right;
  }
  else if (right == id<C>())
  {
    return left;
  }
  return homomorphism<C>::create(mem::construct<hom::_composition<C>>(), left, right);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::composition.
template <typename C>
struct hash<sdd::hom::_composition<C>>
{
  std::size_t
  operator()(const sdd::hom::_composition<C>& c)
  const
  {
    std::size_t seed = sdd::util::hash(c.left());
    sdd::util::hash_combine(seed, c.right());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_COMPOSITON_HH_
