#pragma once

#include <vector>

#include "sdd/mem/cache_entry_fwd.hh"

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename Entry>
class lru_list
{
private:

  using entry_type = Entry;

  struct lru_entry
  {
    lru_entry* prev;
    lru_entry* next;
    entry_type* entry;
  };

  union lru_or_free
  {
    lru_or_free* next_free;
    lru_entry data;
  };

  /// @brief Store both lru entries and free blocks.
  std::vector<lru_or_free> vec_;

  /// @brief The oldest entry.
  lru_entry* oldest_;

  /// @brief The newest entry.
  lru_entry* newest_;

  /// @brief The first free block.
  lru_or_free* first_free_;

public:

  /// @brief A link to an lru_entry.
  using lru_entry_access = lru_entry*;

  /// @brief Constructor.
  lru_list(std::size_t size)
    : vec_(size), oldest_(nullptr), newest_(nullptr), first_free_(&vec_[0])
  {
    for (auto i = 0u; i < size; ++i)
    {
      vec_[i].next_free = &vec_[i + 1];
    }
  }

  /// @brief Add a new cache entry.
  lru_entry_access
  push(entry_type* e)
  noexcept
  {
    if (newest_ == nullptr) // empty list
    {
      assert(oldest_ == nullptr);
      newest_ = oldest_ = &first_free_->data;
      first_free_ = first_free_->next_free;
      newest_->prev = nullptr;
      newest_->next = nullptr;
      newest_->entry = e;
    }
    else
    {
      auto previous_newest = newest_;
      newest_ = &first_free_->data;
      first_free_ = first_free_->next_free;
      newest_->prev = previous_newest;
      previous_newest->next = newest_;
      newest_->next = nullptr;
      newest_->entry = e;
    }
    return newest_;
  }

  /// @brief Move an lru entry to the newest position.
  void
  touch(lru_entry_access access)
  noexcept
  {
    assert(access && "access = nullptr");
    assert(newest_ && "newest_ = nullptr");
    if (newest_ == access) // nothing to do
    {
      return;
    }
    else if (access->prev != nullptr)
    {
      access->prev->next = access->next;
    }
    else // touching oldest entry
    {
      oldest_ = oldest_->next;
      oldest_->prev = nullptr;
    }
    newest_->next = access;
    access->prev = newest_;
    access->next = nullptr;
    newest_ = access;
  }

  /// @brief Remove the oldest entry.
  entry_type*
  pop()
  noexcept
  {
    assert(newest_ != nullptr && "newest_ == nullptr");
    assert(oldest_ != nullptr && "oldest_ == nullptr");

    auto e = oldest_->entry;
    auto previous_first_free = first_free_;
    
    if (oldest_ == newest_)
    {
      first_free_ = reinterpret_cast<lru_or_free*>(oldest_);
      oldest_ = nullptr;
      newest_ = nullptr;
    }
    else
    {
      auto previous_oldest = oldest_;
      oldest_ = oldest_->next;
      first_free_ = reinterpret_cast<lru_or_free*>(previous_oldest);
    }
    first_free_->next_free = previous_first_free;
    return e;
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem
