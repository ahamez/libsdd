#include <vector>

#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"

/*------------------------------------------------------------------------------------------------*/

struct intersection_test
  : public testing::Test
{
  typedef sdd::conf::conf0 conf;
  typedef sdd::SDD<conf> SDD;
  sdd::dd::context<conf> cxt;
  const SDD zero = sdd::zero<conf>();
  const SDD one = sdd::one<conf>();
};

/*------------------------------------------------------------------------------------------------*/

TEST_F(intersection_test, empty_operand)
{
  ASSERT_EQ(zero, intersection(cxt, {SDD('a', {}, one)}));
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(intersection_test, empty_intersection)
{
  conf::Values val {0};
  sdd::dd::intersection_builder<conf, SDD> ops;
  ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(intersection_test, flat_x_inter_zero)
{
  {
    conf::Values val {0};
    sdd::dd::intersection_builder<conf, SDD> ops;
    ops.add(SDD(0, val, one));
    ops.add(zero);
    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    conf::Values val {0,1,2,3};
    sdd::dd::intersection_builder<conf, SDD> ops;
    ops.add(zero);
    ops.add(SDD(0, val, one));
    ops.add(SDD(0, val, one));
    ops.add(SDD(0, val, one));
    ops.add(SDD(0, val, one));
    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    conf::Values val {0};
    sdd::dd::intersection_builder<conf, SDD> ops;
    ops.add(SDD(0, val, SDD(0, val, one)));
    ops.add(sdd::zero<conf>());
    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    conf::Values val {0,1,2,3};
    sdd::dd::intersection_builder<conf, SDD> ops;
    ops.add(SDD(0, val, SDD(0, val, one)));
    ops.add(SDD(0, val, SDD(0, val, one)));
    ops.add(SDD(0, val, SDD(0, val, one)));
    ops.add(SDD(0, val, SDD(0, val, one)));
    ops.add(zero);
    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    conf::Values val {0};    
    sdd::dd::intersection_builder<conf, SDD> ops;
    ops.add(zero);
    ops.add(zero);
    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(intersection_test, flat_x_inter_y)
{
  {
    SDD x(0, {0,1}, one);
    SDD y(0, {1,2}, one);
    SDD ref(0, {1}, one);
    ASSERT_EQ(ref, intersection(cxt, {x,y}));

    sdd::dd::sum_builder<conf, SDD> sum_ops_x;
    sum_ops_x.add(ref);
    sum_ops_x.add(x);
    ASSERT_EQ(x, sum(cxt, std::move(sum_ops_x)));

    sdd::dd::sum_builder<conf, SDD> sum_ops_y;
    sum_ops_y.add(ref);
    sum_ops_y.add(y);
    ASSERT_EQ(y, sum(cxt, std::move(sum_ops_y)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops;

    conf::Values valx {0,1};
    ops.add(SDD(0, valx, one));

    conf::Values valy {2,3};
    ops.add(SDD(0, valy, one));

    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops;

    conf::Values valx {0,1};
    ops.add(SDD(0, valx, SDD(1, valx, one)));

    conf::Values valy {1,2};
    ops.add(SDD(0, valy, SDD(1, valy, one)));

    conf::Values valref {1};

    ASSERT_EQ(SDD(0, valref, SDD(1, valref, one)), intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops;

    conf::Values valx {0,1};
    ops.add(SDD(0, valx, SDD(1, valx, one)));

    conf::Values valy {2,3};
    ops.add(SDD(0, valy, SDD(1, valy, one)));

    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops;

    conf::Values val0x {0,1};
    conf::Values val1x {2,3};
    ops.add(SDD(0, val0x, SDD(1, val1x, one)));

    conf::Values val0y {1,2};
    conf::Values val1y {3,4};
    ops.add(SDD(0, val0y, SDD(1, val1y, one)));

    conf::Values val0ref {1};
    conf::Values val1ref {3};

    ASSERT_EQ(SDD(0, val0ref, SDD(1, val1ref, one)), intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops;

    conf::Values val0x {0,1};
    conf::Values val1x {2,3};
    ops.add(SDD(0, val0x, SDD(1, val1x, one)));

    conf::Values val0y {2,3};
    conf::Values val1y {3,4};

    ops.add(SDD(0, val0y, SDD(1, val1y, one)));

    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops;

    conf::Values val0x {0,1};
    conf::Values val1x {2,3};

    ops.add(SDD(0, val0x, SDD(1, val1x, one)));
    conf::Values val0y {1,2};
    conf::Values val1y {4,5};
    ops.add(SDD(0, val0y, SDD(1, val1y, one)));

    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(intersection_test, flat_nary)
{
  {
    sdd::dd::intersection_builder<conf, SDD> ops;

    conf::Values valx {0,1,2};
    ops.add(SDD(0, valx, one));

    conf::Values valy {1,2,3};
    ops.add(SDD(0, valy, one));

    conf::Values valz {2,3,4};
    ops.add(SDD(0, valz, one));

    conf::Values valref {2};

    ASSERT_EQ(SDD(0, valref, one), intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops;
    
    conf::Values valx {0,1,2};
    ops.add(SDD(0, valx, SDD(1, valx, one)));
    
    conf::Values valy {1,2,3};
    ops.add(SDD(0, valy, SDD(1, valy, one)));
    
    conf::Values valz {2,3,4};
    ops.add(SDD(0, valz, SDD(1, valz, one)));
    
    conf::Values valref {2};
    
    ASSERT_EQ( SDD(0, valref, SDD(1, valref, one))
             , intersection(cxt, std::move(ops)));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(intersection_test, hierarchical_nary)
{
  {
    sdd::dd::intersection_builder<conf, SDD> ops;
    ops.add(SDD('a', SDD('b', {0,1,2}, one), one));
    ops.add(SDD('a', SDD('b', {1,2,3}, one), one));
    ops.add(SDD('a', SDD('b', {2,3,4}, one), one));
    ops.add(SDD('a', SDD('b', {2,3,5}, one), one));
    ASSERT_EQ(SDD('a', SDD('b', {2}, one), one), intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops;
    ops.add(SDD('a', SDD('b', {0,1,2}, one), SDD('a', SDD('b', {0,1,2}, one), one)));
    ops.add(SDD('a', SDD('b', {1,2,3}, one), SDD('a', SDD('b', {1,2,3}, one), one)));
    ops.add(SDD('a', SDD('b', {2,3,4}, one), SDD('a', SDD('b', {2,3,4}, one), one)));
    ASSERT_EQ( SDD('a', SDD('b', {2}, one), SDD('a', SDD('b', {2}, one), one))
             , intersection(cxt, std::move(ops)));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(intersection_test, hierarchical_x_inter_y)
{
  {
    sdd::dd::intersection_builder<conf, SDD> ops;

    conf::Values valx {0,1};
    ops.add(SDD(10, SDD(0, valx, one), one));

    conf::Values valy {1,2};
    ops.add(SDD(10, SDD(0, valy, one), one));

    conf::Values valref {1};

    ASSERT_EQ( SDD(10, SDD(0, valref, one), one)
             , intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops;

    conf::Values valx {0,1};
    ops.add(SDD(10, SDD(0, valx, one), one));

    conf::Values valy {2,3};
    ops.add(SDD(10, SDD(0, valy, one), one));

    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops;

    conf::Values valx {0,1};
    SDD x(0, valx, one);
    ops.add(SDD(10, x, SDD(11, x, one)));

    conf::Values valy {1,2};
    SDD y(0, valy, one);
    ops.add(SDD(10, y, SDD(11, y, one)));

    conf::Values valref {1};
    SDD ref(0, valref, one);

    ASSERT_EQ( SDD(10, ref, SDD(11, ref, one))
             , intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops;

    conf::Values valx {0,1};
    SDD x(0, valx, one);
    ops.add(SDD(10, x, SDD(11, x, one)));

    conf::Values valy {2,3};
    SDD y(0, valy, one);
    ops.add(SDD(10, y, SDD(11, y, one)));

    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops;

    conf::Values val10x {0,1};
    conf::Values val11x {2,3};
    SDD x10(0, val10x, one);
    SDD x11(0, val11x, one);
    ops.add(SDD(10, x10, SDD(11, x11, one)));

    conf::Values val10y {1,2};
    conf::Values val11y {3,4};
    SDD y10(0, val10y, one);
    SDD y11(0, val11y, one);
    ops.add(SDD(10, y10, SDD(11, y11, one)));

    conf::Values val10ref {1};
    conf::Values val11ref {3};
    SDD ref10(0, val10ref, one);
    SDD ref11(0, val11ref, one);

    ASSERT_EQ( SDD(10, ref10, SDD(11, ref11, one))
             , intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops;
    
    conf::Values val10x {0,1};
    conf::Values val11x {2,3};
    SDD x10(0, val10x, one);
    SDD x11(0, val11x, one);
    ops.add(SDD(10, x10, SDD(11, x11, one)));
    
    conf::Values val10y {2,3};
    conf::Values val11y {3,4};
    SDD y10(0, val10y, one);
    SDD y11(0, val11y, one);
    ops.add(SDD(10, y10, SDD(11, y11, one)));
    
    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops;
    
    conf::Values val10x;
    val10x.insert(0);
    val10x.insert(1);
    conf::Values val11x;
    val11x.insert(2);
    val11x.insert(3);
    SDD x10(0, val10x, one);
    SDD x11(0, val11x, one);
    ops.add(SDD(10, x10, SDD(11, x11, one)));

    conf::Values val10y {1,2};
    conf::Values val11y {4,5};
    SDD y10(0, val10y, one);
    SDD y11(0, val11y, one);
    ops.add(SDD(10, y10, SDD(11, y11, one)));
    
    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(intersection_test, values)
{
  {
    sdd::dd::intersection_builder<conf, conf::Values> ops;
    ASSERT_EQ(conf::Values(), intersection(cxt, std::move(ops)));
  }
  {
    conf::Values val0 {0,1,2};
    conf::Values val1 {1,2};
    conf::Values val2 {2};
    conf::Values ref {2};
    sdd::dd::intersection_builder<conf, conf::Values> ops {val0, val1, val2};
    ASSERT_EQ(ref, intersection(cxt, std::move(ops)));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(intersection_test, iterable)
{
  std::vector<SDD> vec { SDD('a', {0,1}, one)
                       , SDD('a', {0,2}, one)
                       , SDD('a', {0,1}, one)
                       , SDD('a', {0,1,2}, one)};
  ASSERT_EQ(SDD('a', {0}, one), sdd::intersection<conf>(vec.begin(), vec.end()));
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(intersection_test, initializer_list)
{
  ASSERT_EQ(SDD('a', {0}, one), sdd::intersection({ SDD('a', {0,1}, one)
                                                  , SDD('a', {0,2}, one)
                                                  , SDD('a', {0,1}, one)
                                                  , SDD('a', {0,1,2}, one)}));
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(intersection_test, operators)
{
  ASSERT_EQ(zero, SDD('a', {0}, one) & SDD('a', {1}, one));
  ASSERT_EQ(SDD('a', {1}, one), SDD('a', {0,1}, one) & SDD('a', {1,2}, one));

  SDD x('a', {0,1}, one);
  x &= SDD('a', {1}, one);
  ASSERT_EQ(SDD('a', {1}, one), x);
}

/*------------------------------------------------------------------------------------------------*/
