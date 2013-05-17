#include "gtest/gtest.h"

#include "sdd/mem/hash_table.hh"
#include "sdd/mem/unique_table.hh"

/*------------------------------------------------------------------------------------------------*/

namespace {

struct foo
{
//  typedef boost::intrusive::link_mode<boost::intrusive::normal_link> link_mode;
//  boost::intrusive::unordered_set_member_hook<link_mode> member_hook_;
  sdd::mem::intrusive_member_hook<foo> hook;
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