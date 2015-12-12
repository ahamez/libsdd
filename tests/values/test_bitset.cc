#include "gtest/gtest.h"

#include "sdd/values/bitset.hh"

using bitset = sdd::values::bitset<64>;

/*------------------------------------------------------------------------------------------------*/

TEST(bitset_test, empty)
{
  bitset b;
  ASSERT_TRUE(b.empty());
}

/*------------------------------------------------------------------------------------------------*/

TEST(bitset_test, insertion)
{
  bitset b;
  ASSERT_TRUE(b.empty());
  b.insert(1);
  b.insert(2);
  b.insert(10);
  ASSERT_EQ(bitset({1,2,10}), b);
}

/*------------------------------------------------------------------------------------------------*/

TEST(bitset_test, size)
{
  ASSERT_EQ(0u, bitset({}).size());
  ASSERT_EQ(3u, bitset({1,2,10}).size());
}

/*------------------------------------------------------------------------------------------------*/

TEST(bitset_test, swap)
{
  bitset b1 {0,1,2};
  bitset b2 {0};
  swap(b1, b2);
  ASSERT_EQ(bitset {0}, b1);
  ASSERT_EQ(bitset({0,1,2}), b2);
}

/*------------------------------------------------------------------------------------------------*/

TEST(bitset_test, left_shift)
{
  ASSERT_EQ(bitset({1,2,3}), bitset({0,1,2}).operator<<(1));
}

/*------------------------------------------------------------------------------------------------*/

TEST(bitset_test, right_shift)
{
  ASSERT_EQ(bitset({0,1}), bitset({0,1,2}).operator>>(1));
}

/*------------------------------------------------------------------------------------------------*/

TEST(bitset_test, sum)
{
  ASSERT_EQ(bitset(),      sum(bitset(), bitset()));
  ASSERT_EQ(bitset({0}),   sum(bitset({0}), bitset()));
  ASSERT_EQ(bitset({0}),   sum(bitset({0}), bitset({0})));
  ASSERT_EQ(bitset({0,1}), sum(bitset({0}), bitset({1})));
  ASSERT_EQ(bitset({0,1}), sum(bitset({0}), bitset({0,1})));
}

/*------------------------------------------------------------------------------------------------*/

TEST(bitset_test, intersection)
{
  ASSERT_EQ(bitset(),      intersection(bitset(), bitset()));
  ASSERT_EQ(bitset(),      intersection(bitset({0}), bitset()));
  ASSERT_EQ(bitset({0}),   intersection(bitset({0}), bitset({0})));
  ASSERT_EQ(bitset(),      intersection(bitset({0}), bitset({1})));
  ASSERT_EQ(bitset({0}),   intersection(bitset({0}), bitset({0,1})));
  ASSERT_EQ(bitset({0}),   intersection(bitset({0,2}), bitset({0,1})));
  ASSERT_EQ(bitset({0,1}), intersection(bitset({0,1,2}), bitset({0,1,3})));
}

/*------------------------------------------------------------------------------------------------*/

TEST(bitset_test, difference)
{
  ASSERT_EQ(bitset(),      difference(bitset(), bitset()));
  ASSERT_EQ(bitset({0}),   difference(bitset({0}), bitset()));
  ASSERT_EQ(bitset(),      difference(bitset({0}), bitset({0})));
  ASSERT_EQ(bitset({0}),   difference(bitset({0}), bitset({1})));
  ASSERT_EQ(bitset(),      difference(bitset({0}), bitset({0,1})));
  ASSERT_EQ(bitset({2}),   difference(bitset({0,2}), bitset({0,1})));
  ASSERT_EQ(bitset({2}),   difference(bitset({0,1,2}), bitset({0,1,3})));
}

/*------------------------------------------------------------------------------------------------*/
