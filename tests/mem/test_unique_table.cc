#include "gtest/gtest.h"

#include <boost/intrusive/unordered_set.hpp>

#include "sdd/mem/unique_table.hh"

/*------------------------------------------------------------------------------------------------*/

namespace {

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

  std::size_t
  extra_bytes()
  const noexcept
  {
    return 0;
  }

  // Needed by an assertion in unique_table.
  bool
  is_not_referenced()
  const noexcept
  {
    return true;
  }
};

}

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

/*------------------------------------------------------------------------------------------------*/

TEST(unique_table_test, insertion)
{
  {
    sdd::mem::unique_table<foo> ut(100);

    char* addr1 = ut.allocate(0);
    foo* i1_ptr = new (addr1) foo(42);
    const foo& i1 = ut(i1_ptr);

    char* addr2 = ut.allocate(0);
    foo* i2_ptr = new (addr2) foo(42);
    const foo& i2 = ut(i2_ptr);

    ASSERT_EQ(&i1, &i2);
    ut.erase(const_cast<foo&>(i1));
  }
  {
    sdd::mem::unique_table<foo> ut(100);

    char* addr1 = ut.allocate(0);
    foo* i1_ptr = new (addr1) foo(42);
    const foo& i1 = ut(i1_ptr);

    char* addr2 = ut.allocate(0);
    foo* i2_ptr = new (addr2) foo(43);
    const foo& i2 = ut(i2_ptr);

    ASSERT_NE(&i1, &i2);
    ut.erase(const_cast<foo&>(i1));
    ut.erase(const_cast<foo&>(i2));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST(unique_table_test, rehash)
{
  sdd::mem::unique_table<foo> ut(1);

  char* addr1 = ut.allocate(0);
  foo* f1_ptr = new (addr1) foo(0);
  const foo& f1 = ut(f1_ptr);

  std::vector<const foo*> ptrs;
  ptrs.reserve(10000);
  // insert enough data to force at least one rehash
  for (int i = 1; i < 10000; ++i)
  {
    char* addr = ut.allocate(0);
    ptrs.push_back(&ut(new (addr) foo(i)));
  }

  char* addr2 = ut.allocate(0);
  foo* f2_ptr = new (addr2) foo(0);
  const foo& f2 = ut(f2_ptr);

  // ensure that addresses didn't move (ok, just for one...)
  ASSERT_EQ(&f1, &f2);

  ut.erase(const_cast<foo&>(f1));

  for (auto p : ptrs)
  {
    ut.erase(*const_cast<foo*>(p));
  }
}

/*------------------------------------------------------------------------------------------------*/