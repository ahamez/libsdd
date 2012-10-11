#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/definition.hh"
#include "sdd/order/utility.hh"

/*-------------------------------------------------------------------------------------------*/

struct order_utility_test
  : public testing::Test
{
  typedef sdd::conf::conf0 conf;
  typedef sdd::SDD<conf> SDD;
  const SDD zero = sdd::zero<conf>();
  const SDD one = sdd::one<conf>();

  order_utility_test()
  {
  }
};

using sdd::order::order;

/*-------------------------------------------------------------------------------------------*/

struct initializer
{
  sdd::conf::conf0::Values
  operator()(const std::string&)
  const
  {
    return {0};
  }
};

/*-------------------------------------------------------------------------------------------*/

TEST_F(order_utility_test, empty)
{
  auto o = order<conf>();
  ASSERT_EQ(one, sdd::order::sdd(o, initializer()));
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(order_utility_test, flat)
{
  {
    order<conf> o;
    o.add("foo");
    ASSERT_EQ(SDD(0, {0}, one), sdd::order::sdd(o, initializer()));
  }
  {
    order<conf> o;
    o.add("foo", order<conf>());
    ASSERT_EQ(SDD(0, {0}, one), sdd::order::sdd(o, initializer()));
  }
  {
    order<conf> o;
    o.add("foo1").add("foo2");
    ASSERT_EQ(SDD(1, {0}, SDD(0, {0}, one)), sdd::order::sdd(o, initializer()));
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(order_utility_test, hierarchical)
{
  {
    order<conf> o;
    o.add("foo");
    order<conf> p;
    p.add("bar", o);
    ASSERT_EQ(SDD(0, SDD(0, {0}, one), one), sdd::order::sdd(p, initializer()));
  }
  {
    order<conf> o;
    o.add("foo1").add("foo2");
    order<conf> p;
    p.add("bar", o);
    ASSERT_EQ(SDD(0, SDD(1, {0}, SDD(0, {0}, one)), one), sdd::order::sdd(p, initializer()));
  }
  {
    order<conf> o;
    o.add("foo");
    order<conf> p;
    p.add("bar1").add("bar2");
    ASSERT_EQ( SDD(1, SDD(0, {0}, one), SDD(0, SDD(0, {0}, one), one))
             , sdd::order::sdd(p, initializer()));
  }
}

/*-------------------------------------------------------------------------------------------*/
