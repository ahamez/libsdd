#include <vector>

#include "gtest/gtest.h"

#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/manager.hh"

#include "tests/configuration.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct sum_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;
  sdd::dd::context<C>& cxt;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;

  sum_test()
    : m(sdd::init(small_conf<C>()))
    , cxt(sdd::global<C>().sdd_context)
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(sum_test, configurations);
#include "tests/macros.hh"
#define flat_alpha_builder sdd::dd::alpha_builder<conf, values_type>
#define hier_alpha_builder sdd::dd::alpha_builder<conf, SDD>
#define flat_sum_builder sdd::dd::sum_builder<conf, values_type>

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(sum_test, empty_operands)
{
  ASSERT_EQ(zero, sum(cxt, {cxt, {}}));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(sum_test, one_operand)
{
  {
    ASSERT_EQ(one, sum(cxt, {cxt, {one}}));
  }
  {
    ASSERT_EQ(zero, sum(cxt, {cxt, {zero}}));
  }
  {
    SDD x(0, {0}, one);
    ASSERT_EQ(x, sum(cxt, {cxt, {x}}));
  }
  {
    SDD x(0, SDD(1, {0}, one), one);
    ASSERT_EQ(x, sum(cxt, {cxt, {x}}));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(sum_test, any_with_zero)
{
  {
    ASSERT_EQ(one, sum(cxt, {cxt, {zero, one}}));
    ASSERT_EQ(one, sum(cxt, {cxt, {one, zero}}));
  }
  {
    SDD x(0, {0}, one);
    ASSERT_EQ(x, sum(cxt, {cxt, {x, zero}}));
    ASSERT_EQ(x, sum(cxt, {cxt, {zero, x}}));
  }
  {
    SDD x(0, SDD(1, {0}, one), one);
    ASSERT_EQ(x, sum(cxt, {cxt, {zero, x}}));
    ASSERT_EQ(x, sum(cxt, {cxt, {x, zero}}));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(sum_test, same_operand_n_times)
{
  {
    ASSERT_EQ(one, sum(cxt, {cxt, {one, one, one}}));
  }
  {
    ASSERT_EQ(zero, sum(cxt, {cxt, {zero, zero, zero}}));
  }
  {
    SDD x(0, {0}, one);
    ASSERT_EQ(x, sum(cxt, {cxt, {x, x, x}}));
  }
  {
    SDD x(0, SDD(1, {0}, one), one);
    SDD s = sum(cxt, {cxt, {x,x}});
    ASSERT_EQ(x, sum(cxt, {cxt, {x, x}}));
    ASSERT_EQ(x, sum(cxt, {cxt, {x, x, x}}));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(sum_test, flat_same_partition_different_succesors)
{
  SDD x0(0, {0}, SDD(1, {0}, one));
  SDD x1(0, {0}, SDD(1, {1}, one));

  ASSERT_EQ( SDD(0, {0}, SDD(1, {0,1}, one))
           , sum(cxt, {cxt, {x0, x1}}));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(sum_test, flat_commutativity)
{
  SDD za(0, {1}  , SDD(1, {1}, one));
  SDD zb(0, {2,3}, SDD(1, {2,3}, one));
  SDD zc(0, {3,4}, SDD(1, {3,4}, one));
  
  SDD za_zb = sum(cxt, {cxt, {za,zb}});
  SDD za_zc = sum(cxt, {cxt, {za,zc}});
  SDD zb_zc = sum(cxt, {cxt, {zb,zc}});
  
  SDD za_zb_za_zc = sum(cxt, {cxt, {za_zb,za_zc}});
  SDD za_zb_zb_zc = sum(cxt, {cxt, {za_zb,zb_zc}});
  
  ASSERT_EQ(za_zb_za_zc, za_zb_zb_zc);
  
  SDD z = sum(cxt, {cxt, {za,zb,zc}});
  
  ASSERT_EQ(za_zb_za_zc, z);
  ASSERT_EQ(za_zb_zb_zc, z);
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(sum_test, flat_no_successors)
{
  {
    flat_alpha_builder builder(cxt);
    builder.add(values_type {0,1}, one);
    ASSERT_EQ( SDD(0, std::move(builder))
             , sum(cxt, {cxt, {SDD(0, {0}, one), SDD(0, {1}, one)}}));
  }
  {
    flat_alpha_builder builder(cxt);
    builder.add(values_type {0,1}, one);
    ASSERT_EQ( SDD(0, std::move(builder))
             , sum(cxt, {cxt, {SDD(0, {0,1}, one), SDD(0, {1}, one)}}));
  }
  {
    flat_alpha_builder builder(cxt);
    builder.add(values_type {0,1}, one);
    ASSERT_EQ( SDD(0, std::move(builder))
             , sum(cxt, {cxt, {SDD(0, {0,1}, one), SDD(0, {0,1}, one)}}));
  }
  {
    // Force creation order.
    SDD a0(0, {0,1,2}, one);
    SDD a1(0, {1,2}, one);
    SDD a2(0, {0,1}, one);
    ASSERT_EQ(SDD(0, {0,1,2}, one), sum(cxt, {cxt, {a0, a1, a2}}));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(sum_test, hierarchical_no_successors)
{
  {
    hier_alpha_builder builder(cxt);
    builder.add(SDD(0, {0,1}, one), one);
    ASSERT_EQ( SDD(10, std::move(builder))
             , sum(cxt, {cxt, { SDD(10, SDD(0, {0}, one), one)
                              , SDD(10, SDD(0, {1}, one), one)}}));
  }
  {
    hier_alpha_builder builder(cxt);
    builder.add(SDD(0, {0,1}, one), one);
    ASSERT_EQ( SDD(10, std::move(builder))
             , sum(cxt, {cxt, { SDD(10, SDD(0, {0,1}, one), one)
                              , SDD(10, SDD(0, {1}, one), one)}}));
  }
  {
    hier_alpha_builder builder(cxt);
    builder.add(SDD(0, {0,1}, one), one);
    ASSERT_EQ( SDD(10, std::move(builder))
             , sum(cxt, {cxt, { SDD(10, SDD(0, {0,1}, one), one)
                              , SDD(10, SDD(0, {0,1}, one), one)}}));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(sum_test, flat_partition_changing)
{
  flat_alpha_builder builder(cxt);
  builder.add(values_type {1}, SDD(1, {4}, one));
  builder.add(values_type {2}, SDD(1, {4,5}, one));
  builder.add(values_type {3}, SDD(1, {5}, one));
  ASSERT_EQ( SDD(0, std::move(builder))
           , sum(cxt, {cxt, { SDD(0, {1,2}, SDD(1, {4}, one))
                            , SDD(0, {2,3}, SDD(1, {5}, one))}}));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(sum_test, hierarchical_partition_changing)
{
  hier_alpha_builder builder(cxt);
  builder.add(SDD('a', {1}, one), SDD('y', SDD('b', {4}, one), one));
  builder.add(SDD('a', {2}, one), SDD('y', SDD('b', {4,5}, one), one));
  builder.add(SDD('a', {3}, one), SDD('y', SDD('b', {5}, one), one));
  ASSERT_EQ( SDD('x', std::move(builder))
           , sum(cxt, {cxt, { SDD('x', SDD('a', {1,2}, one), SDD('y', SDD('b', {4}, one), one))
                            , SDD('x', SDD('a', {2,3}, one), SDD('y', SDD('b', {5}, one), one))}}));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(sum_test, values)
{
  {
    flat_sum_builder ops{cxt};
    ASSERT_EQ(values_type(), sum(cxt, std::move(ops)));
  }
  {
    values_type val0 {0};
    values_type val1 {1};
    values_type val2 {2};
    values_type ref {0,1,2};
    flat_sum_builder ops(cxt, {val0, val1, val2});
    ASSERT_EQ(ref, sum(cxt, std::move(ops)));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(sum_test, iterable)
{
  std::vector<SDD> vec { SDD('a', {0,1}, one)
                       , SDD('a', {0,2}, one)
                       , SDD('a', {0,1}, one)
                       , SDD('a', {1,2}, one)};
  ASSERT_EQ(SDD('a', {0,1,2}, one), sdd::sum<conf>(vec.begin(), vec.end()));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(sum_test, initializer_list)
{
  ASSERT_EQ(SDD('a', {0,1,2}, one), sdd::sum({ SDD('a', {0,1}, one)
                                             , SDD('a', {0,2}, one)
                                             , SDD('a', {0,1}, one)
                                             , SDD('a', {1,2}, one)}));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(sum_test, operators)
{
  ASSERT_EQ(SDD('a', {0,1}, one), SDD('a', {0}, one) + SDD('a', {1}, one));

  SDD x('a', {0}, one);
  x += SDD('a', {1}, one);
  ASSERT_EQ(SDD('a', {0,1}, one), x);
}

/*------------------------------------------------------------------------------------------------*/
