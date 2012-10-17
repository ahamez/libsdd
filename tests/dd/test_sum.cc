#include <vector>

#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"

/*-------------------------------------------------------------------------------------------*/

struct sum_test
  : public testing::Test
{
  typedef sdd::conf::conf0 conf;
  typedef sdd::SDD<conf> SDD;
  sdd::context<conf> cxt;
  typedef sdd::alpha_builder<conf, conf::Values> flat_alpha_builder;
  typedef sdd::alpha_builder<conf, SDD> hier_alpha_builder;
  const SDD zero = sdd::zero<conf>();
  const SDD one = sdd::one<conf>();

  sum_test()
  {
  }
};

/*-------------------------------------------------------------------------------------------*/

TEST_F(sum_test, empty_operands)
{
  ASSERT_EQ(zero, sum(cxt, {}));
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(sum_test, one_operand)
{
  {
    ASSERT_EQ(one, sum(cxt, {one}));
  }
  {
    ASSERT_EQ(zero, sum(cxt, {zero}));
  }
  {
    SDD x(0, {0}, one);
    ASSERT_EQ(x, sum(cxt, {x}));
  }
  {
    SDD x(0, SDD(1, {0}, one), one);
    ASSERT_EQ(x, sum(cxt, {x}));
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(sum_test, any_with_zero)
{
  {
    ASSERT_EQ(one, sum(cxt, {zero, one}));
    ASSERT_EQ(one, sum(cxt, {one, zero}));
  }
  {
    SDD x(0, {0}, one);
    ASSERT_EQ(x, sum(cxt, {x, zero}));
    ASSERT_EQ(x, sum(cxt, {zero, x}));
  }
  {
    SDD x(0, SDD(1, {0}, one), one);
    ASSERT_EQ(x, sum(cxt, {zero, x}));
    ASSERT_EQ(x, sum(cxt, {x, zero}));
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(sum_test, same_operand_n_times)
{
  {
    ASSERT_EQ(one, sum(cxt, {one, one, one}));
  }
  {
    ASSERT_EQ(zero, sum(cxt, {zero, zero, zero}));
  }
  {
    SDD x(0, {0}, one);
    ASSERT_EQ(x, sum(cxt, {x, x, x}));
  }
  {
    SDD x(0, SDD(1, {0}, one), one);
    SDD s = sum(cxt, {x,x});
    ASSERT_EQ(x, sum(cxt, {x, x}));
    ASSERT_EQ(x, sum(cxt, {x, x, x}));
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(sum_test, flat_same_partition_different_succesors)
{
  SDD x0(0, {0}, SDD(1, {0}, one));
  SDD x1(0, {0}, SDD(1, {1}, one));

  ASSERT_EQ( SDD(0, {0}, SDD(1, {0,1}, one))
           , sum(cxt, {x0, x1}));
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(sum_test, flat_commutativity)
{
  SDD za(0, {1}  , SDD(1, {1}, one));
  SDD zb(0, {2,3}, SDD(1, {2,3}, one));
  SDD zc(0, {3,4}, SDD(1, {3,4}, one));
  
  SDD za_zb = sum(cxt, {za,zb});
  SDD za_zc = sum(cxt, {za,zc});
  SDD zb_zc = sum(cxt, {zb,zc});
  
  SDD za_zb_za_zc = sum(cxt, {za_zb,za_zc});
  SDD za_zb_zb_zc = sum(cxt, {za_zb,zb_zc});
  
  ASSERT_EQ(za_zb_za_zc, za_zb_zb_zc);
  
  SDD z = sum(cxt, {za,zb,zc});
  
  ASSERT_EQ(za_zb_za_zc, z);
  ASSERT_EQ(za_zb_zb_zc, z);
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(sum_test, flat_no_successors)
{
  {
    flat_alpha_builder builder;
    builder.add({0,1}, SDD(true));
    ASSERT_EQ( SDD(0, std::move(builder))
             , sum(cxt, {SDD(0, {0}, one), SDD(0, {1}, one)}));
  }
  {
    flat_alpha_builder builder;
    builder.add({0,1}, SDD(true));
    ASSERT_EQ( SDD(0, std::move(builder))
             , sum(cxt, {SDD(0, {0,1}, one), SDD(0, {1}, one)}));
  }
  {
    flat_alpha_builder builder;
    builder.add({0,1}, SDD(true));
    ASSERT_EQ( SDD(0, std::move(builder))
             , sum(cxt, {SDD(0, {0,1}, one), SDD(0, {0,1}, one)}));
  }
  {
    // Force creation order.
    SDD a0(0, {0,1,2}, one);
    SDD a1(0, {1,2}, one);
    SDD a2(0, {0,1}, one);
    ASSERT_EQ(SDD(0, {0,1,2}, one), sum(cxt, {a0, a1, a2}));
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(sum_test, hierarchical_no_successors)
{
  {
    hier_alpha_builder builder;
    builder.add(SDD(0, {0,1}, one), SDD(true));
    ASSERT_EQ( SDD(10, std::move(builder))
             , sum(cxt, { SDD(10, SDD(0, {0}, one), one)
                        , SDD(10, SDD(0, {1}, one), one)}));
  }
  {
    hier_alpha_builder builder;
    builder.add(SDD(0, {0,1}, one), SDD(true));
    ASSERT_EQ( SDD(10, std::move(builder))
             , sum(cxt, { SDD(10, SDD(0, {0,1}, one), one)
                        , SDD(10, SDD(0, {1}, one), one)}));
  }
  {
    hier_alpha_builder builder;
    builder.add(SDD(0, {0,1}, one), SDD(true));
    ASSERT_EQ( SDD(10, std::move(builder))
             , sum(cxt, { SDD(10, SDD(0, {0,1}, one), one)
             , SDD(10, SDD(0, {0,1}, one), one)}));
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(sum_test, flat_partition_changing)
{
  flat_alpha_builder builder;
  builder.add({1}, SDD(1, {4}, one));
  builder.add({2}, SDD(1, {4,5}, one));
  builder.add({3}, SDD(1, {5}, one));
  ASSERT_EQ( SDD(0, std::move(builder))
           , sum(cxt, { SDD(0, {1,2}, SDD(1, {4}, one))
                      , SDD(0, {2,3}, SDD(1, {5}, one))}));
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(sum_test, hierarchical_partition_changing)
{
  hier_alpha_builder builder;
  builder.add(SDD('a', {1}, one), SDD('y', SDD('b', {4}, one), one));
  builder.add(SDD('a', {2}, one), SDD('y', SDD('b', {4,5}, one), one));
  builder.add(SDD('a', {3}, one), SDD('y', SDD('b', {5}, one), one));
  ASSERT_EQ( SDD('x', std::move(builder))
           , sum(cxt, { SDD('x', SDD('a', {1,2}, one), SDD('y', SDD('b', {4}, one), one))
                      , SDD('x', SDD('a', {2,3}, one), SDD('y', SDD('b', {5}, one), one))}));
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(sum_test, values)
{
  {
    sdd::sum_builder<conf, conf::Values> ops;
    ASSERT_EQ(conf::Values(), sum(cxt, std::move(ops)));
  }
  {
    conf::Values val0 {0};
    conf::Values val1 {1};
    conf::Values val2 {2};
    conf::Values ref {0,1,2};
    sdd::sum_builder<conf, conf::Values> ops {val0, val1, val2};
    ASSERT_EQ(ref, sum(cxt, std::move(ops)));
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(sum_test, iterable)
{
  std::vector<SDD> vec { SDD('a', {0,1}, one)
                       , SDD('a', {0,2}, one)
                       , SDD('a', {0,1}, one)
                       , SDD('a', {1,2}, one)};
  ASSERT_EQ(SDD('a', {0,1,2}, one), sdd::sum<conf>(vec.begin(), vec.end()));
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(sum_test, initializer_list)
{
  ASSERT_EQ(SDD('a', {0,1,2}, one), sdd::sum({ SDD('a', {0,1}, one)
                                                 , SDD('a', {0,2}, one)
                                                 , SDD('a', {0,1}, one)
                                                 , SDD('a', {1,2}, one)}));
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(sum_test, operators)
{
  ASSERT_EQ(SDD('a', {0,1}, one), SDD('a', {0}, one) + SDD('a', {1}, one));

  SDD x('a', {0}, one);
  x += SDD('a', {1}, one);
  ASSERT_EQ(SDD('a', {0,1}, one), x);
}

/*-------------------------------------------------------------------------------------------*/
