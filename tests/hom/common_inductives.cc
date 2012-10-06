#include "common_inductives.hh"

/*-------------------------------------------------------------------------------------------*/

const SDD one = sdd::one<conf>();
const hom id = sdd::hom::Id<conf>();

/*-------------------------------------------------------------------------------------------*/

incr::incr(unsigned char var, unsigned int val)
  : var_(var)
  , value_(val)
{
}

bool
incr::skip(unsigned char var)
const noexcept
{
  return var != var_;
}

hom
incr::operator()(unsigned char var, const SDD& x)
const
{
  return Cons(var, x, Inductive<conf>(*this));
}

hom
incr::operator()(unsigned char var, const bitset& val)
const
{
  if (val.content().test(2))
  {
    return Cons(var, val, id);
  }
  else
  {
    return Cons(var, val << value_, id);
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
  return var_ == other.var_ and value_ == other.value_;
}

std::ostream&
operator<<(std::ostream& os, const incr& i)
{
  return os << "localInductive(" << static_cast<unsigned int>(i.var_) << ", " << i.value_ << ")";
}

/*-------------------------------------------------------------------------------------------*/

namespace std {

std::size_t
hash<incr>::operator()(const incr& i)
const noexcept
{
  return std::hash<unsigned char>()(i.var_) xor std::hash<unsigned char>()(i.value_);
}

} // namespace std

/*-------------------------------------------------------------------------------------------*/
