#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/dd/paths.hh"
#include "sdd/manager.hh"

#include "tests/configuration.hh"

/*------------------------------------------------------------------------------------------------*/

struct paths_test
  : public testing::Test
{
  typedef sdd::conf0 conf;
  typedef sdd::SDD<conf> SDD;

  sdd::manager<conf> m;

  const SDD zero;
  const SDD one;

  paths_test()
    : m(sdd::manager<conf>::init(small_conf()))
    , zero(sdd::zero<conf>())
    , one(sdd::one<conf>())
  {
  }
};

/*------------------------------------------------------------------------------------------------*/

TEST_F(paths_test, zero)
{
  ASSERT_EQ(0, count_paths(zero));
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(paths_test, one)
{
  ASSERT_EQ(1, count_paths(one));
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(paths_test, flat)
{
  ASSERT_EQ(3, count_paths(SDD('a', {0,1,2}, one)));
  ASSERT_EQ(9, count_paths(SDD('a', {0,1,2}, SDD('b', {0,1,2}, one))));
  ASSERT_EQ(6, count_paths(SDD('a', {0,1,2}, one) + SDD('a', {3,4,5}, one)));
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(paths_test, hierarchical)
{
  ASSERT_EQ(3, count_paths(SDD('a', SDD('b', {0,1,2}, one), one)));
  ASSERT_EQ(9, count_paths( SDD( 'a', SDD('b', {0,1,2}, one)
                               , SDD( 'a', SDD('b', {0,1,2}, one)
                                    , one
                                    )
                               )
                          )
           );
  ASSERT_EQ(9, count_paths( SDD( 'a', SDD('b', SDD('c', {0,1,2}, one), one)
                               , SDD( 'a', SDD('b', SDD('c', {0,1,2}, one), one)
                                    , one
                                    )
                               )
                          )
           );
  ASSERT_EQ(6, count_paths( SDD('a', SDD('b', {0,1,2}, one), one)
                          + SDD('a', SDD('b', {3,4,5}, one), one)));
}

/*------------------------------------------------------------------------------------------------*/
