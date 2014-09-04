#include <type_traits> // is_same

#include "gtest/gtest.h"

#include "sdd/util/typelist.hh"

/*------------------------------------------------------------------------------------------------*/

struct foo {};
struct bar {};
struct baz {};

/*------------------------------------------------------------------------------------------------*/

using std::is_same;
using namespace sdd::util;

/*------------------------------------------------------------------------------------------------*/

TEST(typelist_test, index_of)
{
  std::size_t value;
  {
    value = sdd::util::index_of<foo, foo, bar, baz>::value;
    ASSERT_EQ(value, 0u);
    value = sdd::util::index_of<bar, foo, bar, baz>::value;
    ASSERT_EQ(value, 1u);
    value = sdd::util::index_of<baz, foo, bar, baz>::value;
    ASSERT_EQ(value, 2u);
  }
  {
    value = sdd::util::index_of<int, foo, int, baz>::value;
    ASSERT_EQ(value, 1u);
    value = sdd::util::index_of<int, char, int, baz>::value;
    ASSERT_EQ(value, 1u);
    value = sdd::util::index_of<int, int, int, baz>::value;
    ASSERT_EQ(value, 0u);
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST(typelist_test, nth)
{
  ASSERT_TRUE((is_same<foo, nth_t<0, foo, bar, baz>>::value));
  ASSERT_TRUE((is_same<bar, nth_t<1, foo, bar, baz>>::value));
  ASSERT_TRUE((is_same<baz, nth_t<2, foo, bar, baz>>::value));
  ASSERT_TRUE((is_same<int, nth_t<2, foo, bar, int>>::value));
}

/*------------------------------------------------------------------------------------------------*/

TEST(typelist_test, index_of_nth)
{
  ASSERT_TRUE((is_same<foo, nth_t<index_of<foo, foo, bar, baz>::value, foo, bar, baz>>::value));
  ASSERT_TRUE((is_same<bar, nth_t<index_of<bar, foo, bar, baz>::value, foo, bar, baz>>::value));
  ASSERT_TRUE((is_same<baz, nth_t<index_of<baz, foo, bar, baz>::value, foo, bar, baz>>::value));
  ASSERT_TRUE((is_same<int, nth_t<index_of<int, foo, bar, int>::value, foo, bar, int>>::value));
}

/*------------------------------------------------------------------------------------------------*/
