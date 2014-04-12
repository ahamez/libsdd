#ifndef _SDD_MEM_UNIQUE_TABLE_HH_
#define _SDD_MEM_UNIQUE_TABLE_HH_

#include <array>
#include <cassert>
#include <iostream>
#include <vector>

#include "sdd/mem/hash_table.hh"

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

namespace {

/// @internal
class blocks_map
{
private:

  static constexpr std::size_t max_blocks = 4096;
  std::array<std::vector<char*>, 8> array;

public:

  ~blocks_map()
  {
    for (const auto& vec : array)
    {
      for (auto ptr : vec)
      {
        delete[] ptr;
      }
    }
  }

  template <typename Ptr>
  void
  add_block(Ptr x, std::size_t size)
  noexcept
  {
    char* ptr = reinterpret_cast<char*>(x);
    if (size > 2048)
    {
      delete[] ptr;
    }
    else
    {
      if (size < 16)
      {
        size = 16;
      }

      std::vector<char*>& vec = array[pos(size)];
      if (vec.size() == max_blocks)
      {
        delete[] ptr;
      }
      else
      {
        vec.emplace_back(ptr);
      }
    }
  }

  char*
  get_block(std::size_t size)
  noexcept
  {
    if (size > 2048)
    {
      return new char[size];
    }
    else
    {
      if (size < 16)
      {
        size = 16;
      }

      std::vector<char*>& vec = array[pos(size)];
      if (vec.empty())
      {
        // No re-usable block.
        return new char[size];
      }
      else
      {
        char* ptr = vec.back();
        vec.pop_back();
        return ptr;
      }
    }
  }

private:

  static
  std::size_t
  pos(std::size_t size)
  noexcept
  {
    size -= 1;
    std::size_t pos = 0;
    while (size >>=1)
    {
      ++pos;
    }
    return pos - 3;
  }
};

} // namespace anonymous

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
  blocks_map blocks_;

public:

  /// @brief Constructor.
  /// @param initial_size Initial capacity of the container.
  unique_table(std::size_t initial_size)
    : set_(initial_size), stats_(), blocks_()
  {}

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
      blocks_.add_block(ptr, size);
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
    return blocks_.get_block(sizeof(Unique) + extra_bytes);
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
    const std::size_t size = sizeof(Unique) + x.extra_bytes();
    x.~Unique();
    blocks_.add_block(&const_cast<Unique&>(x), size);
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
