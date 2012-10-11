#ifndef _SDD_COMMON_INDUCTIVES_HH_
#define _SDD_COMMON_INDUCTIVES_HH_

#include <functional> // hash
#include <string>

#include "tests/hom/common.hh"

/*-------------------------------------------------------------------------------------------*/

struct targeted_incr
{
  const std::string var_;
  const unsigned int value_;

  targeted_incr(const std::string&, unsigned int);

  bool
  skip(const std::string&)
  const noexcept;

  bool
  selector()
  const noexcept;

  hom
  operator()(const order<conf>&, const SDD& x)
  const;

  hom
  operator()(const order<conf>&, const bitset& val)
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

/*-------------------------------------------------------------------------------------------*/

struct incr
{
  const unsigned int value_;

  incr(unsigned int);

  bool
  skip(const std::string&)
  const noexcept;

  bool
  selector()
  const noexcept;

  hom
  operator()(const order<conf>&, const SDD& x)
  const;

  hom
  operator()(const order<conf>&, const bitset& val)
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

/*-------------------------------------------------------------------------------------------*/

struct targeted_noop
{
  const std::string var_;

  targeted_noop(const std::string&);

  bool
  skip(const std::string&)
  const noexcept;

  bool
  selector()
  const noexcept;

  hom
  operator()(const order<conf>&, const SDD& val)
  const;

  hom
  operator()(const order<conf>&, const bitset& val)
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

/*-------------------------------------------------------------------------------------------*/

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

/*-------------------------------------------------------------------------------------------*/

#endif // _SDD_COMMON_INDUCTIVES_HH_
