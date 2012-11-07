#ifndef _SDD_HOM_DEFINITION_HH_
#define _SDD_HOM_DEFINITION_HH_

#include <iosfwd>

#include "sdd/dd/sum.hh"
#include "sdd/hom/closure.hh"
#include "sdd/hom/composition.hh"
#include "sdd/hom/cons.hh"
#include "sdd/hom/constant.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation.hh"
#include "sdd/hom/expression.hh"
#include "sdd/hom/fixpoint.hh"
#include "sdd/hom/identity.hh"
#include "sdd/hom/inductive.hh"
#include "sdd/hom/local.hh"
#include "sdd/hom/saturation_fixpoint.hh"
#include "sdd/hom/saturation_sum.hh"
#include "sdd/hom/sum.hh"
#include "sdd/hom/values_function.hh"
#include "sdd/internal/mem/ptr.hh"
#include "sdd/internal/mem/ref_counted.hh"
#include "sdd/internal/mem/variant.hh"
#include "sdd/internal/util/print_sizes_fwd.hh"

namespace sdd { namespace hom {

/*-------------------------------------------------------------------------------------------*/

/// @brief An homomorphism operation.
template <typename C>
class homomorphism
{
private:

  /// @brief A canonized homomorphism.
  typedef internal::mem::variant< const closure<C>
                                , const composition<C>
                                , const cons<C, SDD<C>>
                                , const cons<C, typename C::Values>
                                , const constant<C>
                                , const expression<C>
                                , const fixpoint<C>
                                , const identity<C>
                                , const inductive<C>
                                , const local<C>
                                , const saturation_fixpoint<C>
                                , const saturation_sum<C>
                                , const sum<C>
                                , const values_function<C>
                                >
          data_type;

  /// @brief A unified and canonized homomorphism, meant to be stored in a unique table.
  ///
  /// It is automatically erased when there is no more reference to it.
  typedef internal::mem::ref_counted<const data_type> unique_type;

  /// @brief Define the smart pointer around a unified homomorphism.
  ///
  /// It handles the reference counting as well as the deletion of the homomorphism when it is
  /// no longer referenced.
  typedef internal::mem::ptr<const unique_type> ptr_type;

  /// @brief The real smart pointer around a unified homomorphism.
  ptr_type ptr_;

public:

  /// @brief The variable type.
  typedef typename C::Variable variable_type;

  /// @brief The identifier type.
  typedef typename C::Identifier identifier_type;

  /// @brief Move constructor.
  ///
  /// O(1).
  homomorphism(homomorphism&&) noexcept = default;

  /// @brief Move operator.
  ///
  /// O(1).
  homomorphism&
  operator=(homomorphism&&) noexcept = default;

  /// @brief Copy constructor.
  ///
  /// O(1).
  homomorphism(const homomorphism&) noexcept = default;

  /// @brief Copy operator.
  ///
  /// O(1).
  homomorphism&
  operator=(const homomorphism&) noexcept = default;

  /// @brief Apply this homomorphism on an SDD.
  SDD<C>
  operator()(const order::order<C>& o, const SDD<C>& x)
  const
  {
    return (*this)(initial_context<C>(), o, x);
  }

  /// @brief Tell if this homomorphism skips a given identifier.
  bool
  skip(const order::order<C>& o)
  const noexcept
  {
    return apply_visitor(skip_helper(), ptr()->data(), o);
  }

  /// @brief Tell if this homomorphism returns only subsets.
  bool
  selector()
  const noexcept
  {
    return apply_visitor(selector_helper(), ptr()->data());
  }

/// @cond INTERNAL_DOC

  /// @brief Get the content of the homomorphism (an internal::mem::ref_counted).
  ///
  /// O(1).
  const unique_type&
  operator*()
  const noexcept
  {
    return *ptr_;
  }

  /// @brief Get a pointer to the content of the homomorphism (an internal::mem::ref_counted).
  ///
  /// O(1).
  const unique_type*
  operator->()
  const noexcept
  {
    return ptr_.operator->();
  }

  /// @brief Get the real smart pointer of the unified data.
  ///
  /// O(1).
  ptr_type
  ptr()
  const noexcept
  {
    return ptr_;
  }

  /// @brief Construct an homomorphism from a ptr.
  ///
  /// O(1).
  homomorphism(const ptr_type& ptr)
  noexcept
    : ptr_(ptr)
  {
  }

  /// @brief Construct an homomorphism from a moved ptr.
  ///
  /// O(1).
  homomorphism(ptr_type&& ptr)
  noexcept
    : ptr_(std::move(ptr))
  {
  }

  /// @brief Apply this homomorphism on an SDD, in a given context.
  SDD<C>
  operator()(context<C>& cxt, const order::order<C>& o, const SDD<C>& x)
  const
  {
    // hard-wired cases:
    // - if the current homomorphism is Id, then directly return the operand
    // - if the current operand is |0|, then directly return it
    if (*this == Id<C>() or x.empty())
    {
      return x;
    }
    return cxt.cache()(cached_homomorphism<C>(cxt, o, *this, x));
  }

  /// @brief Create an homomorphism from a concrete type (e.g. Id, Cons, etc.).
  template<typename T, typename... Args>
  static
  homomorphism
  create(internal::mem::construct<T>, Args&&... args)
  {
    unique_type* u =
      new unique_type(internal::mem::construct<T>(), std::forward<Args>(args)...);
    return homomorphism(internal::mem::unify(u));
  }

  /// @brief Dispatch the skip predicate call to concrete homomorphisms.
  struct skip_helper
  {
    typedef bool result_type;

    template <typename H>
    bool
    operator()(const H& h, const order::order<C>& o)
    const noexcept
    {
      return h.skip(o);
    }
  };

  /// @brief Dispatch the selector predicate call to concrete homomorphisms.
  struct selector_helper
  {
    typedef bool result_type;

    template <typename H>
    bool
    operator()(const H& h)
    const noexcept
    {
      return h.selector();
    }
  };

  friend void internal::util::print_sizes<C>(std::ostream&);

/// @endcond
};

/*-------------------------------------------------------------------------------------------*/

/// @related homomorphism
template <typename C>
inline
bool
operator==(const homomorphism<C>& lhs, const homomorphism<C>& rhs)
noexcept
{
  return lhs.ptr() == rhs.ptr();
}

/// @related homomorphism
template <typename C>
inline
bool
operator!=(const homomorphism<C>& lhs, const homomorphism<C>& rhs)
noexcept
{
  return not (lhs.ptr() == rhs.ptr());
}

/// @related homomorphism
template <typename C>
inline
bool
operator<(const homomorphism<C>& lhs, const homomorphism<C>& rhs)
noexcept
{
  return lhs.ptr() < rhs.ptr();
}

/// @related homomorphism
template <typename C>
std::ostream&
operator<<(std::ostream& os, const homomorphism<C>& h)
{
  return os << h->data();
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace dd::shom

namespace std {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Hash specialization for sdd::hom::homomorphism
template <typename C>
struct hash<sdd::hom::homomorphism<C>>
{
  std::size_t
  operator()(const sdd::hom::homomorphism<C>& h)
  const noexcept
  {
    return std::hash<decltype(h.ptr())>()(h.ptr());
  }
};

/// @endcond

/*-------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_DEFINITION_HH_
