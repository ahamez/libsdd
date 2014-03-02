#ifndef _SDD_MEM_UNIQUE_TABLE_HH_
#define _SDD_MEM_UNIQUE_TABLE_HH_

#include <cassert>

#include "sdd/mem/hash_table.hh"
#include "sdd/util/boost_flat_map_no_warnings.hh"

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

namespace /* anonymous */ {

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
};

} // namespace anonymous

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

  /// @brief Index re-usable memory blocks by size.
  boost::container::flat_multimap<std::size_t, char*> blocks_;

  /// @brief The number of re-usable memory blocks to keep.
  static constexpr std::size_t nb_blocks = 4096;

public:

  /// @brief Constructor.
  /// @param initial_size Initial capacity of the container.
  unique_table(std::size_t initial_size)
    : set_(initial_size)
    , stats_()
    , blocks_()
  {
    blocks_.reserve(nb_blocks);
  }

  /// @brief Destructor.
  ~unique_table()
  {
    for (auto& b : blocks_)
    {
      delete[] b.second;
    }
  }

  /// @brief Unify a data.
  /// @param ptr A pointer to a data constructed with a placement new into the storage returned by
  /// allocate().
  /// @return A reference to the unified data.
  Unique&
  operator()(Unique* ptr)
  {
    ++stats_.access;

    auto insertion = set_.insert(*ptr);
    if (not insertion.second)
    {
      // The inserted Unique already exists. We keep its allocated memory to avoid deallocating
      // memory each time there is a hit.
      ++stats_.hits;
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
      ++stats_.misses;
      stats_.peak = std::max(stats_.peak, set_.size());
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
  erase(const Unique& x)
  noexcept
  {
    assert(x.is_not_referenced() && "Unique still referenced");
    const auto cit = set_.find(x);
    assert(cit != set_.end() && "Unique not found");
    set_.erase(cit);
    x.~Unique();
    delete[] reinterpret_cast<const char*>(&x); // match new char[] of allocate().
  }

  /// @brief Get the load factor of the internal hash table.
  double
  load_factor()
  const noexcept
  {
    return static_cast<double>(set_.size()) / static_cast<double>(set_.bucket_count());
  }

  /// @brief Get the statistics of this unique_table.
  const unique_table_statistics&
  stats()
  const noexcept
  {
    stats_.size = set_.size();
    stats_.load_factor = load_factor();
    return stats_;
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem

#endif // _SDD_MEM_UNIQUE_TABLE_HH_
