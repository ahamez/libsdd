/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <memory> // unique_ptr
#include <tuple>

#include "sdd/mem/cache_entry.hh"
#include "sdd/mem/hash_table.hh"
#include "sdd/mem/lru_list.hh"
#include "sdd/util/hash.hh"

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Used by cache to know if an operation should be cached or not.
///
/// A filter should always return the same result for the same operation.
template <typename T, typename... Filters>
struct apply_filters;

/// @internal
/// @brief Base case.
///
/// All filters have been applied and they all accepted the operation.
template <typename T>
struct apply_filters<T>
{
  constexpr
  bool
  operator()(const T&)
  const
  {
    return true;
  }
};

/// @internal
/// @brief Recursive case.
///
/// Chain filters calls: as soon as a filter reject an operation, the evaluation is stopped.
template <typename T, typename Filter, typename... Filters>
struct apply_filters<T, Filter, Filters...>
{
  bool
  operator()(const T& op)
  const
  {
    return Filter()(op) and apply_filters<T, Filters...>()(op);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The statistics of a cache.
struct cache_statistics
{
  /// @brief The number of entries.
  std::size_t size;

  /// @brief The number of hits.
  std::size_t hits;

  /// @brief The number of misses.
  std::size_t misses;

  /// @brief The number of filtered entries.
  std::size_t filtered;

  /// @brief The number of entries discarded by the LRU policy.
  std::size_t discarded;

  /// @brief The number of buckets with more than one element in the underlying hash table.
  std::size_t collisions;

  /// @brief The number of buckets with only one element in the underlying hash table.
  std::size_t alone;

  /// @brief The number of empty buckets in the underlying hash table.
  std::size_t empty;

  /// @brief The number of buckets in the underlying hash table.
  std::size_t buckets;

  /// @brief The load factor of the underlying hash table.
  double load_factor;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief  A generic cache.
/// @tparam Operation is the operation type.
/// @tparam Filters is a list of filters that reject some operations.
///
/// It uses the LRU strategy to cleanup old entries.
template <typename Context, typename Operation, typename... Filters>
class cache
{
  // Can't copy a cache.
  cache(const cache&) = delete;
  cache* operator=(const cache&) = delete;

private:

  /// @brief The type of the context of this cache.
  using context_type = Context;

  /// @brief The type of the result of an operation stored in the cache.
  using result_type = std::result_of_t<Operation(context_type&)>;

  /// @brief The of an entry that stores an operation and its result.
  using cache_entry_type = cache_entry<Operation, result_type>;

  /// @brief An intrusive hash table.
  using set_type = mem::hash_table<cache_entry_type, false /* no rehash */>;

  /// @brief This cache's context.
  context_type& cxt_;

  /// @brief The wanted load factor for the underlying hash table.
  static constexpr double max_load_factor = 0.85;

  /// @brief The actual storage of caches entries.
  set_type set_;

  /// @brief The the container that sorts cache entries by last access date.
  lru_list<cache_entry_type> lru_list_;

  /// @brief The maximum size this cache is authorized to grow to.
  std::size_t max_size_;

  /// @brief The statistics of this cache.
  mutable cache_statistics stats_;

  /// @brief Fixed-size pool allocator for cache entries.
  struct pool
  {
    union node
    {
      node* next;
      unsigned char data[sizeof(cache_entry_type)];
    };

    std::unique_ptr<node[]> head;
    node* free_list;

    pool(std::size_t size)
      : head(std::make_unique<node[]>(size)), free_list(head.get())
    {
      for (std::size_t i = 0; i < size - 1; ++i)
      {
        free_list[i].next = &free_list[i+1];
      }
      free_list[size - 1].next = nullptr;
    }

    void*
    allocate()
    noexcept
    {
      assert(free_list != nullptr);
      void* p = free_list;
      free_list = free_list->next;
      return p;
    }

    void
    deallocate(void* ptr)
    noexcept
    {
      assert(ptr != nullptr);
      node* p = static_cast<node*>(ptr);
      p->next = free_list;
      free_list = p;
    }
  } pool_;

public:

  /// @brief Construct a cache.
  /// @param context This cache's context.
  /// @param size How many cache entries are kept, should be greater than the order height.
  ///
  /// When the maximal size is reached, a cleanup is launched: half of the cache is removed,
  /// using a LRU strategy. This cache will never perform a rehash, therefore it allocates
  /// all the memory it needs at its construction.
  cache(context_type& context, std::size_t size)
    : cxt_(context)
    , set_(size, max_load_factor)
    , lru_list_()
    , max_size_(set_.bucket_count() * max_load_factor)
    , stats_()
    , pool_(max_size_)
  {}

  /// @brief Destructor.
  ~cache()
  {
    clear();
  }

  /// @brief Cache lookup.
  result_type
  operator()(Operation&& op)
  {
    // Check if the current operation should be cached or not.
    if (not apply_filters<Operation, Filters...>()(op))
    {
      ++stats_.filtered;
      return op(cxt_);
    }

    // Lookup for op.
    typename set_type::insert_commit_data commit_data;
    auto insertion = set_.insert_check( op
                                      , [](auto&& lhs, auto&& rhs){return lhs == rhs.operation;}
                                      , commit_data);

    // Check if op has already been computed.
    if (not insertion.second)
    {
      ++stats_.hits;
      // Move cache entry to the end of the LRU list.
      lru_list_.splice(lru_list_.end(), lru_list_, insertion.first->lru_cit_);
      return insertion.first->result;
    }

    ++stats_.misses;

    cache_entry_type* entry;
    auto res = op(cxt_); // evaluation may throw

    // Clean up the cache, if necessary.
    if (set_.size() == max_size_)
    {
      auto oldest = lru_list_.front();
      set_.erase(oldest);
      oldest->~cache_entry_type();
      pool_.deallocate(oldest);
      lru_list_.pop_front();
      ++stats_.discarded;
    }

    entry = new (pool_.allocate()) cache_entry_type(std::move(op), std::move(res));

    // Add the new cache entry to the end of the LRU list.
    entry->lru_cit_ = lru_list_.insert(lru_list_.end(), entry);

    // Finally, set the result associated to op.
    set_.insert_commit(entry, commit_data); // doesn't throw

    return entry->result;
  }

  /// @brief Remove all entries of the cache.
  void
  clear()
  noexcept
  {
    set_.clear_and_dispose([&](cache_entry_type* x)
                              {
                                x->~cache_entry_type();
                                pool_.deallocate(x);
                              });
  }

  /// @brief Get the number of cached operations.
  std::size_t
  size()
  const noexcept
  {
    return set_.size();
  }

  /// @brief Get the statistics of this cache.
  const cache_statistics&
  statistics()
  const noexcept
  {
    stats_.size = size();
    std::tie(stats_.collisions, stats_.alone, stats_.empty) = set_.collisions();
    stats_.buckets = set_.bucket_count();
    stats_.load_factor = set_.load_factor();
    return stats_;
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem
