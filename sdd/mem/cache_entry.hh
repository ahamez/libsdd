#pragma once

#include <functional> // hash
#include <list>
#include <utility>    // forward

#include "sdd/mem/cache_entry_fwd.hh"
#include "sdd/mem/hash_table.hh"
#include "sdd/mem/lru_list.hh"
#include "sdd/util/hash.hh"

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Associate an operation to its result into the cache.
///
/// The operation acts as a key and the associated result is the value counterpart.
template <typename Operation, typename Result>
struct cache_entry
{
  // Can't copy a cache_entry.
  cache_entry(const cache_entry&) = delete;
  cache_entry& operator=(const cache_entry&) = delete;

  /// @brief
  mem::intrusive_member_hook<cache_entry> hook;

  /// @brief The cached operation.
  const Operation operation;

  /// @brief The result of the evaluation of operation.
  const Result result;

  /// @brief Where this cache entry is stored in the LRU list.
  typename lru_list<Operation, Result>::const_iterator lru_cit_;

  /// @brief Constructor.
  template <typename... Args>
  cache_entry(Operation&& op, Args&&... args)
    : hook()
    , operation(std::move(op))
    , result(std::forward<Args>(args)...)
    , lru_cit_()
  {}

  /// @brief Cache entries are only compared using their operations.
  friend
  bool
  operator==(const cache_entry& lhs, const cache_entry& rhs)
  noexcept
  {
    return lhs.operation == rhs.operation;
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename Operation, typename Result>
struct hash<sdd::mem::cache_entry<Operation, Result>>
{
  std::size_t
  operator()(const sdd::mem::cache_entry<Operation, Result>& x)
  {
    using namespace sdd::hash;
    return seed() (val(x.operation));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

/*------------------------------------------------------------------------------------------------*/
