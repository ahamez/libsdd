#include <algorithm> // shuffle, sort
#include <random>

#include "gtest/gtest.h"

#include "sdd/order/order.hh"

#include "tests/configuration.hh"

/*-------------------------------------------------------------------------------------------*/

template <typename C>
struct order_test
  : public testing::Test
{
  using configuration_type = C;
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(order_test, configurations);
#include "tests/macros.hh"

/*-------------------------------------------------------------------------------------------*/

TYPED_TEST(order_test, builder)
{
  {
    order_builder ob;
    ASSERT_TRUE(ob.empty());
  }
  {
    order_builder ob {"a"};
    ASSERT_EQ("a", ob.identifier().user());
    ASSERT_TRUE(ob.nested().empty());
    ASSERT_TRUE(ob.next().empty());
  }
  {
    order_builder ob {"a", "b"};
    ASSERT_EQ("a", ob.identifier().user());
    ASSERT_TRUE(ob.nested().empty());
    ASSERT_FALSE(ob.next().empty());
    ASSERT_EQ("b", ob.next().identifier().user());
    ASSERT_TRUE(ob.next().next().empty());
  }
  {
    order_builder ob;
    ob.push("b").push("a");
    ASSERT_EQ("a", ob.identifier().user());
    ASSERT_TRUE(ob.nested().empty());
    ASSERT_FALSE(ob.next().empty());
    ASSERT_EQ("b", ob.next().identifier().user());
    ASSERT_TRUE(ob.next().next().empty());
  }
  {
    order_builder ob;
    ob.push("y", order_builder {"b"}).push("x", order_builder {"a"});

    ASSERT_EQ("x", ob.identifier().user());
    ASSERT_FALSE(ob.nested().empty());
    ASSERT_FALSE(ob.next().empty());
    ASSERT_EQ("a", ob.nested().identifier().user());
    ASSERT_TRUE(ob.nested().nested().empty());
    ASSERT_TRUE(ob.nested().next().empty());

    ASSERT_EQ("y", ob.next().identifier().user());
    ASSERT_FALSE(ob.next().nested().empty());
    ASSERT_TRUE(ob.next().next().empty());
    ASSERT_EQ("b", ob.next().nested().identifier().user());
    ASSERT_TRUE(ob.next().nested().nested().empty());
    ASSERT_TRUE(ob.next().nested().next().empty());
  }
}

/*-------------------------------------------------------------------------------------------*/

TYPED_TEST(order_test, order_node_relative_order)
{
  {
    const order o(order_builder {"a", "b"});
    ASSERT_TRUE(o.node("a") < o.node("b"));
  }
  {
    const order o(order_builder {"a", "b", "c"});
    ASSERT_TRUE(o.node("a") < o.node("b"));
    ASSERT_TRUE(o.node("a") < o.node("c"));
    ASSERT_TRUE(o.node("b") < o.node("c"));
  }
  {
    const order o(order_builder("a", order_builder {"x", "y"}) << order_builder {"b"});
    ASSERT_TRUE(o.node("a") < o.node("x"));
    ASSERT_TRUE(o.node("a") < o.node("y"));
    ASSERT_TRUE(o.node("x") < o.node("y"));
    ASSERT_TRUE(o.node("a") < o.node("b"));
    ASSERT_TRUE(o.node("x") < o.node("b"));
    ASSERT_TRUE(o.node("y") < o.node("b"));
  }
}

/*-------------------------------------------------------------------------------------------*/

TYPED_TEST(order_test, constructed_order)
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
    ASSERT_EQ("0", o.identifier().user());
    ASSERT_EQ("1", o.next().identifier().user());
    ASSERT_EQ("2", o.next().next().identifier().user());
 }
  {
    order o(order_builder().push("y", order_builder {"c"})
                           .push("x", order_builder().push("z", order_builder {"b"}))
                           .push("a"));

    ASSERT_TRUE(o.contains(o.node("y").position(), o.node("c").position()));
    ASSERT_TRUE(o.contains(o.node("x").position(), o.node("z").position()));
    ASSERT_TRUE(o.contains(o.node("x").position(), o.node("b").position()));
    ASSERT_TRUE(o.contains(o.node("z").position(), o.node("b").position()));

    ASSERT_FALSE(o.contains(o.node("y").position(), o.node("b").position()));
    ASSERT_FALSE(o.contains(o.node("x").position(), o.node("c").position()));
    ASSERT_FALSE(o.contains(o.node("x").position(), o.node("y").position()));
    ASSERT_FALSE(o.contains(o.node("x").position(), o.node("x").position()));
    ASSERT_FALSE(o.contains(o.node("y").position(), o.node("a").position()));
    ASSERT_FALSE(o.contains(o.node("a").position(), o.node("a").position()));

    ASSERT_FALSE(o.empty());
    ASSERT_EQ("a", o.identifier().user());
    ASSERT_TRUE(o.nested().empty());

    ASSERT_FALSE(o.next().empty());
    ASSERT_EQ("x", o.next().identifier().user());
    ASSERT_FALSE(o.next().nested().empty());
    ASSERT_EQ("z", o.next().nested().identifier().user());
    ASSERT_TRUE(o.next().nested().next().empty());
    ASSERT_FALSE(o.next().nested().nested().empty());
    ASSERT_EQ("b", o.next().nested().nested().identifier().user());
    ASSERT_TRUE(o.next().nested().nested().next().empty());
    ASSERT_TRUE(o.next().nested().nested().nested().empty());
 }
}

/*-------------------------------------------------------------------------------------------*/
