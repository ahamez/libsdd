#include <algorithm> // shuffle, sort
#include <random>

#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/order/order.hh"

/*-------------------------------------------------------------------------------------------*/

struct order_test
  : public testing::Test
{
  typedef sdd::conf0 conf;
  typedef sdd::order_builder<conf> order_builder;
  typedef sdd::order<conf> order;
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

TEST_F(order_test, order)
{
  {
    order o(order_builder {});
    ASSERT_TRUE(o.empty());
  }
  {
    order o(order_builder {"0","1","2"});
    ASSERT_FALSE(o.empty());
    ASSERT_FALSE(o.next().empty());
    ASSERT_FALSE(o.next().next().empty());
    ASSERT_TRUE(o.next().next().next().empty());
    ASSERT_TRUE(o.nested().empty());
    ASSERT_TRUE(o.next().nested().empty());
    ASSERT_TRUE(o.next().next().nested().empty());
    ASSERT_EQ("0", o.identifier());
    ASSERT_EQ("1", o.next().identifier());
    ASSERT_EQ("2", o.next().next().identifier());
 }
  {
    order o(order_builder().add("y", order_builder {"c"})
                           .add("x", order_builder().add("z", order_builder {"b"}))
                           .add("a"));

    const std::vector<std::string> ordered_identifiers {"a", "x", "z", "b", "y", "c"};

    std::vector<std::string> identifiers {"a", "b", "c", "x", "y", "z"};
    std::shuffle(identifiers.begin(), identifiers.end(), std::mt19937(std::random_device()()));
    std::sort( identifiers.begin(), identifiers.end()
             , [&o](const std::string& lhs, const std::string& rhs)
                   {
                     return o.compare(lhs, rhs);
                   }
             );

    ASSERT_EQ(ordered_identifiers, identifiers);

    ASSERT_TRUE(o.contains("y", "c"));
    ASSERT_TRUE(o.contains("x", "z"));
    ASSERT_TRUE(o.contains("x", "b"));
    ASSERT_TRUE(o.contains("z", "b"));

    ASSERT_FALSE(o.contains("y", "b"));
    ASSERT_FALSE(o.contains("x", "c"));
    ASSERT_FALSE(o.contains("x", "y"));
    ASSERT_FALSE(o.contains("x", "x"));
    ASSERT_FALSE(o.contains("a", "y"));
    ASSERT_FALSE(o.contains("a", "a"));

    ASSERT_FALSE(o.empty());
    ASSERT_EQ("a", o.identifier());
    ASSERT_TRUE(o.nested().empty());

    ASSERT_FALSE(o.next().empty());
    ASSERT_EQ("x", o.next().identifier());
    ASSERT_FALSE(o.next().nested().empty());
    ASSERT_EQ("z", o.next().nested().identifier());
    ASSERT_TRUE(o.next().nested().next().empty());
    ASSERT_FALSE(o.next().nested().nested().empty());
    ASSERT_EQ("b", o.next().nested().nested().identifier());
    ASSERT_TRUE(o.next().nested().nested().next().empty());
    ASSERT_TRUE(o.next().nested().nested().nested().empty());
 }
}

/*-------------------------------------------------------------------------------------------*/
