#include "gtest/gtest.h"

#include "tests/hom/common.hh"
#include "sdd/sdd.hh"

/*-------------------------------------------------------------------------------------------*/

const SDD one = sdd::one<conf>();

struct order_utility_test
  : public testing::Test
{
};

/*-------------------------------------------------------------------------------------------*/

struct initializer
{
  sdd::conf0::Values
  operator()(const std::string&)
  const
  {
    return {0};
  }
};

/*-------------------------------------------------------------------------------------------*/

TEST_F(order_utility_test, empty)
{
  auto o = order(order_builder());
  ASSERT_EQ(one, SDD(o, initializer()));
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(order_utility_test, flat)
{
  {
    order_builder ob;
    ob.add("foo");
    ASSERT_EQ(SDD(0, {0}, one), SDD(order(ob), initializer()));
  }
  {
    order_builder ob;
    ob.add("foo", order_builder());
    ASSERT_EQ(SDD(0, {0}, one), SDD(order(ob), initializer()));
  }
  {
    order_builder ob;
    ob.add("foo1").add("foo2");
    ASSERT_EQ(SDD(1, {0}, SDD(0, {0}, one)), SDD(order(ob), initializer()));
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(order_utility_test, hierarchical)
{
  {
    order_builder ob0;
    ob0.add("foo");
    order_builder ob1;
    ob1.add("bar", ob0);
    ASSERT_EQ(SDD(0, SDD(0, {0}, one), one), SDD(order(ob1), initializer()));
  }
  {
    order_builder ob0;
    ob0.add("foo1").add("foo2");
    order_builder ob1;
    ob1.add("bar", ob0);
    ASSERT_EQ( SDD(0, SDD(1, {0}, SDD(0, {0}, one)), one)
             , SDD(order(ob1), initializer()));
  }
  {
    order_builder nested_a {"a"};
    order_builder nested_b {"b"};
    order_builder ob;
    ob.add("y", nested_b);
    ob.add("x", nested_a);
    ASSERT_EQ( SDD(1, SDD(0, {0}, one), SDD(0, SDD(0, {0}, one), one))
             , SDD(order(ob), initializer()));
  }
}

/*-------------------------------------------------------------------------------------------*/
