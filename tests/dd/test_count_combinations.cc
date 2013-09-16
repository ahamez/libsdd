#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/count_combinations.hh"
#include "sdd/dd/definition.hh"
#include "sdd/manager.hh"

#include "tests/configuration.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct count_combinations_test
  : public testing::Test
{
  typedef C configuration_type;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;

  count_combinations_test()
    : m(sdd::manager<C>::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(count_combinations_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(count_combinations_test, terminal_zero)
{
  ASSERT_EQ(0u, count_combinations(zero));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(count_combinations_test, terminal_one)
{
  ASSERT_EQ(1u, count_combinations(one));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(count_combinations_test, flat)
{
  ASSERT_EQ(3u, count_combinations(SDD('a', {0,1,2}, one)));
  ASSERT_EQ(9u, count_combinations(SDD('a', {0,1,2}, SDD('b', {0,1,2}, one))));
  ASSERT_EQ(6u, count_combinations(SDD('a', {0,1,2}, one) + SDD('a', {3,4,5}, one)));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(count_combinations_test, hierarchical)
{
  ASSERT_EQ(3, count_combinations(SDD('a', SDD('b', {0,1,2}, one), one)));
  ASSERT_EQ(9, count_combinations( SDD( 'a', SDD('b', {0,1,2}, one)
                               , SDD( 'a', SDD('b', {0,1,2}, one)
                                    , one
                                    )
                               )
                          )
           );
  ASSERT_EQ(9, count_combinations( SDD( 'a', SDD('b', SDD('c', {0,1,2}, one), one)
                               , SDD( 'a', SDD('b', SDD('c', {0,1,2}, one), one)
                                    , one
                                    )
                               )
                          )
           );
  ASSERT_EQ(6, count_combinations( SDD('a', SDD('b', {0,1,2}, one), one)
                          + SDD('a', SDD('b', {3,4,5}, one), one)));
}

/*------------------------------------------------------------------------------------------------*/