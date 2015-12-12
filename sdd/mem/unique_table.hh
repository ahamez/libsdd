/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <cassert>
#include <memory> // unique_ptr

#include "sdd/mem/hash_table.hh"

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @brief A unique table statistics.
struct unique_table_statistics
{
  /// @brief The actual number of unified elements.
  std::size_t size;

  /// @brief The maximum number of stored elements.
  std::size_t peak;

  /// @brief The actual load factor.
  double load_factor;

  /// @brief The total number of access.
  std::size_t access;

  /// @brief The number of hits.
  std::size_t hits;

  /// @brief The number of misses.
  std::size_t misses;

  /// @brief The number of times the underlying hash table has been rehashed.
  std::size_t rehash;

  /// @brief The number of buckets with more than one element in the underlying hash table.
  std::size_t collisions;

  /// @brief The number of buckets with only one element in the underlying hash table.
  std::size_t alone;

  /// @brief The number of empty buckets in the underlying hash table.
  std::size_t empty;

  /// @brief The number of buckets in the underlying hash table.
  std::size_t buckets;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief A table to unify data.
template <typename Unique>
class unique_table
{
  // Can't copy a unique_table.
  unique_table(const unique_table&) = delete;
  unique_table& operator=(const unique_table&) = delete;

private:

  /// @brief The actual container of unified data.
  mem::hash_table<Unique> set_;

  /// @brief The statistics of this unique_table.
  mutable unique_table_statistics stats_;

  /// @brief Keep the memory of an insertion that was a hit.
  std::unique_ptr<char[]> cache_;

  /// @brief The number of bytes of the cached memory.
  std::size_t cache_size_;

public:

  /// @brief Constructor.
  /// @param initial_size Initial capacity of the container.
  unique_table(std::size_t initial_size)
    : set_(initial_size), stats_(), cache_(nullptr), cache_size_(0)
  {}

  /// @brief Unify a data.
  /// @param ptr A pointer to a data constructed with a placement new into the storage returned by
  /// allocate().
  /// @param extra_bytes ptr might have been allocated with more bytes than sizeof(Unique); this
  /// parameter indicates this extra number of bytes.
  /// @return A reference to the unified data.
  Unique&
  operator()(Unique* ptr, std::size_t extra_bytes)
  {
    assert(ptr != nullptr);
    ++stats_.access;

    auto insertion = set_.insert(ptr);
    if (not insertion.second) // ptr already exists
    {
      ++stats_.hits;
      ptr->~Unique();
      if ((sizeof(Unique) + extra_bytes) > cache_size_)
      {
        // The inserted ptr's memory to cache is bigger than the previously held cache. Thus it
        // might fit better allocations request by allocate().
        cache_.reset(reinterpret_cast<char*>(ptr));
        cache_size_ = sizeof(Unique) + extra_bytes;
      }
      else
      {
        delete[] reinterpret_cast<char*>(ptr);  // match new char[] of allocate().
      }
    }
    else
    {
      ++stats_.misses;
      stats_.peak = std::max(stats_.peak, set_.size());
    }
    return *insertion.first;
  }

  /// @brief Allocate a memory block large enough for the given size.
  char*
  allocate(std::size_t extra_bytes)
  {
    if (cache_ and cache_size_ >= (sizeof(Unique) + extra_bytes))
    {
      // re-use cached allocation
      auto res = cache_.get();
      cache_.release();
      cache_size_ = 0;
      return res;
    }
    else
    {
      // no cached allocation or it was too small
      return new char[sizeof(Unique) + extra_bytes];
    }
  }

  /// @brief Erase the given unified data.
  ///
  /// All subsequent uses of the erased data are invalid.
  void
  erase(const Unique* x)
  noexcept
  {
    assert(x != nullptr);
    assert(x->is_not_referenced() && "Unique still referenced");
    set_.erase(x);
    x->~Unique();
    delete[] reinterpret_cast<const char*>(x); // match new char[] of allocate().
  }

  /// @brief Get the statistics of this unique_table.
  const unique_table_statistics&
  stats()
  const noexcept
  {
    stats_.size = set_.size();
    stats_.load_factor = set_.load_factor();
    stats_.rehash = set_.nb_rehash();
    std::tie(stats_.collisions, stats_.alone, stats_.empty) = set_.collisions();
    stats_.buckets = set_.bucket_count();
    return stats_;
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem
