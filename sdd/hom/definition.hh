#pragma once

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
#include "sdd/hom/function.hh"
#include "sdd/hom/identity.hh"
#include "sdd/hom/if_then_else.hh"
#include "sdd/hom/inductive.hh"
#include "sdd/hom/intersection.hh"
#include "sdd/hom/local.hh"
#include "sdd/hom/saturation_fixpoint.hh"
#include "sdd/hom/saturation_intersection.hh"
#include "sdd/hom/saturation_sum.hh"
#include "sdd/hom/sum.hh"
#include "sdd/mem/ptr.hh"
#include "sdd/mem/ref_counted.hh"
#include "sdd/mem/variant.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief An homomorphism operation.
template <typename C>
class homomorphism final
{
private:

  /// @brief A canonized homomorphism.
  typedef mem::variant< hom::_composition<C>
                      , hom::_cons<C, SDD<C>>
                      , hom::_cons<C, typename C::Values>
                      , hom::_constant<C>
                      , hom::_expression<C>
                      , hom::_fixpoint<C>
                      , hom::_function<C>
                      , hom::_identity<C>
                      , hom::_if_then_else<C>
                      , hom::_inductive<C>
                      , hom::_intersection<C>
                      , hom::_local<C>
                      , hom::_saturation_fixpoint<C>
                      , hom::_saturation_intersection<C>
                      , hom::_saturation_sum<C>
                      , hom::_simple_expression<C>
                      , hom::_sum<C>
                      >
          data_type;

public:

  /// @internal
  /// @brief A unified and canonized homomorphism, meant to be stored in a unique table.
  ///
  /// It is automatically erased when there is no more reference to it.
  using unique_type = mem::ref_counted<data_type>;

  /// @internal
  /// @brief Define the smart pointer around a unified homomorphism.
  ///
  /// It handles the reference counting as well as the deletion of the homomorphism when it is
  /// no longer referenced.
  using ptr_type = mem::ptr<unique_type>;

private:

  /// @brief The real smart pointer around a unified homomorphism.
  ptr_type ptr_;

public:

  /// @brief Copy constructor.
  ///
  /// O(1).
  homomorphism(const homomorphism&) noexcept = default;

  /// @brief Copy operator.
  ///
  /// O(1).
  homomorphism&
  operator=(const homomorphism&) noexcept = default;

  /// @brief Move constructor.
  ///
  /// O(1).
  homomorphism(homomorphism&&) noexcept = default;

  /// @brief Move operator.
  ///
  /// O(1).
  homomorphism&
  operator=(homomorphism&&) noexcept = default;

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
    return visit(skip_helper(), *this, o);
  }

  /// @internal
  /// @brief Tell if this homomorphism returns only subsets.
  bool
  selector()
  const noexcept
  {
    return visit(selector_helper(), *this);
  }

  /// @internal
  /// @brief Get the content (of type mem::ref_counted) of the homomorphism.
  ///
  /// O(1).
  const data_type&
  operator*()
  const noexcept
  {
    return ptr_->data();
  }

  /// @internal
  /// @brief Get a pointer to the content (of type mem::ref_counted) of the homomorphism.
  ///
  /// O(1).
  const data_type*
  operator->()
  const noexcept
  {
    return &ptr_->data();
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
  {}

  /// @internal
  /// @brief Apply this homomorphism on an SDD, in a given context.
  SDD<C>
  operator()(hom::context<C>& cxt, const order<C>& o, const SDD<C>& x)
  const
  {
    // hard-wired cases:
    // - if the current homomorphism is Id, then directly return the operand
    // - if the current operand is |0|, then directly return it
    if (*this == id<C>() or x.empty())
    {
      return x;
    }
    return cxt.cache()(hom::cached_homomorphism<C>(o, *this, x));
  }

  /// @internal
  /// @brief Create an homomorphism from a concrete type of fixed size (e.g. Id, cons, etc.).
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
  /// @brief Create an homomorphism from a concrete type of variable size (e.g. sum).
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

  /// @brief Equality.
  ///
  /// O(1)
  friend
  bool
  operator==(const homomorphism& lhs, const homomorphism& rhs)
  noexcept
  {
    return lhs.ptr_ == rhs.ptr_;
  }

  /// @brief Inequality.
  ///
  /// O(1)
  friend
  bool
  operator!=(const homomorphism<C>& lhs, const homomorphism& rhs)
  noexcept
  {
    return not (lhs.ptr_ == rhs.ptr_);
  }

  /// @brief Less than comparison.
  ///
  /// O(1). The order is arbitrary and can change at each run.
  friend
  bool
  operator<(const homomorphism<C>& lhs, const homomorphism& rhs)
  noexcept
  {
    return lhs.ptr_ < rhs.ptr_;
  }

  /// @brief homomorphism textual output.
  friend
  std::ostream&
  operator<<(std::ostream& os, const homomorphism& h)
  {
    return os << *h;
  }

private:

  /// @internal
  /// @brief Dispatch the skip predicate call to concrete homomorphisms.
  struct skip_helper
  {
    using result_type = bool;

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
    using result_type = bool;

    template <typename H>
    bool
    operator()(const H& h)
    const noexcept
    {
      return h.selector();
    }
  };
};

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
    return sdd::util::hash(h.ptr());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
