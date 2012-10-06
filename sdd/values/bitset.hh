#ifndef _SDD_VALUES_BISET_HH_
#define _SDD_VALUES_BISET_HH_

#include <bitset>
#include <functional> // hash
#include <initializer_list>
#include <iosfwd>
#include <limits>
#include <type_traits>

namespace sdd { namespace values {

/*-------------------------------------------------------------------------------------------*/

/// @brief Encode a set of values using bits.
template <std::size_t Size>
class bitset
{
private:
  
  friend struct std::hash<bitset<Size>>;

  std::bitset<Size> content_;

public:

  typedef std::size_t value_type;

  bitset()
  noexcept
  	: content_(0)
  {
  }

  bitset(const bitset& b)
  noexcept
 	 : content_(b.content_)
  {
  }
  
  bitset(const std::bitset<Size>& std_b)
  noexcept
 	 : content_(std_b)
  {
  }

  bitset(std::initializer_list<std::size_t> values)
  noexcept
    : content_(0)
  {
    for (auto v : values)
    {
      insert(v);
    }
  }

  bool
  operator==(const bitset& other)
  const noexcept
  {
    return content_ == other.content_;
  }

  bool
  operator!=(const bitset& other)
  const noexcept
  {
    return content_ != other.content_;
  }

  bool
  operator<(const bitset& other)
  const noexcept
  {
    return content_.to_ulong() < other.content_.to_ulong();
  }

  bitset&
  insert(std::size_t n)
  {
    content_.set(n);
    return *this;
  }

  std::size_t
  size()
  const noexcept
  {
    return content_.count();
  }

  bool
  empty()
  const noexcept
  {
    return content_.none();
  }

  friend void
  swap(bitset& lhs, bitset& rhs)
  noexcept
  {
    using std::swap;
    swap(lhs.content_, rhs.content_);
  }

  bitset
  operator<<(std::size_t n)
  const noexcept
  {
    return content_ << n;
  }
  
  bitset
  operator>>(std::size_t n)
  const noexcept
  {
    return content_ >> n;
  }

  std::bitset<Size>
  content()
  const noexcept
  {
    return content_;
  }
};

/*-------------------------------------------------------------------------------------------*/

template <std::size_t Size>
inline
bitset<Size>
sum(const bitset<Size>& lhs, const bitset<Size>& rhs)
noexcept
{
  return bitset<Size>(lhs.content() | rhs.content());
}

/*-------------------------------------------------------------------------------------------*/

template <std::size_t Size>
inline
bitset<Size>
intersection(const bitset<Size>& lhs, const bitset<Size>& rhs)
noexcept
{
  return bitset<Size>(lhs.content() & rhs.content());
}

/*-------------------------------------------------------------------------------------------*/

template <std::size_t Size>
inline
bitset<Size>
difference(const bitset<Size>& lhs, const bitset<Size>& rhs)
noexcept
{
  return bitset<Size>(lhs.content() & ~rhs.content());
}

/*-------------------------------------------------------------------------------------------*/

template <std::size_t Size>
std::ostream&
operator<<(std::ostream& os , const bitset<Size>& b)
{
  os << "{";
  bool first = true;
  for (std::size_t i = 0; i < Size; ++i)
  {
    if (b.content().test(i))
    {
      if (not first)
      {
        os << ",";
      }
      else
      {
        first = false;
      }
      os << i;
    }
  }
  return os << "}";
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::values

namespace std {

/*-------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::values::bitset.
template <std::size_t Size>
struct hash<sdd::values::bitset<Size>>
{
  std::size_t
  operator()(const sdd::values::bitset<Size>& b)
  const noexcept
  {
    return std::hash<unsigned long>()(b.content_.to_ulong());
  }
};
  
/*-------------------------------------------------------------------------------------------*/
  
} // namespace std

#endif // _SDD_VALUES_BISET_HH_
