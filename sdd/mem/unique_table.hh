#ifndef _SDD_MEM_UNIQUE_TABLE_HH_
#define _SDD_MEM_UNIQUE_TABLE_HH_

#include <cassert>
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

  /// @brief The number of times the underlying hash table has been rehashed.
  std::size_t rehash;
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

public:

  /// @brief Constructor.
  /// @param initial_size Initial capacity of the container.
  unique_table(std::size_t initial_size)
    : set_(initial_size), stats_()
  {}

  ~unique_table()
  {
    gc();
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
      ++stats_.hits;
      ptr->~Unique();
      delete[] reinterpret_cast<const char*>(ptr);  // match new char[] of allocate().
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
    return new char[sizeof(Unique) + extra_bytes];
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

  /// @brief Get the statistics of this unique_table.
  const unique_table_statistics&
  stats()
  const noexcept
  {
    stats_.size = set_.size();
    stats_.load_factor = set_.load_factor();
    stats_.rehash = set_.nb_rehash();
    return stats_;
  }

  /// @brief Erase all entries that are marked as unused.
  void
  gc()
  noexcept
  {
    for (const auto& u : set_)
    {
      if (not u.is_not_referenced())
      {
        u.mark();
        mark_nodes(u);
      }
    }
    static std::vector<const Unique*> to_erase(2048, nullptr);
    std::for_each( set_.begin(), set_.end()
                 , [&](const Unique& u)
                   {
                     if (u.marked())
                       u.unmark();
                     else
                       to_erase.emplace_back(&u);
                   });
    std::for_each( to_erase.begin(), to_erase.end()
                 , [this](const Unique* ptr)
                   {
                     erase(*ptr);
                   });
    to_erase.clear();
  }

private:

  static
  void
  mark_nodes(const Unique& u)
  noexcept
  {
    mark_nodes_impl(u, 0);
  }

  template <typename T>
  static
  auto
  mark_nodes_impl(const T& u, int)
  noexcept
  -> decltype(u.data().index_for_type())
  {
    apply_visitor(mark_visitor(), u.data());
  }

  template <typename T>
  static
  auto
  mark_nodes_impl(const T&, long)
  noexcept
  -> decltype(0)
  {
    return 0;
  }

  struct mark_visitor
  {
    using result_type = void;

    template <typename T>
    result_type
    operator()(const T& x)
    const noexcept
    {
      x.mark();
    }
  };
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem

#endif // _SDD_MEM_UNIQUE_TABLE_HH_
