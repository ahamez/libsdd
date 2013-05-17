#ifndef _SDD_MEM_REF_COUNTED_HH_
#define _SDD_MEM_REF_COUNTED_HH_

#include <cassert>
#include <cstdint>     // uint32_t
#include <functional>  // hash
#include <limits>      // numeric_limits
#include <type_traits> // is_nothrow_constructible
#include <utility>     // forward

#include "sdd/util/packed.hh"

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief A wrapper to associate a reference counter to a unified data.
///
/// This type is meant to be used by ptr, which takes care of incrementing and decrementing
/// the reference counter, as well as the deletion of the held data.
template <typename T, typename Hash = std::hash<T>>
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

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // The order is important here: data_ MUST be the last of all fields. This is necessary
  // because, for SDD, we allocate the alpha function directly behind the node, without any
  // indirection, to avoid a pointer. Thus, the address of the alpha function is computed
  // knowing this is right after the node.

public:

  /// @brief
  mem::intrusive_member_hook<ref_counted> hook;

private:

  /// The number of time the encapsulated data is referenced (reference-counting garbage collection)
  mutable uint32_t ref_count_;

  /// @brief The garbage collected data.
  ///
  /// The ptr class is responsible for the detection of dereferenced data and for
  /// instructing the unicity table to erase it.
  const T data_;

public:
  
  template <typename... Args>
  ref_counted(Args&&... args)
  noexcept(std::is_nothrow_constructible<T, Args...>::value)
    : hook()
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
  ///
  /// This information is needed when a data of variable length is allocated.
  std::size_t
  extra_bytes()
  const noexcept
  {
    // Static dispatch.
    return extra_bytes_impl(data_, 0);
  }

  /// @brief Called when the contained type defines extra_bytes.
  template <typename U>
  static auto
  extra_bytes_impl(const U& x, int)
  noexcept
  -> decltype(x.extra_bytes())
  {
    return x.extra_bytes();
  }

  /// @brief Called when the contained type doesn't define extra_bytes.
  template <typename U>
  static constexpr auto
  extra_bytes_impl(const U&, long)
  noexcept
  -> decltype(0)
  {
    return 0;
  }

  /// @brief Tell if the unified data is no longer referenced.
  bool
  is_not_referenced()
  const noexcept
  {
    return ref_count_ == 0;
  }

private:

  /// A ptr should be able to access and modify the reference counter.
  template <typename U> friend class ptr;

  /// @brief A ptr references that unified data.
  void
  increment_reference_counter()
  const noexcept
  {
    assert(ref_count_ < std::numeric_limits<uint32_t>::max());
    ++ref_count_;
  }

  /// @brief A ptr no longer references that unified data.
  void
  decrement_reference_counter()
  const noexcept
  {
    assert(ref_count_ > 0);
    --ref_count_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related ref_counted
template <typename T, typename Hash>
inline
bool
operator==(const ref_counted<T, Hash>& lhs, const ref_counted<T, Hash>& rhs)
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
template <typename T, typename Hash>
struct hash<sdd::mem::ref_counted<T,Hash>>
{
  std::size_t
  operator()(const sdd::mem::ref_counted<T, Hash>& x)
  const noexcept
  {
    return Hash()(x.data());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_MEM_REF_COUNTED_HH_
