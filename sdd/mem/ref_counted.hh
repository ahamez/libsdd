#ifndef _SDD_MEM_REF_COUNTED_HH_
#define _SDD_MEM_REF_COUNTED_HH_

#include <cassert>
#include <cstdint>     // uint32_t
#include <functional>  // hash
#include <type_traits> // is_nothrow_constructible
#include <utility>     // forward

#include <boost/intrusive/unordered_set.hpp>

#include "sdd/util/packed.hh"

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief A wrapper to associate a reference counter to a unified data.
///
/// This type is meant to be used by ptr, which takes care of incrementing and decrementing
/// the reference counter, as well as the deletion of the held data.
template <typename T>
class
#ifdef __clang__
LIBSDD_ATTRIBUTE_PACKED
#endif
ref_counted
{
  // Can't copy a ref_counted.
  ref_counted(const ref_counted&) = delete;
  ref_counted& operator=(const ref_counted&) = delete;
  
  // Can't move a ref_counted.
  ref_counted(ref_counted&&) = delete;
  ref_counted& operator=(ref_counted&&) = delete;
  
private:

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // The order is important here: data_ MUST be the last of all fields. This is necessary
  // because, for SDD, we allocate the alpha function directly behind the node, without any
  // indirection, to avoid a pointer. Thus, the address of the alpha function is computed
  // knowing this is right after the node.

  /// We choose a faster, unsafe mode.
  typedef boost::intrusive::link_mode<boost::intrusive::normal_link> link_mode;
  /// Used by Boost.Intrusive to manage the unicity table.
  boost::intrusive::unordered_set_member_hook<link_mode> member_hook_;

  /// The number of time the encapsulated a data is referenced to implement reference-counting
  /// garbage collection.
  mutable uint32_t ref_count_;

  /// @brief The garbage collected data.
  ///
  /// The ptr class is responsible for the detection of an dereferenced data and for
  /// instructing the unicity table to erase it.
  const T data_;

public:
  
  template <typename... Args>
  ref_counted(Args&&... args)
  noexcept(std::is_nothrow_constructible<T, Args...>::value)
		: member_hook_()
    , ref_count_(0)
    , data_(std::forward<Args>(args)...)
  {
  }

  /// @brief Get a reference of the unified data.
  const T&
  data()
  const noexcept
  {
    return data_;
  }

  /// @brief Get the number of extra bytes that may be used by the contained type.
  std::size_t
  extra_bytes()
  const noexcept
  {
    return data_.extra_bytes();
  }

private:

  /// A ptr should be able to access and modify the reference counter.
  template <typename U> friend class ptr;

  /// The unicity table needs to access member_hook_.
  template <typename U> friend class unique_table;

  void
  increment_reference_counter()
  const noexcept
  {
    ++ref_count_;
  }
  
  void
  decrement_reference_counter()
  const noexcept
  {
    --ref_count_;
  }

  bool
  is_not_referenced()
  const noexcept
  {
    return ref_count_ == 0;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related ref_counted
template <typename T>
inline
bool
operator==(const ref_counted<T>& lhs, const ref_counted<T>& rhs)
noexcept
{
  return lhs.data() == rhs.data();
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::mem::ref_counted
template <typename T>
struct hash<sdd::mem::ref_counted<T>>
{
  std::size_t
  operator()(const sdd::mem::ref_counted<T>& x)
  const noexcept
  {
    return std::hash<T>()(x.data());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_MEM_REF_COUNTED_HH_
