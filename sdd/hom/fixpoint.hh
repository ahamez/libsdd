#ifndef _SDD_HOM_FIXPOINT_HH_
#define _SDD_HOM_FIXPOINT_HH_

#include <iosfwd>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/identity.hh"
#include "sdd/hom/local.hh"
#include "sdd/order/order.hh"
#include "sdd/util/packed.hh"

namespace sdd {

// Forward declaration needed by fixpoint_builder_helper.
template <typename C>
homomorphism<C>
Fixpoint(const homomorphism<C>&);

namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Fixpoint homomorphism.
template <typename C>
class LIBSDD_ATTRIBUTE_PACKED fixpoint
{
private:

  /// @brief The homomorphism to apply until a fixpoint is reached.
  const homomorphism<C> h_;

public:

  /// @brief Constructor.
  fixpoint(const homomorphism<C>& h)
    : h_(h)
  {
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& x)
  const
  {
    SDD<C> x1 = x;
    SDD<C> x2 = x1;
    do
    {
      x2 = x1;
      x1 = h_(cxt, o, x1);
    } while (x1 != x2);
    return x1;
  }

  /// @brief Skip predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return h_.skip(o);
  }

  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return h_.selector();
  }

  /// @brief Get the homomorphism to apply within the fixpoint.
  homomorphism<C>
  hom()
  const noexcept
  {
    return h_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Equality of fixpoint.
/// @related fixpoint
template <typename C>
inline
bool
operator==(const fixpoint<C>& lhs, const fixpoint<C>& rhs)
noexcept
{
  return lhs.hom() == rhs.hom();
}

/// @internal
/// @related fixpoint
template <typename C>
std::ostream&
operator<<(std::ostream& os, const fixpoint<C>& f)
{
  return os << "(" << f.hom() << ")*";
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Concrete creation of Fixpoint.
template <typename C>
struct fixpoint_builder_helper
{
  typedef homomorphism<C> result_type;

  homomorphism<C>
  operator()(const identity<C>&, const homomorphism<C>& h)
  const noexcept
  {
    return h;
  }

  homomorphism<C>
  operator()(const fixpoint<C>&, const homomorphism<C>& h)
  const noexcept
  {
    return h;
  }

  template <typename T>
  homomorphism<C>
  operator()(const T&, const homomorphism<C>& h)
  const noexcept
  {
    return homomorphism<C>::create(mem::construct<fixpoint<C>>(), h);
  }
};

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the Fixpoint homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
Fixpoint(const homomorphism<C>& h)
{
  return h.selector()
       ? h
       : visit(hom::fixpoint_builder_helper<C>(), h, h);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::fixpoint.
template <typename C>
struct hash<sdd::hom::fixpoint<C>>
{
  std::size_t
  operator()(const sdd::hom::fixpoint<C>& f)
  const noexcept
  {
    std::size_t seed = 345789; // avoid to have the same hash as the contained homormorphism
    sdd::util::hash_combine(seed, f.hom());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_FIXPOINT_HH_
