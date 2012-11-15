#ifndef _SDD_COMMON_INDUCTIVES_HH_
#define _SDD_COMMON_INDUCTIVES_HH_

#include <functional> // hash

#include "tests/hom/common.hh"

/*------------------------------------------------------------------------------------------------*/

struct targeted_incr
{
  const unsigned char var_;
  const unsigned int value_;

  targeted_incr(unsigned char var, unsigned int val);

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
  operator==(const targeted_incr& other)
  const noexcept;
};

std::ostream&
operator<<(std::ostream&, const targeted_incr&);

/*------------------------------------------------------------------------------------------------*/

struct incr
{
  const unsigned int value_;

  incr(unsigned int val);

  bool
  skip(unsigned char var)
  const noexcept;

  bool
  selector()
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

std::ostream&
operator<<(std::ostream&, const incr&);

/*------------------------------------------------------------------------------------------------*/

struct targeted_noop
{
  const unsigned char var_;

  targeted_noop(unsigned char v);

  bool
  skip(unsigned char var)
  const noexcept;

  bool
  selector()
  const noexcept;

  hom
  operator()(unsigned char var, const SDD& val)
  const;

  hom
  operator()(unsigned char var, const bitset& val)
  const;

  SDD
  operator()()
  const noexcept;

  bool
  operator==(const targeted_noop& other)
  const noexcept;
};

std::ostream&
operator<<(std::ostream&, const targeted_noop&);

/*------------------------------------------------------------------------------------------------*/

namespace std {

template <>
struct hash<targeted_incr>
{
  std::size_t
  operator()(const targeted_incr&)
  const noexcept;
};

template <>
struct hash<incr>
{
  std::size_t
  operator()(const incr&)
  const noexcept;
};

template <>
struct hash<targeted_noop>
{
  std::size_t
  operator()(const targeted_noop&)
  const noexcept;
};

} // namespace std

/*------------------------------------------------------------------------------------------------*/

#endif // _SDD_COMMON_INDUCTIVES_HH_
