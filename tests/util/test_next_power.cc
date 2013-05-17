#include "gtest/gtest.h"

#include "sdd/util/next_power.hh"

/*------------------------------------------------------------------------------------------------*/

using namespace sdd::util;

/*------------------------------------------------------------------------------------------------*/

TEST(next_power_test, test)
{
  ASSERT_EQ(1u, next_power_of_2(1u));

  ASSERT_EQ(2u, next_power_of_2(2u));

  ASSERT_EQ(4u, next_power_of_2(3u));
  ASSERT_EQ(4u, next_power_of_2(4u));

  ASSERT_EQ(8u, next_power_of_2(5u));
  ASSERT_EQ(8u, next_power_of_2(8u));

  ASSERT_EQ(16u, next_power_of_2(9u));
  ASSERT_EQ(16u, next_power_of_2(16u));

  ASSERT_EQ(32u, next_power_of_2(17u));
  ASSERT_EQ(32u, next_power_of_2(32u));

  ASSERT_EQ(128u, next_power_of_2(65u));
  ASSERT_EQ(128u, next_power_of_2(128u));

  ASSERT_EQ(4096u, next_power_of_2(2149u));
  ASSERT_EQ(4096u, next_power_of_2(4096u));
}

/*------------------------------------------------------------------------------------------------*/
