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
template <typename Data>
struct intrusive_member_hook
{
  mutable Data* next;

  intrusive_member_hook()
    : next(nullptr)
  {}
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename Data, typename HashTable>
class LIBSDD_ATTRIBUTE_PACKED hash_table_iterator
  : public boost::iterator_facade< hash_table_iterator<Data, HashTable>
                                 , Data, boost::forward_traversal_tag>
{
//private:
public:

  /// @brief
  const HashTable* container_;

  /// @brief
  std::uint32_t pos_;

  /// @brief
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

  friend class boost::iterator_core_access;
  friend HashTable;
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
template <typename Data, typename Hash = std::hash<Data>>
class hash_table
{
public:

  /// @brief Used by insert_check
  struct insert_commit_data
  {
    std::size_t hash;
  };

  /// @brief
  typedef hash_table_iterator<Data, hash_table<Data, Hash>> iterator;

  /// @brief
  typedef hash_table_iterator<const Data, hash_table<Data, Hash>> const_iterator;

private:

  friend class hash_table_iterator<Data, hash_table<Data, Hash>>;

  /// @brief
  std::uint32_t nb_buckets_;

  /// @brief
  std::uint32_t size_;

  /// @brief
  Data** buckets_;

public:

  /// @brief Constructor
  hash_table(std::uint32_t size)
    : nb_buckets_(util::next_power_of_2(size))
    , size_(0)
    , buckets_(new Data*[nb_buckets_])
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
  const noexcept
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

    // an other thread might have added x, we must check again
    // mettre un flag dans la bucket (en plus du mutex) pour savoir
    // si une insertion a été faite entre temps? comme ça on évite la boucle
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
  }

  /// @brief
  std::pair<iterator, bool>
  insert(Data& x)
  noexcept
  {
    const std::uint32_t pos = Hash()(x) & (nb_buckets_ - 1);

    Data* previous = nullptr;
    Data* current = buckets_[pos];
    bool insertion = true;

    while (current != nullptr)
    {
      if (x == *current)
      {
        insertion = false;
        break;
      }
      previous = current;
      current = current->hook.next;
    }

    if (insertion)
    {
      if (previous != nullptr)
      {
        previous->hook.next = &x;
      }
      else
      {
        buckets_[pos] = &x;
      }

      current = &x;
      ++size_;
    }

    return std::make_pair(iterator(this, pos, current), insertion);
  }

  /// @brief Return the number of elements.
  std::size_t
  size()
  const noexcept
  {
    return size_;
  }

  /// @brief
  std::size_t
  nb_buckets()
  const noexcept
  {
    return nb_buckets_;
  }

  /// @brief
  iterator
  begin()
  noexcept
  {
    std::uint32_t pos = 0;
    while (pos < nb_buckets_ and buckets_[pos] == nullptr)
    {
      ++pos;
    }

    if (pos == nb_buckets_)
    {
      return end();
    }
    else
    {
      return iterator(this, pos, buckets_[pos]);
    }
  }

  /// @brief
  iterator
  end()
  noexcept
  {
    return iterator(this, nb_buckets_, nullptr);
  }

  /// @brief
  iterator
  find(Data& x)
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

    if (found)
    {
      return iterator(this, pos, current);
    }
    else
    {
      return end();
    }
  }

  /// @brief
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

  /// @brief
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
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem

#endif // _SDD_MEM_HASH_TABLE_HH_
