#include "gtest/gtest.h"

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
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;

  count_combinations_test()
    : m(sdd::init(small_conf<C>()))
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
  ASSERT_EQ(0u, sdd::dd::count_combinations(zero));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(count_combinations_test, terminal_one)
{
  ASSERT_EQ(1u, sdd::dd::count_combinations(one));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(count_combinations_test, flat)
{
  ASSERT_EQ(3u, sdd::dd::count_combinations(SDD('a', {0,1,2}, one)));
  ASSERT_EQ(9u, sdd::dd::count_combinations(SDD('a', {0,1,2}, SDD('b', {0,1,2}, one))));
  ASSERT_EQ(6u, sdd::dd::count_combinations(SDD('a', {0,1,2}, one) + SDD('a', {3,4,5}, one)));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(count_combinations_test, hierarchical)
{
  ASSERT_EQ(3, sdd::dd::count_combinations(SDD('a', SDD('b', {0,1,2}, one), one)));
  ASSERT_EQ(9, sdd::dd::count_combinations( SDD( 'a', SDD('b', {0,1,2}, one)
                               , SDD( 'a', SDD('b', {0,1,2}, one)
                                    , one
                                    )
                               )
                          )
           );
  ASSERT_EQ(9, sdd::dd::count_combinations( SDD( 'a', SDD('b', SDD('c', {0,1,2}, one), one)
                               , SDD( 'a', SDD('b', SDD('c', {0,1,2}, one), one)
                                    , one
                                    )
                               )
                          )
           );
  ASSERT_EQ(6, sdd::dd::count_combinations( SDD('a', SDD('b', {0,1,2}, one), one)
                          + SDD('a', SDD('b', {3,4,5}, one), one)));
}

/*------------------------------------------------------------------------------------------------*/
