#ifndef _SDD_MEM_CACHE_HH_
#define _SDD_MEM_CACHE_HH_

#include "sdd/mem/cache_entry.hh"
#include "sdd/mem/hash_table.hh"
#include "sdd/mem/interrupt.hh"
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
    return Filter()(op) ? apply_filters<T, Filters...>()(op) : false;
  }
};

/*------------------------------------------------------------------------------------------------*/

namespace /* anonymous */ {

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

  /// @brief The number of collisions in the underlying hash table.
  std::size_t collisions;

  /// @brief The number of buckets in the underlying hash table.
  std::size_t buckets;

  /// @brief The load factor of the underlying hash table.
  double load_factor;
};

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief  A generic cache.
/// @tparam Operation is the operation type.
/// @tparam EvaluationError is the exception that the evaluation of an Operation can throw.
/// @tparam Filters is a list of filters that reject some operations.
///
/// It uses the LRU strategy to cleanup old entries.
template < typename Context, typename Operation, typename EvaluationError
         , typename... Filters>
class cache
{
  // Can't copy a cache.
  cache(const cache&) = delete;
  cache* operator=(const cache&) = delete;

private:

  /// @brief The type of the context of this cache.
  using context_type = Context;

  /// @brief The type of the result of an operation stored in the cache.
  using result_type = typename Operation::result_type;

  /// @brief The of an entry that stores an operation and its result.
  using cache_entry_type = cache_entry<Operation, result_type>;

  /// @brief An intrusive hash table.
  using set_type = mem::hash_table<cache_entry_type>;

  /// @brief This cache's context.
  context_type& cxt_;

  /// @brief The wanted load factor for the underlying hash table.
  static constexpr double max_load_factor = 0.85;

  /// @brief The actual storage of caches entries.
  set_type set_;

  /// @brief The the container that sorts cache entries by last access date.
  lru_list<Operation, result_type> lru_list_;

  /// @brief The maximum size this cache is authorized to grow to.
  std::size_t max_size_;

  /// @brief The statistics of this cache.
  mutable cache_statistics stats_;

public:

  /// @brief Construct a cache.
  /// @param context This cache's context.
  /// @param name Give a name to this cache.
  /// @param size tells how many cache entries are keeped in the cache.
  ///
  /// When the maximal size is reached, a cleanup is launched: half of the cache is removed,
  /// using a LRU strategy. This cache will never perform a rehash, therefore it allocates
  /// all the memory it needs at its construction.
  cache(context_type& context, std::size_t size)
    : cxt_(context)
    , set_(size, max_load_factor, true /* no rehash */)
    , lru_list_()
    , max_size_(set_.bucket_count() * max_load_factor)
    , stats_()
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
    // Check if the current operation should not be cached.
    if (not apply_filters<Operation, Filters...>()(op))
    {
      ++stats_.filtered;
      try
      {
        return op(cxt_);
      }
      catch (EvaluationError& e)
      {
        --stats_.filtered;
        e.add_step(std::move(op));
        throw;
      }
    }

    // Lookup for op.
    typename set_type::insert_commit_data commit_data;
    auto insertion = set_.insert_check( op
                                      , std::hash<Operation>()
                                      , [](const Operation& lhs, const cache_entry_type& rhs)
                                          {return lhs == rhs.operation;}
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
    try
    {
      entry = new cache_entry_type(std::move(op), op(cxt_));
    }
    catch (EvaluationError& e)
    {
      --stats_.misses;
      e.add_step(std::move(op));
      throw;
    }

    // Clean up the cache, if necessary.
    while (set_.size() > max_size_)
    {
      auto oldest = lru_list_.front();
      set_.erase(*oldest);
      delete oldest;
      lru_list_.pop_front();
      ++stats_.discarded;
    }

    // Add the new cache entry to the end of the LRU list.
    entry->lru_cit_ = lru_list_.insert(lru_list_.end(), entry);

    // Finally, set the result associated to op.
    set_.insert_commit(*entry, commit_data); // doesn't throw

    return entry->result;
  }

  /// @brief Remove all entries of the cache.
  void
  clear()
  noexcept
  {
    set_.clear_and_dispose([](cache_entry_type* x){delete x;});
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
    stats_.collisions = set_.collisions();
    stats_.buckets = set_.bucket_count();
    stats_.load_factor = set_.load_factor();
    return stats_;
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem

#endif // _SDD_MEM_CACHE_HH_
