#ifndef _SDD_MEM_HASH_TABLE_HH_
#define _SDD_MEM_HASH_TABLE_HH_

#include <algorithm>  // fill
#include <cstdint>    // uint32_t
#include <functional> // hash
#include <utility>    // make_pair, pair

#include <boost/iterator/iterator_facade.hpp>

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
/// @brief A iterator on the hash_table.
template <typename Data, typename HashTable>
class LIBSDD_ATTRIBUTE_PACKED hash_table_iterator
  : public boost::iterator_facade< hash_table_iterator<Data, HashTable>
                                 , Data, boost::forward_traversal_tag>
{
private:

  /// @brief A link to the hash table.
  const HashTable* container_;

  /// @brief The current position in the buckets.
  std::uint32_t pos_;

  /// @brief The actual data this iterator points to.
  Data* data_;

public:

  /// @brief Default constructor.
  hash_table_iterator()
  noexcept
    : container_(nullptr)
    , pos_(0)
    , data_(nullptr)
  {}

  /// @brief Constructor.
  explicit hash_table_iterator(const HashTable* container, std::uint32_t p, Data* data)
  noexcept
    : container_(container)
    , pos_(p)
    , data_(data)
  {}

  /// @brief Copy constructor from an iterator or a const_iterator.
  template <typename OtherValue>
  hash_table_iterator(const hash_table_iterator<OtherValue, HashTable>& other)
  noexcept
    : hash_table_iterator(other.container_, other.pos_, other.data_)
  {}

private:

  // Required by boost::iterator.
  friend class boost::iterator_core_access;

  // The hash table need to access internal elements of this iterator.
  friend HashTable;

  // Friend with const/non-const iterators.
  template <typename, typename> friend class hash_table_iterator;

  /// @brief For boost::iterator.
  void
  increment()
  noexcept
  {
    if (pos_ != container_->nb_buckets_) // not at the end
    {
      if (data_->hook.next != nullptr)
      {
        data_ = data_->hook.next;
      }
      else
      {
        do // find, if any, next bucket with some data in it
        {
          ++pos_;
        } while (pos_ < container_->nb_buckets_ and container_->buckets_[pos_] == nullptr);

        if (pos_ == container_->nb_buckets_) // end
        {
          data_ = nullptr;
        }
        else // non-empty bucket found
        {
          data_ = container_->buckets_[pos_];
        }
      }
    }
  }

  /// @brief For boost::iterator.
  template <typename OtherData>
  bool
  equal(const hash_table_iterator<OtherData, HashTable>& other)
  const noexcept
  {
    return pos_ == other.pos_ and data_ == other.data_;
  }

  /// @brief For boost::iterator.
  Data&
  dereference()
  const noexcept
  {
    return *data_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief An intrusive hash table.
///
/// It's modeled after boost::intrusive. Only the interfaces needed by the libsdd are implemented.
/// It uses chaining to handle collisions.
template <typename Data, typename Hash = std::hash<Data>>
class hash_table
{
public:

  /// @brief Used by insert_check
  struct insert_commit_data
  {
    std::size_t hash;
  };

  /// @brief The type of an iterator on this hash table.
  using iterator = hash_table_iterator<Data, hash_table<Data, Hash>>;

  /// @brief The type of a const iterator on this hash table.
  using const_iterator = hash_table_iterator<const Data, hash_table<Data, Hash>>;

private:

  // Iterators need to access buckets_.
  friend class hash_table_iterator<Data, hash_table<Data, Hash>>;
  friend class hash_table_iterator<const Data, hash_table<Data, Hash>>;

  /// @brief 
  std::uint32_t nb_buckets_;

  /// @brief
  std::uint32_t size_;

  /// @brief
  Data** buckets_;

  /// @brief The maximal allowed load factor.
  const double max_load_factor_;

  /// @brief The number of times this hash table has been rehashed.
  std::size_t rehash_;

  /// @brief
  const bool no_rehash_;

public:

  /// @brief Constructor
  hash_table(std::size_t size, double max_load_factor = 0.75, bool no_rehash = false)
    : nb_buckets_(util::next_power_of_2(static_cast<std::uint32_t>(size)))
    , size_(0)
    , buckets_(new Data*[nb_buckets_])
    , max_load_factor_(max_load_factor)
    , rehash_(0)
    , no_rehash_(no_rehash)
  {
    std::fill(buckets_, buckets_ + nb_buckets_, nullptr);
  }

  /// @brief Destructor
  ~hash_table()
  {
    delete[] buckets_;
  }

  /// @brief
  template <typename T, typename HashT, typename EqT>
  std::pair<iterator, bool>
  insert_check(const T& x, HashT hash, EqT eq, insert_commit_data& commit_data)
  const noexcept(noexcept(hash(x)))
  {
    commit_data.hash = hash(x);
    // same as commit_data.h % nb_buckets_, but much more efficient (works only with powers of 2)
    const std::uint32_t pos = commit_data.hash & (nb_buckets_ - 1);

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

    return std::make_pair(iterator(this, pos, current), insertion);
  }

  /// @brief
  void
  insert_commit(Data& x, const insert_commit_data& commit_data)
  noexcept
  {
    const std::uint32_t pos = commit_data.hash & (nb_buckets_ - 1);

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
    rehash();
  }

  /// @brief Insert an element.
  std::pair<iterator, bool>
  insert(Data& x)
  noexcept(noexcept(Hash()(x)))
  {
    auto res = insert_impl(&x, buckets_, nb_buckets_);
    rehash();
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

  /// @brief Get an iterator to the beginning of this hash table.
  iterator
  begin()
  noexcept
  {
    std::uint32_t pos = 0;
    while (pos < nb_buckets_ and buckets_[pos] == nullptr)
    {
      ++pos;
    }
    return pos == nb_buckets_ ? end() : iterator(this, pos, buckets_[pos]);
  }

  /// @brief Get an iterator to the end of this hash table.
  iterator
  end()
  noexcept
  {
    return iterator(this, nb_buckets_, nullptr);
  }

  /// @brief Find an element.
  iterator
  find(const Data& x)
  noexcept
  {
    const std::uint32_t pos = Hash()(x) & (nb_buckets_ - 1);
    Data* current = buckets_[pos];
    bool found = false;
    while (current != nullptr)
    {
      if (x == *current)
      {
        found = true;
        break;
      }
      current = current->hook.next;
    }

    return found ? iterator(this, pos, current) : end();
  }

  /// @brief Erase an element given its iterator.
  void
  erase(const_iterator cit)
  noexcept
  {
    Data* previous = nullptr;
    Data* current = buckets_[cit.pos_];
    const Data* data = cit.data_;

    while (current != data)
    {
      previous = current;
      current = current->hook.next;
    }

    if (previous == nullptr) // first element in bucket
    {
      buckets_[cit.pos_] = data->hook.next;
    }
    else
    {
      previous->hook.next = data->hook.next;
    }

    --size_;
  }

  /// @brief Clear the whole table.
  template <typename Disposer>
  void
  clear_and_dispose(Disposer disposer)
  {
    auto cit = begin();
    const auto e = end();
    while (cit != e)
    {
      Data* current = cit.data_;
      const auto to_erase = cit;
      ++cit;
      erase(to_erase);
      disposer(current);
    }
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
    return rehash_;
  }

private:

  void
  rehash()
  {
    if (no_rehash_ or (load_factor() < max_load_factor_))
    {
      return;
    }
    ++rehash_;
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

  /// @brief Insert an element.
  std::pair<iterator, bool>
  insert_impl(Data* x, Data** buckets, std::uint32_t nb_buckets)
  noexcept(noexcept(Hash()(*x)))
  {
    const std::uint32_t pos = Hash()(*x) & (nb_buckets - 1);

    Data* current = buckets[pos];

    while (current != nullptr)
    {
      if (*x == *current)
      {
        return std::make_pair(iterator(this, pos, current), false /* no insertion */);
      }
      current = current->hook.next;
    }

    // Push in front of the list.
    x->hook.next = buckets[pos];
    buckets[pos] = x;

    current = x;
    ++size_;
    return std::make_pair(iterator(this, pos, current), true /* insertion */);
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem

#endif // _SDD_MEM_HASH_TABLE_HH_
