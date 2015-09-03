#ifndef _SDD_COMMON_INDUCTIVES_HH_
#define _SDD_COMMON_INDUCTIVES_HH_

#include <functional> // hash
#include <string>

#include "sdd/dd/definition.hh"
#include "sdd/hom/definition.hh"
#include "sdd/values/bitset.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct targeted_incr
{
  using values_type = typename C::Values;

  const std::string var_;
  const unsigned int value_;

  targeted_incr(std::string var, unsigned int val)
    : var_{std::move(var)}
    , value_{val}
  {}

  bool
  skip(const std::string& var)
  const noexcept
  {
    return var != var_;
  }

  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const sdd::SDD<C>& x)
  const
  {
    return sdd::cons(o, x, sdd::inductive<C>(*this));
  }

  template <typename T>
  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const T& val)
  const
  {
    if (val.find(2) != val.end())
    {
      return cons(o, val, sdd::id<C>());
    }
    else
    {
      T new_val;
      for (const auto& v : val)
      {
        new_val.insert(v + value_);
      }
      return cons(o, new_val, sdd::id<C>());
    }
  }

  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const sdd::values::bitset<64>& val)
  const
  {
    if (val.content().test(2))
    {
      return cons(o, val, sdd::id<C>());
    }
    else
    {
      return cons(o, val << value_, sdd::id<C>());
    }
  }

  sdd::SDD<C>
  operator()()
  const noexcept
  {
    return sdd::one<C>();
  }

  bool
  operator==(const targeted_incr& other)
  const noexcept
  {
    return var_ == other.var_ and value_ == other.value_;
  }
};

template <typename C>
std::ostream&
operator<<(std::ostream& os, const targeted_incr<C>& i)
{
  return os << "target_incr(" << i.var_ << ", " << i.value_ << ")";
}

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct incr
{
  using values_type = typename C::Values;
  
  const unsigned int value_;

  incr(unsigned int val)
    : value_(val)
  {}

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

  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const sdd::SDD<C>& x)
  const
  {
    return cons(o, x, inductive<C>(*this));
  }

  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const values_type& val)
  const
  {
    if (val.content().test(2))
    {
      return cons(o, val, sdd::id<C>());
    }
    else
    {
      return cons(o, val << value_, sdd::id<C>());
    }
  }

  sdd::SDD<C>
  operator()()
  const noexcept
  {
    return sdd::one<C>();
  }

  bool
  operator==(const incr& other)
  const noexcept
  {
    return value_ == other.value_;
  }
};

template <typename C>
std::ostream&
operator<<(std::ostream& os, const incr<C>& i)
{
  return os << "incr(" << i.value_ << ")";
}

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct targeted_noop
{
  using values_type = typename C::Values;

  const std::string var_;

  targeted_noop(std::string v)
    : var_{std::move(v)}
  {}

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

  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const sdd::SDD<C>& val)
  const
  {
    return cons(o, val, sdd::id<C>());
  }

  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const values_type& val)
  const
  {
    return cons(o, val, sdd::id<C>());
  }

  sdd::SDD<C>
  operator()()
  const noexcept
  {
    return sdd::one<C>();
  }

  bool
  operator==(const targeted_noop& other)
  const noexcept
  {
    return var_ == other.var_;
  }
};

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct ind
{
  const std::string var_;

  ind(std::string v)
    : var_{std::move(v)}
  {}

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
    return false;
  }

  sdd::homomorphism<C>
  operator()(const sdd::order<C>&, const sdd::SDD<C>&)
  const
  {
    return sdd::id<C>();
  }

  sdd::homomorphism<C>
  operator()(const sdd::order<C>&, const typename C::Values&)
  const
  {
    return sdd::id<C>();
  }

  sdd::SDD<C>
  operator()()
  const noexcept
  {
    return sdd::one<C>();
  }

  bool
  operator==(const ind& other)
  const noexcept
  {
    return var_ == other.var_;
  }
};

template <typename C>
std::ostream&
operator<<(std::ostream& os, const ind<C>& i)
{
  return os << "ind(" << i.var_ << ")";
}

/*------------------------------------------------------------------------------------------------*/

namespace std {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct hash<targeted_incr<C>>
{
  std::size_t
  operator()(const targeted_incr<C>& i)
  const noexcept
  {
    return std::hash<std::string>()(i.var_) xor std::hash<unsigned int>()(i.value_);
  }
};

template <typename C>
struct hash<incr<C>>
{
  std::size_t
  operator()(const incr<C>& i)
  const noexcept
  {
    return std::hash<unsigned int>()(i.value_);
  }
};

template <typename C>
struct hash<targeted_noop<C>>
{
  std::size_t
  operator()(const targeted_noop<C>& i)
  const noexcept
  {
    return std::hash<std::string>()(i.var_);
  }
};

template <typename C>
struct hash<ind<C>>
{
  std::size_t
  operator()(const ind<C>& i)
  const noexcept
  {
    return std::hash<std::string>()(i.var_);
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_COMMON_INDUCTIVES_HH_
