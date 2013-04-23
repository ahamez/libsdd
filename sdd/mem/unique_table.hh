#ifndef _SDD_MEM_UNIQUE_TABLE_HH_
#define _SDD_MEM_UNIQUE_TABLE_HH_

#include <cassert>

#include <boost/container/flat_map.hpp>
#include <boost/intrusive/unordered_set.hpp>

namespace sdd { namespace mem {

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

  /// The number of re-usable memory blocks to keep.
  static constexpr std::size_t nb_blocks = 4096;

  /// Index re-usable memory blocks by size.
  boost::container::flat_multimap<std::size_t, char*> blocks_;

public:

  /// @brief Constructor.
  /// @param initial_size Initial capacity of the container.
  unique_table(std::size_t initial_size)
  	: buckets_(new bucket_type[set_type::suggested_upper_bucket_count(initial_size)])
    , set_(new set_type(bucket_traits( buckets_
                                     , set_type::suggested_upper_bucket_count(initial_size))))
    , blocks_()
  {
    blocks_.reserve(nb_blocks);
  }

  /// @brief Default constructor.
  unique_table()
    : unique_table(1000000)
  {
  }

  /// @brief Destructor.
  ~unique_table()
  {
    delete set_;
    delete[] buckets_;
    for (auto& b : blocks_)
    {
      delete[] b.second;
    }
  }

  /// @brief Unify a data.
  /// @param ptr A pointer to a data constructed with a placement new into the storage returned by
  /// allocate().
  /// @return A reference to the unified data.
  const Unique&
  operator()(Unique* ptr)
  {
    if (load_factor() >= 0.9)
    {
      rehash();
    }

    auto insertion = set_->insert(*ptr);
    if (not insertion.second)
    {
      // The inserted Unique already exists. We keep its allocated memory to avoid deallocating
      // memory each time there is a hit.
      ++hit_;
      const std::size_t size = sizeof(Unique) + ptr->extra_bytes();
      ptr->~Unique();
      if (blocks_.size() == nb_blocks)
      {
        // Erase last block (the biggest one).
        auto it = blocks_.end() - 1;
        delete[] it->second;
        blocks_.erase(it);
      }
      blocks_.emplace(size, reinterpret_cast<char*>(ptr));
    }
    else
    {
      ++miss_;
    }
    return *insertion.first;
  }

  /// @brief Allocate a memory block large enough for the given size.
  char*
  allocate(std::size_t extra_bytes)
  {
    const std::size_t size = sizeof(Unique) + extra_bytes;
    const auto it = blocks_.lower_bound(size);
    if (it != blocks_.end() and it->first <= (2 * size))
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

  /// @brief Erase the given unified data.
  ///
  /// All subsequent uses of the erased data are invalid.
  void
  erase(Unique& x)
  noexcept
  {
    assert(x.is_not_referenced() && "Unique still referenced.");
    set_->erase_and_dispose(x, [](Unique* ptr){delete ptr;});
  }

  /// @brief Get the load factor of the internal hash table.
  double
  load_factor()
  const noexcept
  {
    return static_cast<double>(set_->size()) / static_cast<double>(set_->bucket_count());
  }

  /// @brief Get the number of unified elements.
  std::size_t
  size()
  const noexcept
  {
    return set_->size();
  }

private:

  /// @brief Rehash the internal hash table.
  void
  rehash()
  {
    ++rehash_;
    const std::size_t new_size = set_type::suggested_upper_bucket_count(set_->bucket_count() * 2);
    bucket_type* new_buckets = new bucket_type[new_size];
    set_->rehash(bucket_traits(new_buckets, new_size));
    delete[] buckets_;
    buckets_ = new_buckets;
  }
};

/*------------------------------------------------------------------------------------------------*/

namespace /* anonymous */ {

/// @internal
/// @related unique_table
template <typename Unique>
inline
mem::unique_table<Unique>&
global_unique_table()
noexcept
{
  static mem::unique_table<Unique> unique_table;
  return unique_table;
}

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Allocate a memory block large enough for the given size, in the global unique table.
/// @related unique_table
template <typename Unique>
inline
char*
allocate(std::size_t extra_bytes = 0)
{
  return global_unique_table<Unique>().allocate(extra_bytes);
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Unify a data in the global unique table.
/// @param u_ptr A pointer to a data constructed with a placement new into the storage returned by
/// allocate().
/// @param size The size of the data. It must be the same as the one given to allocate().
/// @related unique_table
template <typename Unique>
inline
const Unique&
unify(Unique* ptr)
{
  return global_unique_table<Unique>()(ptr);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem

#endif // _SDD_MEM_UNIQUE_TABLE_HH_
