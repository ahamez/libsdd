#include "gtest/gtest.h"

#include "sdd/manager.hh"

#include "tests/configuration.hh"

/*-------------------------------------------------------------------------------------------*/

template <typename C>
struct order_utility_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;

  order_utility_test()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
  {}
};

/*-------------------------------------------------------------------------------------------*/

template <typename C>
struct initializer
{
  template <typename T>
  typename C::Values
  operator()(const T&)
  const
  {
    return {0};
  }
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(order_utility_test, configurations);
#include "tests/macros.hh"

/*-------------------------------------------------------------------------------------------*/

TYPED_TEST(order_utility_test, empty)
{
  auto o = order(order_builder());
  ASSERT_EQ(one, SDD(o, initializer<conf>()));
}

/*-------------------------------------------------------------------------------------------*/

TYPED_TEST(order_utility_test, sdd_initializer_flat)
{
  {
    order_builder ob;
    ob.push("foo");
    ASSERT_EQ(SDD(0, {0}, one), SDD(order(ob), initializer<conf>()));
  }
  {
    order_builder ob;
    ob.push("foo", order_builder());
    ASSERT_EQ(SDD(0, {0}, one), SDD(order(ob), initializer<conf>()));
  }
  {
    order_builder ob;
    ob.push("foo1").push("foo2");
    ASSERT_EQ(SDD(1, {0}, SDD(0, {0}, one)), SDD(order(ob), initializer<conf>()));
  }
}

/*-------------------------------------------------------------------------------------------*/

TYPED_TEST(order_utility_test, sdd_initializer_hierarchical)
{
  {
    order_builder ob0;
    ob0.push("foo");
    order_builder ob1;
    ob1.push("bar", ob0);
    ASSERT_EQ(SDD(0, SDD(0, {0}, one), one), SDD(order(ob1), initializer<conf>()));
  }
  {
    order_builder ob0;
    ob0.push("foo1").push("foo2");
    order_builder ob1;
    ob1.push("bar", ob0);
    ASSERT_EQ( SDD(0, SDD(1, {0}, SDD(0, {0}, one)), one)
             , SDD(order(ob1), initializer<conf>()));
  }
  {
    order_builder nested_a {"a"};
    order_builder nested_b {"b"};
    order_builder ob;
    ob.push("y", nested_b);
    ob.push("x", nested_a);
    ASSERT_EQ( SDD(1, SDD(0, {0}, one), SDD(0, SDD(0, {0}, one), one))
             , SDD(order(ob), initializer<conf>()));
  }
}

/*-------------------------------------------------------------------------------------------*/
