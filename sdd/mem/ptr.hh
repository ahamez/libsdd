#ifndef _SDD_MEM_PTR_HH_
#define _SDD_MEM_PTR_HH_

#include <functional> // hash

#include "sdd/mem/unique_table.hh"

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief  A smart pointer to manage unified ressources.
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
  const Unique* x_;

public:

  /// @brief Constructor with a unified data.
  ptr(const Unique& p)
  noexcept
  	: x_(&p)
  {
    x_->increment_reference_counter();
  }

  /// @brief Copy constructor.
  ptr(const ptr& other)
  noexcept
    : x_(other.x_)
  {
    if (x_ != nullptr)
    {
      x_->increment_reference_counter();
    }
  }

  /// @brief Copy operator.
  ptr&
  operator=(const ptr& other)
  noexcept
  {
    if (not (*this == other))
    {
      if (x_ != nullptr)
      {
        x_->decrement_reference_counter();
        erase_x_if_necessary();
      }
      x_ = other.x_;
      if (x_ != nullptr)
      {
        x_->increment_reference_counter();
      }
    }
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
    if (not (*this == other))
    {
      if (x_ != nullptr)
      {
        x_->decrement_reference_counter();
        erase_x_if_necessary();
      }
      x_ = other.x_;
      other.x_ = nullptr;
    }
    return *this;
  }

  /// @brief Destructor.
  ~ptr()
  {
    if (x_ != nullptr)
    {
      x_->decrement_reference_counter();
      erase_x_if_necessary();
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

private:

  /// @brief If the managed data is no longer referenced, erase it.
  void
  erase_x_if_necessary()
  noexcept
  {
    if (x_->is_not_referenced())
    {
      typedef typename std::remove_const<Unique>::type U;
      U& x = *const_cast<U*>(x_);
      global_unique_table<U>().erase(x);
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related ptr
///
/// O(1).
template <typename Unique>
inline
bool
operator==(const ptr<Unique>& lhs, const ptr<Unique>& rhs)
noexcept
{
  return lhs.operator->() == rhs.operator->();
}

/// @internal
/// @related ptr
///
/// O(1).
template <typename Unique>
inline
bool
operator<(const ptr<Unique>& lhs, const ptr<Unique>& rhs)
noexcept
{
  return lhs.operator->() < rhs.operator->();
}

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
    return std::hash<Unique*>()(x.operator->());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_MEM_PTR_HH_
