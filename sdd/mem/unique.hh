/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

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
template <typename T>
class
#ifdef __clang__
LIBSDD_ATTRIBUTE_PACKED
#endif
unique
{
  unique(const unique&) = delete;
  unique& operator=(const unique&) = delete;
  unique(unique&&) = delete;
  unique& operator=(unique&&) = delete;

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // The order is important here: data_ MUST be the last of all fields. This is necessary
  // because, for SDD, we allocate the alpha function directly behind the node, without any
  // indirection, to avoid a pointer. Thus, the address of the alpha function is computed
  // knowing this is right after the node.

private:

  /// @brief Used by mem::hash_table to store some informations.
  mem::intrusive_member_hook<unique> hook;

  /// @brief The number of time the encapsulated data is referenced
  ///
  /// Implements a reference-counting garbage collection.
  std::uint32_t ref_count_;

  /// @brief The garbage collected data.
  ///
  /// The ptr class is responsible for the detection of dereferenced data and for
  /// instructing the unicity table to erase it.
  const T data_;

public:
  
  template <typename... Args>
  unique(Args&&... args)
  noexcept(std::is_nothrow_constructible<T, Args...>::value)
    : hook(), ref_count_(0), data_(std::forward<Args>(args)...)
  {}

  /// @brief Get a reference of the unified data.
  const T&
  data()
  const noexcept
  {
    return data_;
  }

  /// @brief Tell if the unified data is no longer referenced.
  bool
  is_not_referenced()
  const noexcept
  {
    return ref_count_ == 0;
  }

  /// @brief Equality.
  friend
  bool
  operator==(const unique& lhs, const unique& rhs)
  noexcept
  {
    return lhs.data_ == rhs.data_;
  }

  /// @brief A ptr references that unified data.
  void
  increment_reference_counter()
  noexcept
  {
    assert(ref_count_ < std::numeric_limits<uint32_t>::max());
    ++ref_count_;
  }

  /// @brief A ptr no longer references that unified data.
  void
  decrement_reference_counter()
  noexcept
  {
    assert(ref_count_ > 0);
    --ref_count_;
  }

  // hash_table needs to access the hook.
  template <typename, bool> friend class hash_table;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::mem::unique
template <typename T>
struct hash<sdd::mem::unique<T>>
{
  std::size_t
  operator()(const sdd::mem::unique<T>& x)
  const noexcept(noexcept(hash<T>()(x.data())))
  {
    return hash<T>()(x.data());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
