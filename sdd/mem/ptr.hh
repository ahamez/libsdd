/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <cassert>
#include <functional>  // hash, function
#include <type_traits> // remove_const

#include "sdd/mem/unique_table.hh"
#include "sdd/util/hash.hh"

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Define the type of a deletion handler.
///
/// A deletion handler is called by ptr whenever a data is no longer referenced.
template <typename Unique>
using handler_type = std::function<void (const Unique*)>;

/// @internal
/// @brief Get the deletion handler for a given Unique type.
template <typename Unique>
handler_type<Unique>&
deletion_handler()
{
  static handler_type<Unique> handler = [](const Unique*){assert(false && "Unset handler");};
  return handler;
}

/// @internal
/// @brief Reset the deletion handler for a given Unique type.
template <typename Unique>
void
reset_deletion_handler()
{
  deletion_handler<Unique>() = [](const Unique*){assert(false && "Reset handler");};
}


/// @internal
/// @brief Set the deletion handler for a given Unique type.
template <typename Unique>
void
set_deletion_handler(const handler_type<Unique>& h)
{
  deletion_handler<Unique>() = h;
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief A smart pointer to manage unified ressources.
/// @tparam Unique the type of the unified ressource.
///
/// Unified ressources are ref_counted elements constructed with an unique_table.
template <typename Unique>
class ptr
{
  // Can't default construct a ptr.
  ptr() = delete;

private:

  /// @brief Pointer to the managed ressource, a unified data.
  Unique* x_;

public:

  /// @brief Constructor with a unified data.
  explicit
  ptr(Unique* p)
  noexcept
  	: x_(p)
  {
    x_->increment_reference_counter();
  }

  /// @brief Copy constructor.
  ptr(const ptr& other)
  noexcept
    : x_(other.x_)
  {
    assert(other.x_ != nullptr);
    x_->increment_reference_counter();
  }

  /// @brief Copy operator.
  ptr&
  operator=(const ptr& other)
  noexcept
  {
    assert(other.x_ != nullptr); // Don't copy from an already moved ptr.
    if (x_ != nullptr)
    {
      x_->decrement_reference_counter();
      if (x_->is_not_referenced())
      {
        deletion_handler<Unique>()(x_);
      }
    }
    x_ = other.x_;
    x_->increment_reference_counter();
    return *this;
  }

  /// @brief Move constructor.
  ptr(ptr&& other)
  noexcept
    : x_(other.x_)
  {
    other.x_ = nullptr;
  }

  /// @brief Move operator.
  ptr&
  operator=(ptr&& other)
  noexcept
  {
    if (x_ != nullptr)
    {
      x_->decrement_reference_counter();
      if (x_->is_not_referenced())
      {
        deletion_handler<Unique>()(x_);
      }
    }
    x_ = other.x_;
    other.x_ = nullptr;
    return *this;
  }

  /// @brief Destructor.
  ~ptr()
  {
    if (x_ != nullptr)
    {
      x_->decrement_reference_counter();
      if (x_->is_not_referenced())
      {
        deletion_handler<Unique>()(x_);
      }
    }
  }

  /// @brief Get a reference to the unified data.
  const Unique&
  operator*()
  const noexcept
  {
    return *x_;
  }

  /// @brief Get a pointer to the unified data.
  const Unique*
  operator->()
  const noexcept
  {
    return x_;
  }

  /// @brief Swap.
  friend void
  swap(ptr& lhs, ptr& rhs)
  noexcept
  {
    using std::swap;
    swap(lhs.x_, rhs.x_);
  }

  /// @brief Swap.
  friend void
  swap(ptr&& lhs, ptr& rhs)
  noexcept
  {
    using std::swap;
    swap(lhs.x_, rhs.x_);
  }

  /// @brief Swap.
  friend void
  swap(ptr& lhs, ptr&& rhs)
  noexcept
  {
    using std::swap;
    swap(lhs.x_, rhs.x_);
  }

  friend
  bool
  operator==(const ptr& lhs, const ptr& rhs)
  noexcept
  {
    return lhs.x_ == rhs.x_;
  }

  friend
  bool
  operator<(const ptr& lhs, const ptr& rhs)
  noexcept
  {
    return lhs.x_ < rhs.x_;
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::mem::ptr
template <typename Unique>
struct hash<sdd::mem::ptr<Unique>>
{
  std::size_t
  operator()(const sdd::mem::ptr<Unique>& x)
  const noexcept
  {
    return sdd::hash::seed(x.operator->());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
