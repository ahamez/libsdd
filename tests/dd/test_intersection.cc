#include <vector>

#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/manager.hh"

#include "tests/configuration.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct intersection_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;
  sdd::dd::context<C>& cxt;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;

  intersection_test()
    : m(sdd::init(small_conf<C>()))
    , cxt(sdd::global<C>().sdd_context)
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(intersection_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(intersection_test, empty_operand)
{
  ASSERT_EQ(zero, intersection(cxt, {cxt, {SDD('a', values_type(), one)}}));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(intersection_test, empty_intersection)
{
  values_type val {0};
  sdd::dd::intersection_builder<conf, SDD> ops(cxt);
  ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(intersection_test, flat_x_inter_zero)
{
  {
    values_type val {0};
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);
    ops.add(SDD(0, val, one));
    ops.add(zero);
    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    values_type val {0,1,2,3};
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);
    ops.add(zero);
    ops.add(SDD(0, val, one));
    ops.add(SDD(0, val, one));
    ops.add(SDD(0, val, one));
    ops.add(SDD(0, val, one));
    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    values_type val {0};
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);
    ops.add(SDD(0, val, SDD(0, val, one)));
    ops.add(zero);
    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    values_type val {0,1,2,3};
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);
    ops.add(SDD(0, val, SDD(0, val, one)));
    ops.add(SDD(0, val, SDD(0, val, one)));
    ops.add(SDD(0, val, SDD(0, val, one)));
    ops.add(SDD(0, val, SDD(0, val, one)));
    ops.add(zero);
    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    values_type val {0};    
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);
    ops.add(zero);
    ops.add(zero);
    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(intersection_test, flat_x_inter_y)
{
  {
    SDD x(0, {0,1}, one);
    SDD y(0, {1,2}, one);
    SDD ref(0, {1}, one);
    ASSERT_EQ(ref, intersection(cxt, {cxt, {x,y}}));

    sdd::dd::sum_builder<conf, SDD> sum_ops_x(cxt);
    sum_ops_x.add(ref);
    sum_ops_x.add(x);
    ASSERT_EQ(x, sum(cxt, std::move(sum_ops_x)));

    sdd::dd::sum_builder<conf, SDD> sum_ops_y(cxt);
    sum_ops_y.add(ref);
    sum_ops_y.add(y);
    ASSERT_EQ(y, sum(cxt, std::move(sum_ops_y)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);

    values_type valx {0,1};
    ops.add(SDD(0, valx, one));

    values_type valy {2,3};
    ops.add(SDD(0, valy, one));

    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);

    values_type valx {0,1};
    ops.add(SDD(0, valx, SDD(1, valx, one)));

    values_type valy {1,2};
    ops.add(SDD(0, valy, SDD(1, valy, one)));

    values_type valref {1};

    ASSERT_EQ(SDD(0, valref, SDD(1, valref, one)), intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);

    values_type valx {0,1};
    ops.add(SDD(0, valx, SDD(1, valx, one)));

    values_type valy {2,3};
    ops.add(SDD(0, valy, SDD(1, valy, one)));

    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);

    values_type val0x {0,1};
    values_type val1x {2,3};
    ops.add(SDD(0, val0x, SDD(1, val1x, one)));

    values_type val0y {1,2};
    values_type val1y {3,4};
    ops.add(SDD(0, val0y, SDD(1, val1y, one)));

    values_type val0ref {1};
    values_type val1ref {3};

    ASSERT_EQ(SDD(0, val0ref, SDD(1, val1ref, one)), intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);

    values_type val0x {0,1};
    values_type val1x {2,3};
    ops.add(SDD(0, val0x, SDD(1, val1x, one)));

    values_type val0y {2,3};
    values_type val1y {3,4};

    ops.add(SDD(0, val0y, SDD(1, val1y, one)));

    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);

    values_type val0x {0,1};
    values_type val1x {2,3};

    ops.add(SDD(0, val0x, SDD(1, val1x, one)));
    values_type val0y {1,2};
    values_type val1y {4,5};
    ops.add(SDD(0, val0y, SDD(1, val1y, one)));

    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(intersection_test, flat_nary)
{
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);

    values_type valx {0,1,2};
    ops.add(SDD(0, valx, one));

    values_type valy {1,2,3};
    ops.add(SDD(0, valy, one));

    values_type valz {2,3,4};
    ops.add(SDD(0, valz, one));

    values_type valref {2};

    ASSERT_EQ(SDD(0, valref, one), intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);
    
    values_type valx {0,1,2};
    ops.add(SDD(0, valx, SDD(1, valx, one)));
    
    values_type valy {1,2,3};
    ops.add(SDD(0, valy, SDD(1, valy, one)));
    
    values_type valz {2,3,4};
    ops.add(SDD(0, valz, SDD(1, valz, one)));
    
    values_type valref {2};
    
    ASSERT_EQ( SDD(0, valref, SDD(1, valref, one))
             , intersection(cxt, std::move(ops)));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(intersection_test, hierarchical_nary)
{
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);
    ops.add(SDD('a', SDD('b', {0,1,2}, one), one));
    ops.add(SDD('a', SDD('b', {1,2,3}, one), one));
    ops.add(SDD('a', SDD('b', {2,3,4}, one), one));
    ops.add(SDD('a', SDD('b', {2,3,5}, one), one));
    ASSERT_EQ(SDD('a', SDD('b', {2}, one), one), intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);
    ops.add(SDD('a', SDD('b', {0,1,2}, one), SDD('a', SDD('b', {0,1,2}, one), one)));
    ops.add(SDD('a', SDD('b', {1,2,3}, one), SDD('a', SDD('b', {1,2,3}, one), one)));
    ops.add(SDD('a', SDD('b', {2,3,4}, one), SDD('a', SDD('b', {2,3,4}, one), one)));
    ASSERT_EQ( SDD('a', SDD('b', {2}, one), SDD('a', SDD('b', {2}, one), one))
             , intersection(cxt, std::move(ops)));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(intersection_test, hierarchical_x_inter_y)
{
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);

    values_type valx {0,1};
    ops.add(SDD(10, SDD(0, valx, one), one));

    values_type valy {1,2};
    ops.add(SDD(10, SDD(0, valy, one), one));

    values_type valref {1};

    ASSERT_EQ( SDD(10, SDD(0, valref, one), one)
             , intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);

    values_type valx {0,1};
    ops.add(SDD(10, SDD(0, valx, one), one));

    values_type valy {2,3};
    ops.add(SDD(10, SDD(0, valy, one), one));

    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);

    values_type valx {0,1};
    SDD x(0, valx, one);
    ops.add(SDD(10, x, SDD(11, x, one)));

    values_type valy {1,2};
    SDD y(0, valy, one);
    ops.add(SDD(10, y, SDD(11, y, one)));

    values_type valref {1};
    SDD ref(0, valref, one);

    ASSERT_EQ( SDD(10, ref, SDD(11, ref, one))
             , intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);

    values_type valx {0,1};
    SDD x(0, valx, one);
    ops.add(SDD(10, x, SDD(11, x, one)));

    values_type valy {2,3};
    SDD y(0, valy, one);
    ops.add(SDD(10, y, SDD(11, y, one)));

    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);

    values_type val10x {0,1};
    values_type val11x {2,3};
    SDD x10(0, val10x, one);
    SDD x11(0, val11x, one);
    ops.add(SDD(10, x10, SDD(11, x11, one)));

    values_type val10y {1,2};
    values_type val11y {3,4};
    SDD y10(0, val10y, one);
    SDD y11(0, val11y, one);
    ops.add(SDD(10, y10, SDD(11, y11, one)));

    values_type val10ref {1};
    values_type val11ref {3};
    SDD ref10(0, val10ref, one);
    SDD ref11(0, val11ref, one);

    ASSERT_EQ( SDD(10, ref10, SDD(11, ref11, one))
             , intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);
    
    values_type val10x {0,1};
    values_type val11x {2,3};
    SDD x10(0, val10x, one);
    SDD x11(0, val11x, one);
    ops.add(SDD(10, x10, SDD(11, x11, one)));
    
    values_type val10y {2,3};
    values_type val11y {3,4};
    SDD y10(0, val10y, one);
    SDD y11(0, val11y, one);
    ops.add(SDD(10, y10, SDD(11, y11, one)));
    
    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
  {
    sdd::dd::intersection_builder<conf, SDD> ops(cxt);
    
    values_type val10x;
    val10x.insert(0);
    val10x.insert(1);
    values_type val11x;
    val11x.insert(2);
    val11x.insert(3);
    SDD x10(0, val10x, one);
    SDD x11(0, val11x, one);
    ops.add(SDD(10, x10, SDD(11, x11, one)));

    values_type val10y {1,2};
    values_type val11y {4,5};
    SDD y10(0, val10y, one);
    SDD y11(0, val11y, one);
    ops.add(SDD(10, y10, SDD(11, y11, one)));
    
    ASSERT_EQ(zero, intersection(cxt, std::move(ops)));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(intersection_test, values)
{
  {
    sdd::dd::intersection_builder<conf, values_type> ops(cxt);
    ASSERT_EQ(values_type(), intersection(cxt, std::move(ops)));
  }
  {
    values_type val0 {0,1,2};
    values_type val1 {1,2};
    values_type val2 {2};
    values_type ref {2};
    sdd::dd::intersection_builder<conf, values_type> ops(cxt, {val0, val1, val2});
    ASSERT_EQ(ref, intersection(cxt, std::move(ops)));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(intersection_test, iterable)
{
  std::vector<SDD> vec { SDD('a', {0,1}, one)
                       , SDD('a', {0,2}, one)
                       , SDD('a', {0,1}, one)
                       , SDD('a', {0,1,2}, one)};
  ASSERT_EQ(SDD('a', {0}, one), sdd::intersection<conf>(vec.begin(), vec.end()));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(intersection_test, initializer_list)
{
  ASSERT_EQ(SDD('a', {0}, one), sdd::intersection({ SDD('a', {0,1}, one)
                                                  , SDD('a', {0,2}, one)
                                                  , SDD('a', {0,1}, one)
                                                  , SDD('a', {0,1,2}, one)}));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(intersection_test, operators)
{
  ASSERT_EQ(zero, SDD('a', {0}, one) & SDD('a', {1}, one));
  ASSERT_EQ(SDD('a', {1}, one), SDD('a', {0,1}, one) & SDD('a', {1,2}, one));

  SDD x('a', {0,1}, one);
  x &= SDD('a', {1}, one);
  ASSERT_EQ(SDD('a', {1}, one), x);
}

/*------------------------------------------------------------------------------------------------*/
