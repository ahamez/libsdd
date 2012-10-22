#include "gtest/gtest.h"

#include "tests/hom/common.hh"
#include "sdd/order/utility.hh"

/*-------------------------------------------------------------------------------------------*/

struct order_test
  : public testing::Test
{
};

/*-------------------------------------------------------------------------------------------*/

TEST_F(order_test, order_builder)
{
  {
    order_builder ob;
    ASSERT_TRUE(ob.empty());
  }
  {
    order_builder ob {"a"};
    ASSERT_EQ("a", ob.identifier());
    ASSERT_TRUE(ob.nested().empty());
    ASSERT_TRUE(ob.next().empty());
  }
  {
    order_builder ob {"a", "b"};
    ASSERT_EQ("a", ob.identifier());
    ASSERT_TRUE(ob.nested().empty());
    ASSERT_FALSE(ob.next().empty());
    ASSERT_EQ("b", ob.next().identifier());
    ASSERT_TRUE(ob.next().next().empty());
  }
  {
    order_builder ob;
    ob.add("b").add("a");
    ASSERT_EQ("a", ob.identifier());
    ASSERT_TRUE(ob.nested().empty());
    ASSERT_FALSE(ob.next().empty());
    ASSERT_EQ("b", ob.next().identifier());
    ASSERT_TRUE(ob.next().next().empty());
  }
  {
    order_builder ob;
    ob.add("y", order_builder {"b"}).add("x", order_builder {"a"});

    ASSERT_EQ("x", ob.identifier());
    ASSERT_FALSE(ob.nested().empty());
    ASSERT_FALSE(ob.next().empty());
    ASSERT_EQ("a", ob.nested().identifier());
    ASSERT_TRUE(ob.nested().nested().empty());
    ASSERT_TRUE(ob.nested().next().empty());

    ASSERT_EQ("y", ob.next().identifier());
    ASSERT_FALSE(ob.next().nested().empty());
    ASSERT_TRUE(ob.next().next().empty());
    ASSERT_EQ("b", ob.next().nested().identifier());
    ASSERT_TRUE(ob.next().nested().nested().empty());
    ASSERT_TRUE(ob.next().nested().next().empty());
  }
}

/*-------------------------------------------------------------------------------------------*/
