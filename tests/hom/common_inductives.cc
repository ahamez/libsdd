#include "common_inductives.hh"

/*-------------------------------------------------------------------------------------------*/

const SDD one = sdd::one<conf>();
const hom id = sdd::hom::Id<conf>();

/*-------------------------------------------------------------------------------------------*/

targeted_incr::targeted_incr(const std::string& var, unsigned int val)
  : var_(var)
  , value_(val)
{
}

bool
targeted_incr::skip(const std::string& var)
const noexcept
{
  return var != var_;
}

hom
targeted_incr::operator()(const order<conf>& o, const SDD& x)
const
{
  return Cons(o.identifier(), o, x, Inductive<conf>(*this));
}

hom
targeted_incr::operator()(const order<conf>& o, const bitset& val)
const
{
  if (val.content().test(2))
  {
    return Cons(o.identifier(), o, val, id);
  }
  else
  {
    return Cons(o.identifier(), o, val << value_, id);
  }
}

SDD
targeted_incr::operator()()
const noexcept
{
  return one;
}

bool
targeted_incr::operator==(const targeted_incr& other)
const noexcept
{
  return var_ == other.var_ and value_ == other.value_;
}

std::ostream&
operator<<(std::ostream& os, const targeted_incr& i)
{
  return os << "target_incr(" << i.var_ << ", " << i.value_ << ")";
}

/*-------------------------------------------------------------------------------------------*/

incr::incr(unsigned int val)
  : value_(val)
{
}

bool
incr::skip(const std::string&)
const noexcept
{
  return false;
}

bool
incr::selector()
const noexcept
{
  return false;
}

hom
incr::operator()(const order<conf>& o, const SDD& x)
const
{
  return Cons(o.identifier(), o, x, Inductive<conf>(*this));
}

hom
incr::operator()(const order<conf>& o, const bitset& val)
const
{
  if (val.content().test(2))
  {
    return Cons(o.identifier(), o, val, id);
  }
  else
  {
    return Cons(o.identifier(), o, val << value_, id);
  }
}

SDD
incr::operator()()
const noexcept
{
  return one;
}

bool
incr::operator==(const incr& other)
const noexcept
{
  return value_ == other.value_;
}

std::ostream&
operator<<(std::ostream& os, const incr& i)
{
  return os << "incr(" << i.value_ << ")";
}

/*-------------------------------------------------------------------------------------------*/

targeted_noop::targeted_noop(const std::string& v)
  : var_(v)
{
}

bool
targeted_noop::skip(const std::string& var)
const noexcept
{
  return var != var_;
}

bool
targeted_noop::selector()
const noexcept
{
  return true;
}

hom
targeted_noop::operator()(const order<conf>& o, const SDD& val)
const
{
  return Cons(o.identifier(), o, val, id);
}

hom
targeted_noop::operator()(const order<conf>& o, const bitset& val)
const
{
  return Cons(o.identifier(), o, val, id);
}

SDD
targeted_noop::operator()()
const noexcept
{
  return one;
}

bool
targeted_noop::operator==(const targeted_noop& other)
const noexcept
{
  return var_ == other.var_;
}

std::ostream&
operator<<(std::ostream& os, const targeted_noop& i)
{
  return os << "targeted_noop(" << i.var_ << ")";
}

/*-------------------------------------------------------------------------------------------*/

namespace std {

std::size_t
hash<targeted_incr>::operator()(const targeted_incr& i)
const noexcept
{
  return std::hash<std::string>()(i.var_) xor std::hash<unsigned int>()(i.value_);
}

std::size_t
hash<incr>::operator()(const incr& i)
const noexcept
{
  return std::hash<unsigned int>()(i.value_);
}

std::size_t
hash<targeted_noop>::operator()(const targeted_noop& i)
const noexcept
{
  return std::hash<std::string>()(i.var_);
}


} // namespace std

/*-------------------------------------------------------------------------------------------*/
