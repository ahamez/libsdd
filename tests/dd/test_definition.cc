#include <sstream>

#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"

/*------------------------------------------------------------------------------------------------*/

struct definition_test
  : public testing::Test
{
  typedef sdd::conf::conf0 conf;
  typedef sdd::SDD<conf> SDD;
  const SDD zero = sdd::zero<conf>();
  const SDD one = sdd::one<conf>();

  definition_test()
  {
  }
};

/*------------------------------------------------------------------------------------------------*/

TEST_F(definition_test, empty_successor)
{
  ASSERT_EQ(zero, SDD('a', {0}, zero));
  conf::Values val;
  val.insert(0);
  ASSERT_EQ(zero, SDD('a', val, zero));
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(definition_test, empty_valuation)
{
  ASSERT_EQ(zero, SDD('a', {}, one));
  conf::Values val;
  ASSERT_EQ(zero, SDD('a', val, one));
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(definition_test, print)
{
  {
    std::stringstream ss;
    ss << zero;
    ASSERT_NE(static_cast<std::size_t>(0), ss.str().size());
  }
  {
    std::stringstream ss;
    ss << one;
    ASSERT_NE(static_cast<std::size_t>(0), ss.str().size());
  }
  {
    std::stringstream ss;
    ss << SDD('a', {0}, one);
    ASSERT_NE(static_cast<std::size_t>(0), ss.str().size());
  }
  {
    std::stringstream ss;
    ss << SDD('a', {0,1}, one);
    ASSERT_NE(static_cast<std::size_t>(0), ss.str().size());
  }
  {
    std::stringstream ss;
    SDD x('x', SDD('a', {0}, one), SDD('y', SDD('b', {0}, one), one));
    SDD y('x', SDD('a', {1}, one), SDD('y', SDD('b', {1}, one), one));
    ss << x + y;
    ASSERT_NE(static_cast<std::size_t>(0), ss.str().size());
  }
}

/*------------------------------------------------------------------------------------------------*/
