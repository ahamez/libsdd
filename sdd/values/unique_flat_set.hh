#ifndef _SDD_VALUES_UNIQUE_FLAT_SET_HH_
#define _SDD_VALUES_UNIQUE_FLAT_SET_HH_

#include <algorithm>  // copy, set_difference, set_intersection, set_union
#include <functional> // hash
#include <initializer_list>
#include <iosfwd>
#include <iterator>   // inserter
#include <utility>    // pair

#include <boost/container/flat_set.hpp>
#include <boost/intrusive/unordered_set.hpp>

#include "sdd/internal/util/hash.hh"

namespace sdd { namespace values {

/*------------------------------------------------------------------------------------------------*/

/// @brief A unified set of values, implemented with a sorted vector.
template <typename Value>
class unique_flat_set
{
public:

  /// @brief The type of the real container.
  typedef boost::container::flat_set<Value> flat_set_type;

  /// @brief The type of the contained value.
  typedef Value value_type;

private:

  /// @brief Faster, unsafe mode for Boost.Intrusive.
  typedef boost::intrusive::link_mode<boost::intrusive::normal_link> link_mode;

  /// @brief An entry in the unique table of set of values.
  struct entry
    : public boost::intrusive::unordered_set_base_hook<link_mode>
  {
    // Can't copy an entry.
    entry(const entry&) = delete;
    entry& operator=(const entry&) = delete;

    /// @brief The unified flat set.
    const flat_set_type data;

    /// @brief Constructor.
    template <typename... Args>
    entry(Args&&... args)
      : data(std::forward<Args>(args)...)
    {
    }

    /// @brief Comparison.
    bool
    operator==(const entry& other)
    const noexcept
    {
      return data == other.data;
    }
  };

  /// @brief Hash an entry.
  struct hash
  {
    std::size_t
    operator()(const entry& e)
    const noexcept
    {
      std::size_t seed = 0;
      for (const auto& v : e.data)
      {
        internal::util::hash_combine(seed, v);
      }
      return seed;
    }
  };

  /// @brief The type of the set of flat sets.
  typedef typename boost::intrusive::make_unordered_set<entry, boost::intrusive::hash<hash>>::type
          set_type;
  typedef typename set_type::bucket_type bucket_type;
  typedef typename set_type::bucket_traits bucket_traits;

  /// @brief A pointer to the unified set of values.
  const flat_set_type* data_;

public:

  /// @brief The type of an iterator on a flat set of values.
  typedef typename flat_set_type::const_iterator const_iterator;

  /// @brief Default constructor.
  unique_flat_set()
    : data_(empty_set())
  {
  }

  /// @brief Constructor with a range.
  template <typename InputIterator>
  unique_flat_set(InputIterator begin, InputIterator end)
    : data_(unify(begin, end))
  {
  }

  /// @brief Constructor with a initializer_list.
  unique_flat_set(std::initializer_list<Value> values)
    : unique_flat_set(values.begin(), values.end())
  {
  }

  /// @brief Constructor from a temporary flat_set_type.
  unique_flat_set(flat_set_type&& fs)
    : data_(unify(std::move(fs)))
  {
  }

  unique_flat_set(const unique_flat_set&) = default;
  unique_flat_set& operator=(const unique_flat_set&) = default;
  unique_flat_set(unique_flat_set&&) = default;
  unique_flat_set& operator=(unique_flat_set&&) = default;

  /// @brief Insert a value.
  void
  insert(const Value& x)
  {
    flat_set_type s(*data_);
    const auto insertion = s.insert(x);
    if (insertion.second)
    {
      data_ = unify(std::move(s));
    }
  }

  /// @brief Get the beginning of this set of values.
  const_iterator
  cbegin()
  const noexcept
  {
    return data_->cbegin();
  }

  /// @brief Get the end of this set of values.
  const_iterator
  cend()
  const noexcept
  {
    return data_->cend();
  }

  /// @brief Tell if this set of values is empty.
  bool
  empty()
  const noexcept
  {
    return data_->empty();
  }

  /// @brief Get the number of contained values.
  std::size_t
  size()
  const noexcept
  {
    return data_->size();
  }

  /// @brief Find a value.
  const_iterator
  find(const Value& x)
  const
  {
    return data_->find(x);
  }

  /// @brief Erase a value.
  std::size_t
  erase(const Value& x)
  {
    flat_set_type fs(*data_);
    const std::size_t nb_erased = fs.erase(x);
    unify(std::move(fs));
    return nb_erased;
  }

  /// @internal
  /// @brief Get the pointer to the unified data.
  const flat_set_type* const
  data()
  const noexcept
  {
    return data_;
  }

private:

  /// @brief Help cleaning the static set.
  struct set_disposer
  {
    bucket_type* buckets;
    set_type* set;

    set_disposer(std::size_t size)
      : buckets(new bucket_type[size])
      , set(new set_type(bucket_traits(buckets, size)))
    {
    }

    ~set_disposer()
    {
      set->clear_and_dispose([](entry* e){delete e;});
      delete set;
      delete[] buckets;
    }
  };

  /// @brief Get the static set of flat sets.
  static
  set_type&
  set()
  {
    static set_disposer disposer(32000);
    return *disposer.set;
  }

  /// @brief Get the static empty flat set.
  static
  const flat_set_type*
  empty_set()
  {
    static auto e = unify(flat_set_type());
    return e;
  }

  /// @brief Unify a flat_set_type using a unique table.
  template <typename... Args>
  static
  const flat_set_type*
  unify(Args&&... args)
  {
    entry* ptr = new entry(std::forward<Args>(args)...);
    const auto insertion = set().insert(*ptr);
    if (not insertion.second)
    {
      delete ptr;
    }
    return &(insertion.first->data);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Equality of unique_flat_set
/// @related unique_flat_set
template <typename Value>
inline
bool
operator==(const unique_flat_set<Value>& lhs, const unique_flat_set<Value>& rhs)
noexcept
{
  // Pointer equality.
  return lhs.data() == rhs.data();
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Comparison of unique_flat_set
/// @related unique_flat_set
///
/// The order on unique_flat_set is arbitrary. 
template <typename Value>
inline
bool
operator<(const unique_flat_set<Value>& lhs, const unique_flat_set<Value>& rhs)
noexcept
{
  // Pointer comparison.
  return lhs.data() < rhs.data();
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Textual output of a unique_flat_set
/// @related unique_flat_set
template <typename Value>
std::ostream&
operator<<(std::ostream& os, const unique_flat_set<Value>& fs)
{
  os << "{";
  if (not fs.empty())
  {
    std::copy(fs.cbegin(), std::prev(fs.cend()), std::ostream_iterator<Value>(os, ","));
    os << *std::prev(fs.cend());
  }
  return os << "}";
}

/*------------------------------------------------------------------------------------------------*/

/// @related unique_flat_set
template <typename Value>
inline
unique_flat_set<Value>
difference(const unique_flat_set<Value>& lhs, const unique_flat_set<Value>& rhs)
noexcept
{
  typename unique_flat_set<Value>::flat_set_type res;
  std::set_difference( lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend()
                     , std::inserter(res, res.begin()));
  return unique_flat_set<Value>(std::move(res));
}

/*------------------------------------------------------------------------------------------------*/

/// @related unique_flat_set
template <typename Value>
inline
unique_flat_set<Value>
intersection(const unique_flat_set<Value>& lhs, const unique_flat_set<Value>& rhs)
noexcept
{
  typename unique_flat_set<Value>::flat_set_type res;
  std::set_intersection( lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend()
                       , std::inserter(res, res.begin()));
  return unique_flat_set<Value>(std::move(res));
}

/*------------------------------------------------------------------------------------------------*/

/// @related unique_flat_set
template <typename Value>
inline
unique_flat_set<Value>
sum(const unique_flat_set<Value>& lhs, const unique_flat_set<Value>& rhs)
noexcept
{
  typename unique_flat_set<Value>::flat_set_type res;
  std::set_union( lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend()
                , std::inserter(res, res.begin()));
  return unique_flat_set<Value>(std::move(res));
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::values

namespace std {

/// @brief Hash specialization for sdd::values::flat_set
template <typename Value>
struct hash<sdd::values::unique_flat_set<Value>>
{
  std::size_t
  operator()(const sdd::values::unique_flat_set<Value>& fs)
  const noexcept
  {
    return std::hash<decltype(fs.data())>()(fs.data());
  }
};

} // namespace std

#endif // _SDD_VALUES_UNIQUE_FLAT_SET_HH_
