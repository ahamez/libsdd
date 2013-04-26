#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/manager.hh"

#include "tests/configuration.hh"

/*------------------------------------------------------------------------------------------------*/

struct difference_test
  : public testing::Test
{
  typedef sdd::conf0 conf;
  typedef sdd::SDD<conf> SDD;

  sdd::manager<conf> m;
  sdd::dd::context<conf>& cxt;

  const SDD zero;
  const SDD one;

  difference_test()
    : m(sdd::manager<conf>::init(small_conf()))
    , cxt(sdd::global<conf>().sdd_context)
    , zero(sdd::zero<conf>())
    , one(sdd::one<conf>())
  {
  }
};

/*------------------------------------------------------------------------------------------------*/

TEST_F(difference_test, x_minus_zero)
{
  {
    conf::Values val;
    val.insert(0);
    const SDD x(0, val, one);
    ASSERT_EQ(x, difference(cxt, x, zero));
  }
  {
    conf::Values val;
    val.insert(0);
    const SDD x(0, val, SDD(1, val, one));
    ASSERT_EQ(x, difference(cxt, x, zero));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(difference_test, zero_minus_x)
{
  {
    conf::Values val;
    val.insert(0);
    const SDD x(0, val, one);
    ASSERT_EQ(zero, difference(cxt, zero, x));
  }
  {
    conf::Values val;
    val.insert(0);
    const SDD x(0, val, SDD(1, val, one));
    ASSERT_EQ(zero, difference(cxt, zero, x));    
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(difference_test, x_minus_x)
{
  {
    conf::Values val;
    val.insert(0);
    const SDD x(0, val, one);
    ASSERT_EQ(zero, difference(cxt, x, x));
  }
  {
    conf::Values val;
    val.insert(0);
    const SDD x(0, val, SDD(0, val, one));
    ASSERT_EQ(zero, difference(cxt, x, x));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(difference_test, flat_x_minus_y)
{
  {
    conf::Values valx;
    valx.insert(0);
    const SDD x(0, valx, one);

    conf::Values valy;
    valy.insert(1);
    const SDD y(0, valy, one);

    ASSERT_EQ(x, difference(cxt, x, y));
    ASSERT_EQ(y, difference(cxt, y, x));
  }
  {
    conf::Values valx;
    valx.insert(0);
    const SDD x(0, valx, SDD(1, valx, one));
    
    conf::Values valy;
    valy.insert(1);
    const SDD y(0, valy, SDD(1, valy, one));
    
    ASSERT_EQ(x, difference(cxt, x, y));
    ASSERT_EQ(y, difference(cxt, y, x));
  }
  {
    conf::Values valx;
    valx.insert(0);
    valx.insert(1);
    const SDD x(0, valx, one);
    
    conf::Values valy;
    valy.insert(1);
    const SDD y(0, valy, one);

    conf::Values valref;
    valref.insert(0);
    const SDD ref(0, valref, one);

    ASSERT_EQ(ref, difference(cxt, x, y));
    ASSERT_EQ(zero, difference(cxt, y, x));
  }
  {
    conf::Values valx;
    valx.insert(0);
    valx.insert(1);
    const SDD x(0, valx, SDD(1, valx, one));
    
    conf::Values valy;
    valy.insert(1);
    const SDD y(0, valy, SDD(1, valy, one));
    
    conf::Values valref0;
    valref0.insert(0);
    conf::Values valref1;
    valref1.insert(1);

    sdd::dd::sum_builder<conf, SDD> sum_ops;
    sum_ops.add(SDD(0, valref0, SDD(1, valref0, one)));
    sum_ops.add(SDD(0, valref0, SDD(1, valref1, one)));
    sum_ops.add(SDD(0, valref1, SDD(1, valref0, one)));
    const SDD ref = sum(cxt, std::move(sum_ops));

    ASSERT_EQ(ref, difference(cxt, x, y));
    ASSERT_EQ(zero, difference(cxt, y, x));
  }
  {
    conf::Values valx;
    valx.insert(0);
    const SDD x(0, valx, one);

    conf::Values valy;
    valy.insert(1);
    const SDD y(0, valy, one);

    sdd::dd::sum_builder<conf, SDD> sum_ops;
    sum_ops.add(x);
    sum_ops.add(y);
    const SDD x_plus_y = sum(cxt, std::move(sum_ops));
    
    ASSERT_EQ(x, difference(cxt, x, y));
    ASSERT_EQ(y, difference(cxt, y, x));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(difference_test, hierarchical_x_minus_y)
{
  {
    conf::Values valx;
    valx.insert(0);
    const SDD x(10, SDD(0, valx, one), one);
    
    conf::Values valy;
    valy.insert(1);
    const SDD y(10, SDD(0, valy, one), one);
    
    ASSERT_EQ(x, difference(cxt, x, y));
    ASSERT_EQ(y, difference(cxt, y, x));
  }
  {
    conf::Values valx;
    valx.insert(0);
    const SDD x(10, SDD(0, valx, SDD(0, valx, one)), one);
    
    conf::Values valy;
    valy.insert(1);
    const SDD y(10, SDD(0, valy, SDD(0, valy, one)), one);
    
    ASSERT_EQ(x, difference(cxt, x, y));
    ASSERT_EQ(y, difference(cxt, y, x));
  }
  {
    conf::Values valx;
    valx.insert(0);
    const SDD nestedx(0, valx, one);
    const SDD x(10, nestedx, SDD(11, nestedx, one));
    
    conf::Values valy;
    valy.insert(1);
    const SDD nestedy(0, valy, one);
    const SDD y(10, nestedy, SDD(11, nestedy, one));
    
    ASSERT_EQ(x, difference(cxt, x, y));
    ASSERT_EQ(y, difference(cxt, y, x));
  }
  {
    conf::Values valx;
    valx.insert(0);
    valx.insert(1);
    const SDD nestedx(0, valx, SDD(1, valx, one));
    const SDD x(10, nestedx, one);
    
    conf::Values valy;
    valy.insert(1);
    const SDD nestedy(0, valy, SDD(1, valy, one));
    const SDD y(10, nestedy, one);

    conf::Values valref0;
    valref0.insert(0);
    conf::Values valref1;
    valref1.insert(1);
    sdd::dd::sum_builder<conf, SDD> nested_sum_ops;
    nested_sum_ops.add(SDD(0, valref0, SDD(1, valref0, one)));
    nested_sum_ops.add(SDD(0, valref0, SDD(1, valref1, one)));
    nested_sum_ops.add(SDD(0, valref1, SDD(1, valref0, one)));
    const SDD nestedref = sum(cxt, std::move(nested_sum_ops));
    const SDD ref(10, nestedref, one);

    ASSERT_EQ(ref, difference(cxt, x, y));
    ASSERT_EQ(zero, difference(cxt, y, x));
  }
  {
    conf::Values valx;
    valx.insert(0);
    valx.insert(1);
    const SDD nestedx(0, valx, SDD(1, valx, one));
    const SDD x(10, nestedx, SDD(11, nestedx, one));

    conf::Values valy;
    valy.insert(1);
    const SDD nestedy(0, valy, SDD(1, valy, one));
    const SDD y(10, nestedy, SDD(11, nestedy, one));

    conf::Values valref0;
    valref0.insert(0);
    conf::Values valref1;
    valref1.insert(1);

    const SDD nested00(0, valref0, SDD(1, valref0, one));
    const SDD nested01(0, valref0, SDD(1, valref1, one));
    const SDD nested10(0, valref1, SDD(1, valref0, one));
    const SDD nested11(0, valref1, SDD(1, valref1, one));

    sdd::dd::sum_builder<conf, SDD> sum_ops;
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

TEST_F(difference_test, values)
{
  ASSERT_EQ(conf::Values{0}, difference(cxt, conf::Values{0,1}, conf::Values{1}));
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(difference_test, operators)
{
  ASSERT_EQ(zero, SDD('a', {0}, one) - SDD('a', {0}, one));
  ASSERT_EQ(SDD('a', {1}, one), SDD('a', {1}, one) - SDD('a', {0}, one));
  ASSERT_EQ(SDD('a', {0}, one), SDD('a', {0,1}, one) - SDD('a', {1,2}, one));

  SDD x('a', {0,1}, one);
  x -= SDD('a', {1}, one);
  ASSERT_EQ(SDD('a', {0}, one), x);
}

/*------------------------------------------------------------------------------------------------*/
