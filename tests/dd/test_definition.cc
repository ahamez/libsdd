#include <sstream>

#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/check_compatibility.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/manager.hh"

#include "tests/configuration.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct definition_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;

  definition_test()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(definition_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(definition_test, empty_successor)
{
  ASSERT_EQ(zero, SDD('a', {0}, zero));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(definition_test, empty_valuation)
{
  ASSERT_EQ(zero, SDD('a', values_type(), one));
  ASSERT_EQ(zero, SDD('a', zero, one));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(definition_test, print)
{
  {
    std::stringstream ss;
    ss << zero;
    ASSERT_NE(0u, ss.str().size());
  }
  {
    std::stringstream ss;
    ss << one;
    ASSERT_NE(0u, ss.str().size());
  }
  {
    std::stringstream ss;
    ss << SDD('a', {0}, one);
    ASSERT_NE(0u, ss.str().size());
  }
  {
    std::stringstream ss;
    ss << SDD('a', {0,1}, one);
    ASSERT_NE(0u, ss.str().size());
  }
  {
    std::stringstream ss;
    SDD x('x', SDD('a', {0}, one), SDD('y', SDD('b', {0}, one), one));
    SDD y('x', SDD('a', {1}, one), SDD('y', SDD('b', {1}, one), one));
    ss << x + y;
    ASSERT_NE(0u, ss.str().size());
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(definition_test, check_compatibility)
{
  using namespace sdd::dd;
  {
    ASSERT_NO_THROW(check_compatibility(zero, zero));
    ASSERT_NO_THROW(check_compatibility(one, one));
    ASSERT_NO_THROW(check_compatibility(SDD(0, {1}, one), SDD(0, {2}, one)));
    ASSERT_NO_THROW(check_compatibility( SDD(0, SDD(1, {1}, one), one)
                                       , SDD(0, SDD(2, {2}, one), one)));
  }
  {
    ASSERT_THROW(check_compatibility(zero, one), sdd::top<conf>);
    ASSERT_THROW(check_compatibility(one, zero), sdd::top<conf>);

    ASSERT_THROW(check_compatibility(zero, SDD(0, {1}, one)), sdd::top<conf>);
    ASSERT_THROW(check_compatibility(SDD(0, {1}, one), zero), sdd::top<conf>);
    ASSERT_THROW(check_compatibility(one, SDD(0, {1}, one)), sdd::top<conf>);
    ASSERT_THROW(check_compatibility(SDD(0, {1}, one), one), sdd::top<conf>);

    ASSERT_THROW(check_compatibility(zero, SDD(0, SDD(1, {1}, one), one)), sdd::top<conf>);
    ASSERT_THROW(check_compatibility(SDD(0, SDD(1, {1}, one), one), zero), sdd::top<conf>);
    ASSERT_THROW(check_compatibility(one, SDD(0, SDD(1, {1}, one), one)), sdd::top<conf>);
    ASSERT_THROW(check_compatibility(SDD(0, SDD(1, {1}, one), one), one), sdd::top<conf>);

    ASSERT_THROW(check_compatibility(SDD(0, {1}, one), SDD(1, {1}, one)), sdd::top<conf>);
    ASSERT_THROW(check_compatibility(SDD(1, {1}, one), SDD(0, {1}, one)), sdd::top<conf>);

    ASSERT_THROW( check_compatibility(SDD(0, SDD(1, {1}, one), one), SDD(1, one, one))
                , sdd::top<conf>);
    ASSERT_THROW( check_compatibility(SDD(1, one, one), SDD(0, SDD(1, {1}, one), one))
                , sdd::top<conf>);

    ASSERT_THROW(check_compatibility(SDD(0, {1}, one), SDD(1, one, one)), sdd::top<conf>);
    ASSERT_THROW(check_compatibility(SDD(0, one, one), SDD(1, {1}, one)), sdd::top<conf>);
    ASSERT_THROW(check_compatibility(SDD(0, {1}, one), SDD(1, one, one)), sdd::top<conf>);
    ASSERT_THROW(check_compatibility(SDD(1, one, one), SDD(0, {1}, one)), sdd::top<conf>);

  }
}

/*------------------------------------------------------------------------------------------------*/
