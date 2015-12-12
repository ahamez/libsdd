/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

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
#include "sdd/mem/unique.hh"
#include "sdd/mem/variant.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief An homomorphism operation.
template <typename C>
class homomorphism final
{
private:

  /// @brief A canonized homomorphism.
  using data_type =  mem::variant< hom::_composition<C>
                                 , hom::_cons<C, SDD<C>>
                                 , hom::_cons<C, typename C::Values>
                                 , hom::_constant<C>
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
                                 , hom::_sum<C>>;

public:

  /// @internal
  /// @brief A unified and canonized homomorphism, meant to be stored in a unique table.
  ///
  /// It is automatically erased when there is no more reference to it.
  using unique_type = mem::unique<data_type>;

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

  /// @brief Apply this homomorphism on an SDD.
  SDD<C>
  operator()(const order<C>& o, SDD<C>&& x)
  const
  {
    return this->operator()(global<C>().hom_context, o, std::move(x));
  }

  /// @internal
  /// @brief Tell if this homomorphism skips a given identifier.
  ///
  /// The identifier considered is the head of the given order (order::identifier()).
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return visit([&](const auto& h){return h.skip(o);}, *this);
  }

  /// @internal
  /// @brief Tell if this homomorphism returns only subsets.
  bool
  selector()
  const noexcept
  {
    return visit([](const auto& h){return h.selector();}, *this);
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
  const ptr_type&
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
    : ptr_{ptr}
  {}

  /// @internal
  /// @brief Construct an homomorphism from a ptr.
  ///
  /// O(1).
  homomorphism(ptr_type&& ptr)
  noexcept
    : ptr_{std::move(ptr)}
  {}

  /// @internal
  /// @brief Apply this homomorphism on an SDD, in a given context.
  template <typename SDD_>
  SDD<C>
  operator()(hom::context<C>& cxt, const order<C>& o, SDD_&& x)
  const
  {
    // hard-wired cases:
    // - if the current homomorphism is Id, then directly return the operand
    // - if the current operand is |0|, then directly return it
    return *this == id<C>() or x.empty()
         ? x
         : cxt.cache()({o, *this, std::forward<SDD_>(x)});
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
};

/*------------------------------------------------------------------------------------------------*/

namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Create an homomorphism from a concrete type of variable size (e.g. sum).
template<typename C, typename T, typename... Args>
homomorphism<C>
make_variable_size(std::size_t extra_bytes, Args&&... args)
{
  using unique_type = typename homomorphism<C>::unique_type;
  using ptr_type = typename homomorphism<C>::ptr_type;
  auto& ut = global<C>().hom_unique_table;
  char* addr = ut.allocate(extra_bytes);
  unique_type* u = new (addr) unique_type(mem::construct<T>(), std::forward<Args>(args)...);
  return {ptr_type(&ut(u, extra_bytes))};
}

/// @internal
/// @brief Create an homomorphism from a concrete type of fixed size (e.g. Id, cons, etc.).
template<typename C, typename T, typename... Args>
homomorphism<C>
make(Args&&... args)
{
  return make_variable_size<C, T>(0, std::forward<Args>(args)...);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

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
    return sdd::hash::seed(h.ptr());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
