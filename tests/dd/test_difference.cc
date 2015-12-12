#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/manager.hh"

#include "tests/configuration.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct difference_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;
  sdd::dd::context<C>& cxt;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;

  difference_test()
    : m(init(small_conf<C>()))
    , cxt(sdd::global<C>().sdd_context)
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(difference_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(difference_test, x_minus_zero)
{
  {
    values_type val;
    val.insert(0);
    const SDD x(0, val, one);
    ASSERT_EQ(x, difference(cxt, x, zero));
  }
  {
    values_type val;
    val.insert(0);
    const SDD x(0, val, SDD(1, val, one));
    ASSERT_EQ(x, difference(cxt, x, zero));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(difference_test, zero_minus_x)
{
  {
    values_type val;
    val.insert(0);
    const SDD x(0, val, one);
    ASSERT_EQ(zero, difference(cxt, zero, x));
  }
  {
    values_type val;
    val.insert(0);
    const SDD x(0, val, SDD(1, val, one));
    ASSERT_EQ(zero, difference(cxt, zero, x));    
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(difference_test, x_minus_x)
{
  {
    values_type val;
    val.insert(0);
    const SDD x(0, val, one);
    ASSERT_EQ(zero, difference(cxt, x, x));
  }
  {
    values_type val;
    val.insert(0);
    const SDD x(0, val, SDD(0, val, one));
    ASSERT_EQ(zero, difference(cxt, x, x));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(difference_test, flat_x_minus_y)
{
  {
    values_type valx;
    valx.insert(0);
    const SDD x(0, valx, one);

    values_type valy;
    valy.insert(1);
    const SDD y(0, valy, one);

    ASSERT_EQ(x, difference(cxt, x, y));
    ASSERT_EQ(y, difference(cxt, y, x));
  }
  {
    values_type valx;
    valx.insert(0);
    const SDD x(0, valx, SDD(1, valx, one));
    
    values_type valy;
    valy.insert(1);
    const SDD y(0, valy, SDD(1, valy, one));
    
    ASSERT_EQ(x, difference(cxt, x, y));
    ASSERT_EQ(y, difference(cxt, y, x));
  }
  {
    values_type valx;
    valx.insert(0);
    valx.insert(1);
    const SDD x(0, valx, one);
    
    values_type valy;
    valy.insert(1);
    const SDD y(0, valy, one);

    values_type valref;
    valref.insert(0);
    const SDD ref(0, valref, one);

    ASSERT_EQ(ref, difference(cxt, x, y));
    ASSERT_EQ(zero, difference(cxt, y, x));
  }
  {
    values_type valx;
    valx.insert(0);
    valx.insert(1);
    const SDD x(0, valx, SDD(1, valx, one));
    
    values_type valy;
    valy.insert(1);
    const SDD y(0, valy, SDD(1, valy, one));
    
    values_type valref0;
    valref0.insert(0);
    values_type valref1;
    valref1.insert(1);

    sdd::dd::sum_builder<conf, SDD> sum_ops(cxt);
    sum_ops.add(SDD(0, valref0, SDD(1, valref0, one)));
    sum_ops.add(SDD(0, valref0, SDD(1, valref1, one)));
    sum_ops.add(SDD(0, valref1, SDD(1, valref0, one)));
    const SDD ref = sum(cxt, std::move(sum_ops));

    ASSERT_EQ(ref, difference(cxt, x, y));
    ASSERT_EQ(zero, difference(cxt, y, x));
  }
  {
    values_type valx;
    valx.insert(0);
    const SDD x(0, valx, one);

    values_type valy;
    valy.insert(1);
    const SDD y(0, valy, one);

    sdd::dd::sum_builder<conf, SDD> sum_ops(cxt);
    sum_ops.add(x);
    sum_ops.add(y);
    const SDD x_plus_y = sum(cxt, std::move(sum_ops));
    
    ASSERT_EQ(x, difference(cxt, x, y));
    ASSERT_EQ(y, difference(cxt, y, x));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(difference_test, hierarchical_x_minus_y)
{
  {
    values_type valx;
    valx.insert(0);
    const SDD x(10, SDD(0, valx, one), one);
    
    values_type valy;
    valy.insert(1);
    const SDD y(10, SDD(0, valy, one), one);
    
    ASSERT_EQ(x, difference(cxt, x, y));
    ASSERT_EQ(y, difference(cxt, y, x));
  }
  {
    values_type valx;
    valx.insert(0);
    const SDD x(10, SDD(0, valx, SDD(0, valx, one)), one);
    
    values_type valy;
    valy.insert(1);
    const SDD y(10, SDD(0, valy, SDD(0, valy, one)), one);
    
    ASSERT_EQ(x, difference(cxt, x, y));
    ASSERT_EQ(y, difference(cxt, y, x));
  }
  {
    values_type valx;
    valx.insert(0);
    const SDD nestedx(0, valx, one);
    const SDD x(10, nestedx, SDD(11, nestedx, one));
    
    values_type valy;
    valy.insert(1);
    const SDD nestedy(0, valy, one);
    const SDD y(10, nestedy, SDD(11, nestedy, one));
    
    ASSERT_EQ(x, difference(cxt, x, y));
    ASSERT_EQ(y, difference(cxt, y, x));
  }
  {
    values_type valx;
    valx.insert(0);
    valx.insert(1);
    const SDD nestedx(0, valx, SDD(1, valx, one));
    const SDD x(10, nestedx, one);
    
    values_type valy;
    valy.insert(1);
    const SDD nestedy(0, valy, SDD(1, valy, one));
    const SDD y(10, nestedy, one);

    values_type valref0;
    valref0.insert(0);
    values_type valref1;
    valref1.insert(1);
    sdd::dd::sum_builder<conf, SDD> nested_sum_ops(cxt);
    nested_sum_ops.add(SDD(0, valref0, SDD(1, valref0, one)));
    nested_sum_ops.add(SDD(0, valref0, SDD(1, valref1, one)));
    nested_sum_ops.add(SDD(0, valref1, SDD(1, valref0, one)));
    const SDD nestedref = sum(cxt, std::move(nested_sum_ops));
    const SDD ref(10, nestedref, one);

    ASSERT_EQ(ref, difference(cxt, x, y));
    ASSERT_EQ(zero, difference(cxt, y, x));
  }
  {
    values_type valx;
    valx.insert(0);
    valx.insert(1);
    const SDD nestedx(0, valx, SDD(1, valx, one));
    const SDD x(10, nestedx, SDD(11, nestedx, one));

    values_type valy;
    valy.insert(1);
    const SDD nestedy(0, valy, SDD(1, valy, one));
    const SDD y(10, nestedy, SDD(11, nestedy, one));

    values_type valref0;
    valref0.insert(0);
    values_type valref1;
    valref1.insert(1);

    const SDD nested00(0, valref0, SDD(1, valref0, one));
    const SDD nested01(0, valref0, SDD(1, valref1, one));
    const SDD nested10(0, valref1, SDD(1, valref0, one));
    const SDD nested11(0, valref1, SDD(1, valref1, one));

    sdd::dd::sum_builder<conf, SDD> sum_ops(cxt);
    sum_ops.add(SDD(10, nested00, SDD(11, nested00, one)));
    sum_ops.add(SDD(10, nested00, SDD(11, nested01, one)));
    sum_ops.add(SDD(10, nested00, SDD(11, nested10, one)));
    sum_ops.add(SDD(10, nested00, SDD(11, nested11, one)));
    sum_ops.add(SDD(10, nested01, SDD(11, nested00, one)));
    sum_ops.add(SDD(10, nested01, SDD(11, nested01, one)));
    sum_ops.add(SDD(10, nested01, SDD(11, nested10, one)));
    sum_ops.add(SDD(10, nested01, SDD(11, nested11, one)));
    sum_ops.add(SDD(10, nested10, SDD(11, nested00, one)));
    sum_ops.add(SDD(10, nested10, SDD(11, nested01, one)));
    sum_ops.add(SDD(10, nested10, SDD(11, nested10, one)));
    sum_ops.add(SDD(10, nested10, SDD(11, nested11, one)));
    sum_ops.add(SDD(10, nested11, SDD(11, nested00, one)));
    sum_ops.add(SDD(10, nested11, SDD(11, nested01, one)));
    sum_ops.add(SDD(10, nested11, SDD(11, nested10, one)));

    const SDD ref = sum(cxt, std::move(sum_ops));

    ASSERT_EQ(ref, difference(cxt, x, y));
    ASSERT_EQ(zero, difference(cxt, y, x));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(difference_test, values)
{
  ASSERT_EQ(values_type{0}, difference(cxt, values_type{0,1}, values_type{1}));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(difference_test, operators)
{
  ASSERT_EQ(zero, SDD('a', {0}, one) - SDD('a', {0}, one));
  ASSERT_EQ(SDD('a', {1}, one), SDD('a', {1}, one) - SDD('a', {0}, one));
  ASSERT_EQ(SDD('a', {0}, one), SDD('a', {0,1}, one) - SDD('a', {1,2}, one));

  SDD x('a', {0,1}, one);
  x -= SDD('a', {1}, one);
  ASSERT_EQ(SDD('a', {0}, one), x);
}

/*------------------------------------------------------------------------------------------------*/
