#ifndef _SDD_HOM_DEFINITION_HH_
#define _SDD_HOM_DEFINITION_HH_

#include <iosfwd>

#include "sdd/internal_manager_fwd.hh"
#include "sdd/dd/definition.hh"
#include "sdd/dd/sum.hh"
#include "sdd/hom/composition.hh"
#include "sdd/hom/cons.hh"
#include "sdd/hom/constant.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation.hh"
#include "sdd/hom/expression.hh"
#include "sdd/hom/fixpoint.hh"
#include "sdd/hom/identity.hh"
#include "sdd/hom/inductive.hh"
#include "sdd/hom/intersection.hh"
#include "sdd/hom/local.hh"
#include "sdd/hom/saturation_fixpoint.hh"
#include "sdd/hom/saturation_sum.hh"
#include "sdd/hom/sum.hh"
#include "sdd/hom/values_function.hh"
#include "sdd/mem/ptr.hh"
#include "sdd/mem/ref_counted.hh"
#include "sdd/mem/variant.hh"
#include "sdd/util/print_sizes_fwd.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief An homomorphism operation.
template <typename C>
class homomorphism final
{
private:

  /// @brief A canonized homomorphism.
  typedef mem::variant< hom::composition<C>
                      , hom::cons<C, SDD<C>>
                      , hom::cons<C, typename C::Values>
                      , hom::constant<C>
                      , hom::expression<C>
                      , hom::fixpoint<C>
                      , hom::identity<C>
                      , hom::inductive<C>
                      , hom::intersection<C>
                      , hom::local<C>
                      , hom::saturation_fixpoint<C>
                      , hom::saturation_sum<C>
                      , hom::sum<C>
                      , hom::values_function<C>
                      >
          data_type;

public:

  /// @internal
  /// @brief A unified and canonized homomorphism, meant to be stored in a unique table.
  ///
  /// It is automatically erased when there is no more reference to it.
  typedef mem::ref_counted<data_type> unique_type;

  /// @internal
  /// @brief Define the smart pointer around a unified homomorphism.
  ///
  /// It handles the reference counting as well as the deletion of the homomorphism when it is
  /// no longer referenced.
  typedef mem::ptr<unique_type> ptr_type;

private:

  /// @brief The real smart pointer around a unified homomorphism.
  ptr_type ptr_;

public:

  /// @brief The variable type.
  typedef typename C::Variable variable_type;

  /// @brief The identifier type.
  typedef typename C::Identifier identifier_type;

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
  operator()(const order<C>& o, const SDD<C>& x)
  const
  {
    return this->operator()(global<C>().hom_context, o, x);
  }

  /// @internal
  /// @brief Tell if this homomorphism skips a given identifier.
  ///
  /// The identifier considered is the head of the given order (order::identifier()).
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return apply_visitor(skip_helper(), ptr()->data(), o);
  }

  /// @internal
  /// @brief Tell if this homomorphism returns only subsets.
  bool
  selector()
  const noexcept
  {
    return apply_visitor(selector_helper(), ptr()->data());
  }

  /// @internal
  /// @brief Get the content of the homomorphism (an mem::ref_counted).
  ///
  /// O(1).
  const unique_type&
  operator*()
  const noexcept
  {
    return *ptr_;
  }

  /// @internal
  /// @brief Get a pointer to the content of the homomorphism (an mem::ref_counted).
  ///
  /// O(1).
  const unique_type*
  operator->()
  const noexcept
  {
    return ptr_.operator->();
  }

  /// @internal
  /// @brief Get the real smart pointer of the unified data.
  ///
  /// O(1).
  ptr_type
  ptr()
  const noexcept
  {
    return ptr_;
  }

  /// @internal
  /// @brief Construct an homomorphism from a ptr.
  ///
  /// O(1).
  homomorphism(const ptr_type& ptr)
  noexcept
    : ptr_(ptr)
  {
  }

  /// @internal
  /// @brief Construct an homomorphism from a moved ptr.
  ///
  /// O(1).
  homomorphism(ptr_type&& ptr)
  noexcept
    : ptr_(std::move(ptr))
  {
  }

  /// @internal
  /// @brief Apply this homomorphism on an SDD, in a given context.
  SDD<C>
  operator()(hom::context<C>& cxt, const order<C>& o, const SDD<C>& x)
  const
  {
    // hard-wired cases:
    // - if the current homomorphism is Id, then directly return the operand
    // - if the current operand is |0|, then directly return it
    if (*this == Id<C>() or x.empty())
    {
      return x;
    }
    return cxt.cache()(hom::cached_homomorphism<C>(o, *this, x));
  }

  /// @internal
  /// @brief Create an homomorphism from a concrete type of fixed size (e.g. Id, Cons, etc.).
  template<typename T, typename... Args>
  static
  homomorphism
  create(mem::construct<T>, Args&&... args)
  {
    auto& ut = global<C>().hom_unique_table;
    char* addr = ut.allocate(0 /*extra_bytes*/);
    unique_type* u = new (addr) unique_type(mem::construct<T>(), std::forward<Args>(args)...);
    return {ptr_type(ut(u))};
  }

  /// @internal
  /// @brief Create an homomorphism from a concrete type of variable size (e.g. Sum).
  template<typename T, typename... Args>
  static
  homomorphism
  create_variable_size(mem::construct<T>, std::size_t extra_bytes, Args&&... args)
  {
    auto& ut = global<C>().hom_unique_table;
    char* addr = ut.allocate(extra_bytes);
    unique_type* u = new (addr) unique_type(mem::construct<T>(), std::forward<Args>(args)...);
    return {ptr_type(ut(u))};
  }

  /// @internal
  /// @brief Dispatch the skip predicate call to concrete homomorphisms.
  struct skip_helper
  {
    typedef bool result_type;

    template <typename H>
    bool
    operator()(const H& h, const order<C>& o)
    const noexcept
    {
      return h.skip(o);
    }
  };

  /// @internal
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

  friend void util::print_sizes<C>(std::ostream&);
};

/*------------------------------------------------------------------------------------------------*/

/// @brief homomorphism equality.
/// @related homomorphism
///
/// O(1)
template <typename C>
inline
bool
operator==(const homomorphism<C>& lhs, const homomorphism<C>& rhs)
noexcept
{
  return lhs.ptr() == rhs.ptr();
}

/// @brief homomorphism inequality.
/// @related homomorphism
///
/// O(1)
template <typename C>
inline
bool
operator!=(const homomorphism<C>& lhs, const homomorphism<C>& rhs)
noexcept
{
  return not (lhs.ptr() == rhs.ptr());
}

/// @brief homomorphism comparison.
/// @related homomorphism
///
/// O(1)
template <typename C>
inline
bool
operator<(const homomorphism<C>& lhs, const homomorphism<C>& rhs)
noexcept
{
  return lhs.ptr() < rhs.ptr();
}

/// @brief homomorphism textual output.
/// @related homomorphism
template <typename C>
std::ostream&
operator<<(std::ostream& os, const homomorphism<C>& h)
{
  return os << h->data();
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::homomorphism
template <typename C>
struct hash<sdd::homomorphism<C>>
{
  std::size_t
  operator()(const sdd::homomorphism<C>& h)
  const noexcept
  {
    return std::hash<decltype(h.ptr())>()(h.ptr());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_DEFINITION_HH_
