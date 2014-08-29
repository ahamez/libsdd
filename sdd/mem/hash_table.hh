#pragma once

#include <algorithm>   // fill
#include <functional>  // hash
#include <type_traits> // enable_if
#include <utility>     // make_pair, pair

#include "sdd/util/next_power.hh"
#include "sdd/util/packed.hh"

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief It must be stored by the hash table's data as a member named 'hook'.
template <typename Data>
struct intrusive_member_hook
{
  /// @brief Store the next data in a bucket.
  mutable Data* next;

  /// @brief Default constructor.
  intrusive_member_hook()
    : next(nullptr)
  {}
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief An intrusive hash table.
///
/// It's modeled after boost::intrusive. Only the interfaces needed by the libsdd are implemented.
/// It uses chaining to handle collisions.
template <typename Data, bool Rehash = true>
class hash_table
{
public:

  /// @brief Used by insert_check
  struct insert_commit_data
  {
    std::size_t hash;
  };

private:

  /// @brief 
  std::size_t nb_buckets_;

  /// @brief
  std::size_t size_;

  /// @brief
  Data** buckets_;

  /// @brief The maximal allowed load factor.
  const double max_load_factor_;

  /// @brief The number of times this hash table has been rehashed.
  std::size_t nb_rehash_;

public:

  /// @brief Constructor
  hash_table(std::size_t size, double max_load_factor = 0.75)
    : nb_buckets_(util::next_power_of_2(size))
    , size_(0)
    , buckets_(new Data*[nb_buckets_])
    , max_load_factor_(max_load_factor)
    , nb_rehash_(0)
  {
    std::fill(buckets_, buckets_ + nb_buckets_, nullptr);
  }

  /// @brief Destructor
  ~hash_table()
  {
    delete[] buckets_;
  }

  /// @brief
  template <typename T, typename EqT>
  std::pair<Data*, bool>
  insert_check(const T& x, EqT eq, insert_commit_data& commit_data)
  const noexcept(noexcept(std::hash<T>()(x)))
  {
    commit_data.hash = std::hash<T>()(x);
    // same as commit_data.h % nb_buckets_, but much more efficient (works only with powers of 2)
    const std::size_t pos = commit_data.hash & (nb_buckets_ - 1);

    Data* current = buckets_[pos];
    bool insertion = true;
    while (current != nullptr)
    {
      if (eq(x, *current))
      {
        insertion = false;
        break;
      }
      current = current->hook.next;
    }

    return std::make_pair(current, insertion);
  }

  /// @brief
  void
  insert_commit(Data& x, const insert_commit_data& commit_data)
  noexcept(Rehash == false)
  {
    const std::size_t pos = commit_data.hash & (nb_buckets_ - 1);

    Data* previous = nullptr;
    Data* current = buckets_[pos];

    while (current != nullptr)
    {
      if (x == *current)
      {
        return;
      }
      previous = current;
      current = current->hook.next;
    }

    if (previous != nullptr)
    {
      previous->hook.next = &x;
    }
    else
    {
      buckets_[pos] = &x;
    }

    ++size_;

    rehash<Rehash>();
  }

  /// @brief Insert an element.
  std::pair<Data*, bool>
  insert(Data& x)
  {
    auto res = insert_impl(&x, buckets_, nb_buckets_);
    rehash<Rehash>();
    return res;
  }

  /// @brief Return the number of elements.
  std::size_t
  size()
  const noexcept
  {
    return size_;
  }

  /// @brief Return the number of buckets.
  std::size_t
  bucket_count()
  const noexcept
  {
    return nb_buckets_;
  }

  /// @brief Remove an element given its value.
  void
  erase(const Data& x)
  noexcept
  {
    const std::size_t pos = std::hash<Data>()(x) & (nb_buckets_ - 1);
    Data* previous = nullptr;
    Data* current = buckets_[pos];
    while (current != nullptr)
    {
      if (x == *current)
      {
        if (previous == nullptr) // first element in bucket
        {
          buckets_[pos] = current->hook.next;
        }
        else
        {
          previous->hook.next = current->hook.next;
        }
        --size_;
        return;
      }
      previous = current;
      current = current->hook.next;
    }
    assert(false && "Data to erase not found");
  }

  /// @brief Clear the whole table.
  template <typename Disposer>
  void
  clear_and_dispose(Disposer disposer)
  {
    for (std::size_t i = 0; i < nb_buckets_; ++i)
    {
      Data* current = buckets_[i];
      while (current != nullptr)
      {
        const auto to_erase = current;
        current = current->hook.next;
        disposer(to_erase);
      }
      buckets_[i] = nullptr;
    }
    size_ = 0;
  }

  /// @brief Get the load factor of the internal hash table.
  double
  load_factor()
  const noexcept
  {
    return static_cast<double>(size()) / static_cast<double>(bucket_count());
  }

  /// @brief The number of times this hash table has been rehashed.
  std::size_t
  nb_rehash()
  const noexcept
  {
    return nb_rehash_;
  }

  /// @brief The number of collisions.
  std::size_t
  collisions()
  const noexcept
  {
    std::size_t c = 0;
    for (std::size_t i = 0; i < nb_buckets_; ++i)
    {
      std::size_t nb = 0;
      auto current = buckets_[i];
      while (current != nullptr)
      {
        ++nb;
        current = current->hook.next;
      }
      if (nb > 1) ++c;
    }
    return c;
  }

private:

  template<bool DoRehash>
  std::enable_if_t<DoRehash, void>
  rehash()
  {
    if ((load_factor() < max_load_factor_))
    {
      return;
    }
    ++nb_rehash_;
    auto new_nb_buckets = nb_buckets_ * 2;
    auto new_buckets = new Data*[new_nb_buckets];
    std::fill(new_buckets, new_buckets + new_nb_buckets, nullptr);
    size_ = 0;
    for (std::size_t i = 0; i < nb_buckets_; ++i)
    {
      Data* data_ptr = buckets_[i];
      while (data_ptr)
      {
        Data* next = data_ptr->hook.next;
        data_ptr->hook.next = nullptr;
        insert_impl(data_ptr, new_buckets, new_nb_buckets);
        data_ptr = next;
      }
      // else empty bucket
    }
    std::swap(new_buckets, buckets_);
    std::swap(new_nb_buckets, nb_buckets_);
    delete[] new_buckets;
  }

  template<bool DoRehash>
  std::enable_if_t<not DoRehash, void>
  rehash()
  noexcept
  {}

  /// @brief Insert an element.
  std::pair<Data*, bool>
  insert_impl(Data* x, Data** buckets, std::size_t nb_buckets)
  noexcept(noexcept(std::hash<Data>()(*x)))
  {
    const std::size_t pos = std::hash<Data>()(*x) & (nb_buckets - 1);

    Data* current = buckets[pos];

    while (current != nullptr)
    {
      if (*x == *current)
      {
        return std::make_pair(current, false /* no insertion */);
      }
      current = current->hook.next;
    }

    // Push in front of the list.
    x->hook.next = buckets[pos];
    buckets[pos] = x;

    current = x;
    ++size_;
    return std::make_pair(current, true /* insertion */);
  }
};



/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem
