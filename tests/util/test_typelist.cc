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
  ASSERT_TRUE((is_same<foo, nth<0, foo, bar, baz>::type>::value));
  ASSERT_TRUE((is_same<bar, nth<1, foo, bar, baz>::type>::value));
  ASSERT_TRUE((is_same<baz, nth<2, foo, bar, baz>::type>::value));
  ASSERT_TRUE((is_same<int, nth<2, foo, bar, int>::type>::value));
}

/*------------------------------------------------------------------------------------------------*/

TEST(typelist_test, index_of_nth)
{
  ASSERT_TRUE((is_same<foo, nth<index_of<foo, foo, bar, baz>::value, foo, bar, baz>::type>::value));
  ASSERT_TRUE((is_same<bar, nth<index_of<bar, foo, bar, baz>::value, foo, bar, baz>::type>::value));
  ASSERT_TRUE((is_same<baz, nth<index_of<baz, foo, bar, baz>::value, foo, bar, baz>::type>::value));
  ASSERT_TRUE((is_same<int, nth<index_of<int, foo, bar, int>::value, foo, bar, int>::type>::value));
}

/*------------------------------------------------------------------------------------------------*/
