#include "gtest/gtest.h"

#include "sdd/sdd.hh"
#include "sdd/conf/default_configurations.hh"

/*-------------------------------------------------------------------------------------------*/

struct order_utility_test
  : public testing::Test
{
  typedef sdd::conf::conf0 conf;
  typedef sdd::dd::SDD<conf> SDD;
  const SDD zero = sdd::dd::zero<conf>();
  const SDD one = sdd::dd::one<conf>();

  order_utility_test()
  {
  }
};

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
  auto o = sdd::order::empty_order<conf>();
  ASSERT_EQ(one, sdd::order::sdd(o, initializer()));
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(order_utility_test, flat)
{
  {
    auto o = sdd::order::empty_order<conf>();
    o = add_identifier("foo", o);
    ASSERT_EQ(SDD(0, {0}, one), sdd::order::sdd(o, initializer()));
  }
  {
    auto o = sdd::order::empty_order<conf>();
    auto p = sdd::order::empty_order<conf>();
    p = add_identifier("foo", p, o);
    ASSERT_EQ(SDD(0, {0}, one), sdd::order::sdd(p, initializer()));
  }
  {
    auto o = sdd::order::empty_order<conf>();
    o = add_identifier("foo1", o);
    o = add_identifier("foo2", o);
    ASSERT_EQ(SDD(1, {0}, SDD(0, {0}, one)), sdd::order::sdd(o, initializer()));
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(order_utility_test, hierarchical)
{
  {
    auto o = sdd::order::empty_order<conf>();
    o = add_identifier("foo", o);
    auto p = sdd::order::empty_order<conf>();
    p = add_identifier("bar", p, o);
    ASSERT_EQ(SDD(0, SDD(0, {0}, one), one), sdd::order::sdd(p, initializer()));
  }
  {
    auto o = sdd::order::empty_order<conf>();
    o = add_identifier("foo1", o);
    o = add_identifier("foo2", o);
    auto p = sdd::order::empty_order<conf>();
    p = add_identifier("bar", p, o);
    ASSERT_EQ(SDD(0, SDD(1, {0}, SDD(0, {0}, one)), one), sdd::order::sdd(p, initializer()));
  }
  {
    auto o = sdd::order::empty_order<conf>();
    o = add_identifier("foo", o);
    auto p = sdd::order::empty_order<conf>();
    p = add_identifier("bar1", p, o);
    p = add_identifier("bar2", p, o);
    ASSERT_EQ( SDD(1, SDD(0, {0}, one), SDD(0, SDD(0, {0}, one), one))
             , sdd::order::sdd(p, initializer()));
  }
}

/*-------------------------------------------------------------------------------------------*/
