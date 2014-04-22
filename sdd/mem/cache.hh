#ifndef _SDD_MEM_CACHE_HH_
#define _SDD_MEM_CACHE_HH_

#include <forward_list>
#include <iterator>  // distance
#include <list>
#include <numeric>   // accumulate
#include <utility>   // forward

#include "sdd/mem/hash_table.hh"
#include "sdd/mem/interrupt.hh"
#include "sdd/util/hash.hh"
#include "sdd/util/packed.hh"

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
///
/// A statistic is made of several rounds: each time a cache is cleaned up, a new round is
/// created. Thus, one can have detailed statistics to see how well the cache performed.
struct cache_statistics
{
  /// @internal
  /// @brief Statistic between two cleanups.
  struct round
  {
    round()
      : hits(0)
      , misses(0)
      , filtered(0)
    {}

    /// @brief The number of hits in a round.
    std::size_t hits;

    /// @brief The number of misses in a round.
    std::size_t misses;

    /// @brief The number of filtered entries in a round.
    std::size_t filtered;
  };

  /// @brief The list of all rounds.
  std::forward_list<round> rounds;

  /// @brief Default constructor.
  cache_statistics()
    : rounds(1)
  {}

  /// @brief Get the number of rounds.
  std::size_t
  size()
  const noexcept
  {
    return std::distance(rounds.begin(), rounds.end()); // forward_list lacks a size() function
  }

  /// @brief Get the number of performed cleanups.
  std::size_t
  cleanups()
  const noexcept
  {
    return size() - 1;
  }

  round
  total()
  const noexcept
  {
    return std::accumulate( rounds.begin(), rounds.end(), round()
                          , [&](const round& acc, const round& r) -> round
                               {
                                 round res;
                                 res.hits = acc.hits + r.hits;
                                 res.misses = acc.misses + r.misses;
                                 res.filtered = acc.filtered + r.filtered;
                                 return res;
                               }
                          );
  }
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

  // Forward declaration.
  struct cache_entry;

  /// @brief The type of the container that sorts cache entries by last access date.
  using lru_list_type = std::list<cache_entry*>;

  /// @brief Associate an operation to its result into the cache.
  ///
  /// The operation acts as a key and the associated result is the value counterpart.
  struct
#if defined __clang__
  LIBSDD_ATTRIBUTE_PACKED
#endif
  cache_entry
  {
    // Can't copy a cache_entry.
    cache_entry(const cache_entry&) = delete;
    cache_entry& operator=(const cache_entry&) = delete;

    /// @brief
    mem::intrusive_member_hook<cache_entry> hook;

    /// @brief The cached operation.
    const Operation operation;

    /// @brief The result of the evaluation of operation.
    const result_type result;

    /// @brief Where this cache entry is stored in the LRU list.
    typename lru_list_type::const_iterator lru_cit_;

    /// @brief Constructor.
    template <typename... Args>
    cache_entry(Operation&& op, Args&&... args)
      : hook()
      , operation(std::move(op))
      , result(std::forward<Args>(args)...)
      , lru_cit_()
    {}

    /// @brief Cache entries are only compared using their operations.
    bool
    operator==(const cache_entry& other)
    const noexcept
    {
      return operation == other.operation;
    }
  };

  /// @brief Hash a cache_entry.
  ///
  /// We only use the Operation part of a cache_entry to find it in the cache, in order
  /// to manage the set that stores cache entries as a map.
  struct hash_key
  {
    std::size_t
    operator()(const cache_entry& x)
    const noexcept(noexcept(util::hash(x.operation)))
    {
      return util::hash(x.operation);
    }
  };

  /// @brief An intrusive hash table.
  using set_type = mem::hash_table<cache_entry, hash_key>;

  /// @brief This cache's context.
  context_type& cxt_;

  /// @brief The cache name.
  const std::string name_;

  /// @brief The wanted load factor for the underlying hash table.
  static constexpr double max_load_factor = 0.85;

  /// @brief The actual storage of caches entries.
  set_type set_;

  /// @brief The the container that sorts cache entries by last access date.
  lru_list_type lru_list_;

  /// @brief The maximum size this cache is authorized to grow to.
  std::size_t max_size_;

  /// @brief The statistics of this cache.
  cache_statistics stats_;

public:

  /// @brief Construct a cache.
  /// @param context This cache's context.
  /// @param name Give a name to this cache.
  /// @param size tells how many cache entries are keeped in the cache.
  ///
  /// When the maximal size is reached, a cleanup is launched: half of the cache is removed,
  /// using a LRU strategy. This cache will never perform a rehash, therefore it allocates
  /// all the memory it needs at its construction.
  cache(context_type& context, const std::string& name, std::size_t size)
    : cxt_(context)
    , name_(name)
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
      ++stats_.rounds.front().filtered;
      try
      {
        return op(cxt_);
      }
      catch (EvaluationError& e)
      {
        --stats_.rounds.front().filtered;
        e.add_step(std::move(op));
        throw;
      }
    }

    // Lookup for op.
    typename set_type::insert_commit_data commit_data;
    auto insertion = set_.insert_check( op
                                      , std::hash<Operation>()
                                      , [](const Operation& lhs, const cache_entry& rhs)
                                          {return lhs == rhs.operation;}
                                      , commit_data);

    // Check if op has already been computed.
    if (not insertion.second)
    {
      ++stats_.rounds.front().hits;
      // Move cache entry to the end of the LRU list.
      lru_list_.erase(insertion.first->lru_cit_);
      insertion.first->lru_cit_ = lru_list_.insert(lru_list_.end(), &(*insertion.first));
      return insertion.first->result;
    }

    ++stats_.rounds.front().misses;

    cache_entry* entry;
    try
    {
      entry = new cache_entry(std::move(op), op(cxt_));
    }
    catch (EvaluationError& e)
    {
      --stats_.rounds.front().misses;
      e.add_step(std::move(op));
      throw;
    }
    catch (interrupt<result_type>&)
    {
      --stats_.rounds.front().misses;
      throw;
    }

    // Clean up the cache, if necessary.
    while (set_.size() > max_size_)
    {
      auto oldest = lru_list_.front();
      set_.erase(*oldest);
      delete oldest;
      lru_list_.pop_front();
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
    set_.clear_and_dispose([](cache_entry* x){delete x;});
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
    return stats_;
  }

  /// @brief Get this cache's name
  const std::string&
  name()
  const noexcept
  {
    return name_;
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem

#endif // _SDD_MEM_CACHE_HH_
