#ifndef _SDD_COMMON_INDUCTIVES_HH_
#define _SDD_COMMON_INDUCTIVES_HH_

#include <functional> // hash

#include "tests/hom/common.hh"

/*-------------------------------------------------------------------------------------------*/

struct incr
{
  const unsigned char var_;
  const unsigned int value_;

  incr(unsigned char var, unsigned int val);

  bool
  skip(unsigned char var)
  const noexcept;

  hom
  operator()(unsigned char var, const SDD& x)
  const;

  hom
  operator()(unsigned char var, const bitset& val)
  const;

  SDD
  operator()()
  const noexcept;

  bool
  operator==(const incr& other)
  const noexcept;
};

/*-------------------------------------------------------------------------------------------*/

std::ostream&
operator<<(std::ostream& os, const incr& i);

/*-------------------------------------------------------------------------------------------*/

namespace std {

template <>
struct hash<incr>
{
  std::size_t
  operator()(const incr& )
  const noexcept;
};

} // namespace std

/*-------------------------------------------------------------------------------------------*/

#endif // _SDD_COMMON_INDUCTIVES_HH_
