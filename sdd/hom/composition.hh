#pragma once

#include <iosfwd>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/identity.hh"
#include "sdd/hom/local.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief composition homomorphism.
template <typename C>
struct _composition
{
  /// @brief The left homomorphism to apply.
  const homomorphism<C> left;

  /// @brief The right homomorphism to apply.
  const homomorphism<C> right;

  /// @brief Constructor.
  _composition(const homomorphism<C>& l, const homomorphism<C>& r)
    : left(l), right(r)
  {}

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& x)
  const
  {
    return left(cxt, o, right(cxt, o, x));
  }

  /// @brief Skip predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return left.skip(o) and right.skip(o);
  }

  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return left.selector() and right.selector();
  }

  friend
  bool
  operator==(const _composition& lhs, const _composition& rhs)
  noexcept
  {
    return lhs.left == rhs.left and lhs.right == rhs.right;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const _composition& c)
  {
    return os << c.left << " o " << c.right;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Help optimize a composition' operands.
template <typename C>
struct composition_builder_helper
{
  /// @brief Regroup locals.
  homomorphism<C>
  operator()( const hom::_local<C>& l, const hom:: _local<C>& r
            , const homomorphism<C>& lorig, const homomorphism<C>& rorig)
  const
  {
    if (l.target == r.target)
    {
      return local(l.target, composition(l.h, r.h));
    }
    else
    {
      return hom::make<C, hom::_composition<C>>(lorig, rorig);
    }
  }

  template <typename T, typename U>
  homomorphism<C>
  operator()(const T&, const U&, const homomorphism<C>& left, const homomorphism<C>& right)
  const
  {
    if (left == id<C>())
    {
      return right;
    }
    else if (right == id<C>())
    {
      return left;
    }
    else
    {
      return make<C, _composition<C>>(left, right);
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the composition homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
composition(const homomorphism<C>& left, const homomorphism<C>& right)
{
  return binary_visit(hom::composition_builder_helper<C>(), left, right, left, right);
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
    using namespace sdd::hash;
    return seed(c.left) (val(c.right));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
