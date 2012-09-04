#include "gtest/gtest.h"

#include <boost/intrusive/unordered_set.hpp>

#include "sdd/internal/mem/unique_table.hh"

/*-------------------------------------------------------------------------------------------*/

struct foo
{
  typedef boost::intrusive::link_mode<boost::intrusive::normal_link> link_mode;
  boost::intrusive::unordered_set_member_hook<link_mode> member_hook_;
  int i_;

  foo(int i) : i_(i) {}

  bool
  operator==(const foo& other)
  const noexcept
  {
    return i_ == other.i_;
  }
};

namespace std {

template <>
struct hash<foo>
{
  std::size_t
  operator()(const foo& f)
  const noexcept
  {
    return std::hash<int>()(f.i_);
  }
};

}

/*-------------------------------------------------------------------------------------------*/

TEST(unique_table_test, insertion)
{
  {
    sdd::internal::mem::unique_table<foo> ut;

    foo* i1_ptr = new foo(42);
    const foo& i1 = ut(i1_ptr);

    foo* i2_ptr = new foo(42);
    const foo& i2 = ut(i2_ptr);

    ASSERT_EQ(&i1, &i2);
  }
  {
    sdd::internal::mem::unique_table<foo> ut;

    foo* i1_ptr = new foo(42);
    const foo& i1 = ut(i1_ptr);

    foo* i2_ptr = new foo(43);
    const foo& i2 = ut(i2_ptr);

    ASSERT_NE(&i1, &i2);
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST(unique_table_test, rehash)
{
  sdd::internal::mem::unique_table<foo> ut(1);

  foo* f1_ptr = new foo(0);
  const foo& f1 = ut(f1_ptr);

  // insert enough data to force at least one rehash
  for (int i = 1; i < 10000; ++i)
  {
    ut(new foo(i));
  }

  foo* f2_ptr = new foo(0);
  const foo& f2 = ut(f2_ptr);

  // ensure that addresses didn't move (ok, just for one...)
  ASSERT_EQ(&f1, &f2);
}

/*-------------------------------------------------------------------------------------------*/