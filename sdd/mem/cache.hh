#ifndef _SDD_MEM_CACHE_HH_
#define _SDD_MEM_CACHE_HH_

#include <algorithm> // count_if, for_each, nth_element
#include <cstdint>   // uint32_t
#include <forward_list>
#include <numeric>   // accumulate
#include <tuple>
#include <utility>   // forward
#include <vector>

#include <boost/intrusive/unordered_set.hpp>

#include "sdd/util/packed.hh"

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Used by cache to know if an operation should be cached or not.
///
/// A filter should always return the same result for the same operation.
template <typename T, typename... Filters>
struct apply_filters;

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
    {
    }

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
  {
  }

  /// @brief Get the number of rounds.
  std::size_t
  size()
  const noexcept
  {
    return std::count_if(rounds.begin(), rounds.end(), [](const round&){return true;});
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

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief  A generic cache.
/// @tparam Operation is the operation type.
/// @tparam EvaluationError is the exception that the evaluation of an Operation can throw.
/// @tparam Filters is a list of filters that reject some operations.
///
/// It uses the LFU strategy to cleanup old entries.
template <typename Context, typename Operation, typename EvaluationError, typename... Filters>
class cache
{
  // Can't copy a cache.
  cache(const cache&) = delete;
  cache* operator=(const cache&) = delete;

private:

  /// @brief The type of the context of this cache.
  typedef Context context_type;

  /// @brief The type of the result of an operation stored in the cache.
  typedef typename Operation::result_type result_type;

  /// @brief Faster, unsafe mode for Boost.Intrusive.
  typedef boost::intrusive::link_mode<boost::intrusive::normal_link> link_mode;

  /// @brief Associate an operation to its result into the cache.
  ///
  /// The operation acts as a key and the associated result is the value counterpart.
  struct
#if defined __clang__
  LIBSDD_ATTRIBUTE_PACKED
#endif
  cache_entry
    : public boost::intrusive::unordered_set_base_hook<link_mode>
  {
    // Can't copy a cache_entry.
    cache_entry(const cache_entry&) = delete;
    cache_entry& operator=(const cache_entry&) = delete;

    /// @brief The cached operation.
    const Operation operation;

    /// @brief The result of the evaluation of operation.
    const result_type result;

    /// @brief Count the number of times this entry has been accessed.
    ///
    /// Used by the LFU cache cleanup strategy.
    std::uint32_t nb_hits_;

    /// brief The 'in use' bit position in nb_hits_.
    static constexpr std::uint32_t in_use_mask = (1u << 31);

    /// @brief Constructor.
    template <typename... Args>
    cache_entry(Operation&& op, Args&&... args)
      : operation(std::move(op))
      , result(std::forward<Args>(args)...)
      , nb_hits_(in_use_mask) // initially in use
    {
    }

    /// @brief Cache entries are only compared using their operations.
    bool
    operator==(const cache_entry& other)
    const noexcept
    {
      return operation == other.operation;
    }

    /// @brief Get the number of hits, with the 'in use' bit set or not.
    unsigned int
    raw_nb_hits()
    const noexcept
    {
      return nb_hits_;
    }

    /// @brief Set this cache entry to a 'never used' state.
    void
    reset_nb_hits()
    noexcept
    {
      nb_hits_ &= in_use_mask;
    }

    /// @brief Increment the number of hits.
    void
    increment_nb_hits()
    noexcept
    {
      ++nb_hits_;
    }

    /// @brief Set this cache entry to a 'not in use' state.
    void
    reset_in_use()
    noexcept
    {
      nb_hits_ &= ~in_use_mask;
    }

    /// @brief Tell if this cache entry is in an 'in use' state.
    bool
    in_use()
    const noexcept
    {
      return nb_hits_ & in_use_mask;
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
    const noexcept
    {
      return std::hash<Operation>()(x.operation);
    }
  };

  /// @brief This cache's context.
  context_type& cxt_;

  /// @brief The cache name.
  const std::string name_;

  /// @brief The maximum size this cache is authorized to grow to.
  const std::size_t max_size_;

  /// @brief We use Boost.Intrusive to store the cache entries.
  typedef typename boost::intrusive::make_unordered_set< cache_entry
                                                       , boost::intrusive::hash<hash_key>
                                                       >::type
          set_type;

  /// @brief Needed to use Boost.Intrusive.
  typedef typename set_type::bucket_type bucket_type;

  /// @brief Needed to use Boost.Intrusive.
  typedef typename set_type::bucket_traits bucket_traits;

  /// @brief Boost.Intrusive requires us to manage ourselves its buckets.
  bucket_type* buckets_;

  /// @brief The actual storage of caches entries.
  set_type* set_;

  /// @brief The statistics of this cache.
  cache_statistics stats_;

public:

  /// @brief Construct a cache.
  /// @param context This cache's context.
  /// @param name Give a name to this cache.
  /// @param size tells how many cache entries are keeped in the cache.
  ///
  /// When the maximal size is reached, a cleanup is launched: half of the cache is removed,
  /// using an LFU strategy. This cache will never perform a rehash, therefore it allocates
  /// all the memory it needs at its construction.
  cache(context_type& context, const std::string& name, std::size_t size)
    : cxt_(context)
    , name_(name)
    , max_size_(set_type::suggested_upper_bucket_count(size))
    , buckets_(new bucket_type[max_size_])
    , set_(new set_type(bucket_traits(buckets_, max_size_)))
    , stats_()
  {
  }

  /// @brief Destructor.
  ~cache()
  {
    clear();
    delete set_;
    delete[] buckets_;
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
    auto insertion = set_->insert_check( op
                                       , std::hash<Operation>()
                                       , [](const Operation& lhs, const cache_entry& rhs)
                                           {return lhs == rhs.operation;}
                                       , commit_data);

    // Check if op has already been computed.
    if (not insertion.second)
    {
      ++stats_.rounds.front().hits;
      insertion.first->increment_nb_hits();
      return insertion.first->result;
    }

    ++stats_.rounds.front().misses;

    // Remove half of the cache if it's full.
    if (set_->size() >= max_size_)
    {
      cleanup();
    }

    try
    {
      cache_entry* entry = new cache_entry(std::move(op), op(cxt_));
      // A cache entry is constructed with the 'in use' bit set.
      entry->reset_in_use();
      set_->insert_commit(*entry, commit_data); // doesn't throw
      return entry->result;
    }
    catch (EvaluationError& e)
    {
      --stats_.rounds.front().misses;
      e.add_step(std::move(op));
      throw;
    }
  }

  /// @brief Remove half of the cache (LFU strategy).
  void
  cleanup()
  {
    typedef typename set_type::iterator iterator;

    stats_.rounds.emplace_front(cache_statistics::round());

    std::vector<iterator> vec;
    vec.reserve(set_->size());

    for (auto cit = set_->begin(); cit != set_->end(); ++cit)
    {
      vec.push_back(cit);
    }

    // We remove an half of the cache.
    const std::size_t cut_size = vec.size() / 2;

    // Find the median of the number of hits.
    std::nth_element( vec.begin(), vec.begin() + cut_size, vec.end()
                    , [](iterator lhs, iterator rhs)
                        {
                          // We sort using the raw nb_hits counter, with the 'in use' possibily bit
                          // set. As this is the higher bit, the value of nb_hits may be large.
                          // Thus, it's very unlikely that cache entries currently in use will be
                          // put below the median.
                          return lhs->raw_nb_hits() < rhs->raw_nb_hits();
                        });

    // Delete all cache entries with a number of entries smaller than the median.
    std::for_each( vec.begin(), vec.begin() + cut_size
                 , [&](iterator cit)
                      {
                        if (not cit->in_use())
                        {
                          const cache_entry* x = &*cit;
                          set_->erase(cit);
                          delete x;
                        }
                      });

    // Reset the number of hits of all remaining cache entries.
    std::for_each(vec.begin() + cut_size, vec.end(), [](iterator it){it->reset_nb_hits();});
  }

  /// @brief Remove all entries of the cache.
  void
  clear()
  noexcept
  {
    set_->clear_and_dispose([](cache_entry* x){delete x;});
  }

  /// @brief Get the number of cached operations.
  std::size_t
  size()
  const noexcept
  {
    return set_->size();
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
