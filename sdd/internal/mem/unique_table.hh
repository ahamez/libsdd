#ifndef _SDD_INTERNAL_MEM_UNIQUE_TABLE_HH_
#define _SDD_INTERNAL_MEM_UNIQUE_TABLE_HH_

/// @cond INTERNAL_DOC

#include <boost/container/flat_map.hpp>
#include <boost/intrusive/unordered_set.hpp>

namespace sdd { namespace internal { namespace mem {

/*-------------------------------------------------------------------------------------------*/

template <typename Unique>
class unique_table
{
  // Can't copy a unique_table.
  unique_table(const unique_table&) = delete;
  unique_table& operator=(const unique_table&) = delete;

private:

  /// @brief We choose a faster, unsafe mode.
  typedef boost::intrusive::link_mode<boost::intrusive::normal_link> link_mode;

  /// @brief Tell Boost.Intrusive how to access to the member hook.
  typedef boost::intrusive::member_hook< Unique
                                       , boost::intrusive::unordered_set_member_hook<link_mode>
                                       , &Unique::member_hook_>
          member_hook;

  /// @brief Tell Boost.Intrusive we want to use the standard hash mechanism.
  typedef boost::intrusive::hash<std::hash<Unique>> hash_option;

  /// @brief The type of the set that helps to unify data.
  typedef typename boost::intrusive::make_unordered_set<Unique, member_hook, hash_option>::type
          set_type;

  /// @brief A type needed by Boost.Intrusive.
  typedef typename set_type::bucket_type bucket_type;

  /// @brief A type needed by Boost.Intrusive.
  typedef typename set_type::bucket_traits bucket_traits;

  /// @brief The buckets needed by Boost.Intrusive.
  bucket_type* buckets_;

  /// @brief The actual container of unified data.
  set_type* set_;

  /// @brief The number of hits.
  std::size_t hit_;

  /// @brief The number of misses.
  std::size_t miss_;

  /// @brief The number of rehash.
  std::size_t rehash_;

  ///
  boost::container::flat_multimap<std::size_t, char*> blocks_;

public:

  unique_table(std::size_t initial_size)
  	: buckets_(new bucket_type[set_type::suggested_upper_bucket_count(initial_size)])
    , set_(new set_type(bucket_traits( buckets_
                                     , set_type::suggested_upper_bucket_count(initial_size))))
    , blocks_()
  {
  }

  unique_table()
    : unique_table(1000000)
  {
  }

  ~unique_table()
  {
    delete set_;
    delete[] buckets_;
    for (auto& b : blocks_)
    {
      delete[] b.second;
    }
  }

  const Unique&
  operator()(Unique* u_ptr, std::size_t size)
  {
    if (load_factor() >= 0.9)
    {
      ++rehash_;
      rehash();
    }

    auto insertion = set_->insert(*u_ptr);
    if (not insertion.second)
    {
      ++hit_;
      u_ptr->~Unique();
      if (blocks_.size() == 2048)
      {
        // erase last block
        auto it = blocks_.end() - 1;
        delete[] it->second;
        blocks_.erase(it);
      }
      blocks_.emplace(size, reinterpret_cast<char*>(u_ptr));
    }
    else
    {
      ++miss_;
    }
    return *insertion.first;
  }

  char*
  allocate(std::size_t size)
  {
    const auto it = blocks_.find(size);
    if (it != blocks_.end())
    {
      // re-use allocated blocks
      char* addr = it->second;
      blocks_.erase(it);
      return addr;
    }
    else
    {
      return new char[size];
    }
  }

  void
  erase(Unique& x)
  noexcept
  {
    set_->erase_and_dispose(x, [](Unique* ptr){delete ptr;});
  }

  double
  load_factor()
  const noexcept
  {
    return static_cast<double>(set_->size()) / static_cast<double>(set_->bucket_count());
  }

  std::size_t
  size()
  const noexcept
  {
    return set_->size();
  }

private:

  void
  rehash()
  {
    const std::size_t new_size = set_->bucket_count() * 2;
    bucket_type* new_buckets = new bucket_type[new_size];
    set_->rehash(bucket_traits(new_buckets, new_size));
    delete[] buckets_;
    buckets_ = new_buckets;
  }
};

/*-------------------------------------------------------------------------------------------*/

namespace /* anonymous */ {

template <typename Unique>
inline
internal::mem::unique_table<Unique>&
global_unique_table()
noexcept
{
  static internal::mem::unique_table<Unique> unique_table;
  return unique_table;
}

} // namespace anonymous

/*-------------------------------------------------------------------------------------------*/

/// @related unique_table
template <typename Unique>
inline
char*
allocate(std::size_t size)
{
  return global_unique_table<Unique>().allocate(size);
}

/*-------------------------------------------------------------------------------------------*/

template <typename Unique>
inline
const Unique&
unify(Unique* u_ptr, std::size_t size)
{
  return global_unique_table<Unique>()(u_ptr, size);
}

/*-------------------------------------------------------------------------------------------*/

}}} // namespace sdd::internal::mem

/// @endcond

#endif // _SDD_INTERNAL_MEM_UNIQUE_TABLE_HH_
