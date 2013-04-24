#ifndef _SDD_COMMON_INDUCTIVES_HH_
#define _SDD_COMMON_INDUCTIVES_HH_

#include <functional> // hash
#include <string>

#include "tests/hom/common.hh"

/*------------------------------------------------------------------------------------------------*/

namespace /* anonymous */{

struct targeted_incr
{
  const std::string var_;
  const unsigned int value_;

  targeted_incr(const std::string& var, unsigned int val)
    : var_(var)
    , value_(val)
  {
  }

  bool
  skip(const std::string& var)
  const noexcept
  {
    return var != var_;
  }


  hom
  operator()(const order& o, const SDD& x)
  const
  {
    return Cons(o, x, Inductive<conf>(*this));
  }

  hom
  operator()(const order& o, const bitset& val)
  const
  {
    if (val.content().test(2))
    {
      return Cons(o, val, sdd::Id<conf>());
    }
    else
    {
      return Cons(o, val << value_, sdd::Id<conf>());
    }
  }

  SDD
  operator()()
  const noexcept
  {
    return sdd::one<conf>();
  }

  bool
  operator==(const targeted_incr& other)
  const noexcept
  {
    return var_ == other.var_ and value_ == other.value_;
  }
};

std::ostream&
operator<<(std::ostream& os, const targeted_incr& i)
{
  return os << "target_incr(" << i.var_ << ", " << i.value_ << ")";
}

/*------------------------------------------------------------------------------------------------*/

struct incr
{
  const unsigned int value_;

  incr(unsigned int val)
    : value_(val)
  {
  }

  bool
  skip(const std::string&)
  const noexcept
  {
    return false;
  }

  bool
  selector()
  const noexcept
  {
    return false;
  }

  hom
  operator()(const order& o, const SDD& x)
  const
  {
    return Cons(o, x, Inductive<conf>(*this));
  }

  hom
  operator()(const order& o, const bitset& val)
  const
  {
    if (val.content().test(2))
    {
      return Cons(o, val, sdd::Id<conf>());
    }
    else
    {
      return Cons(o, val << value_, sdd::Id<conf>());
    }
  }

  SDD
  operator()()
  const noexcept
  {
    return sdd::one<conf>();
  }

  bool
  operator==(const incr& other)
  const noexcept
  {
    return value_ == other.value_;
  }
};

std::ostream&
operator<<(std::ostream& os, const incr& i)
{
  return os << "incr(" << i.value_ << ")";
}

/*------------------------------------------------------------------------------------------------*/

struct targeted_noop
{
  const std::string var_;

  targeted_noop(const std::string& v)
    : var_(v)
  {
  }

  bool
  skip(const std::string& var)
  const noexcept
  {
    return var != var_;
  }

  bool
  selector()
  const noexcept
  {
    return true;
  }

  hom
  operator()(const order& o, const SDD& val)
  const
  {
    return Cons(o, val, sdd::Id<conf>());
  }

  hom
  operator()(const order& o, const bitset& val)
  const
  {
    return Cons(o, val, sdd::Id<conf>());
  }

  SDD
  operator()()
  const noexcept
  {
    return sdd::one<conf>();
  }

  bool
  operator==(const targeted_noop& other)
  const noexcept
  {
    return var_ == other.var_;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace anonymous

namespace std {

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<targeted_incr>
{
  std::size_t
  operator()(const targeted_incr& i)
  const noexcept
  {
    return std::hash<std::string>()(i.var_) xor std::hash<unsigned int>()(i.value_);
  }
};

template <>
struct hash<incr>
{
  std::size_t
  operator()(const incr& i)
  const noexcept
  {
    return std::hash<unsigned int>()(i.value_);
  }
};

template <>
struct hash<targeted_noop>
{
  std::size_t
  operator()(const targeted_noop& i)
  const noexcept
  {
    return std::hash<std::string>()(i.var_);
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std


#endif // _SDD_COMMON_INDUCTIVES_HH_
