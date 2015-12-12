#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/manager.hh"

#include "tests/configuration.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct top_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;

  top_test()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(top_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(top_test, sum_terminal)
{
  {
    SDD x('a', {0}, one);
    ASSERT_THROW(x + one, sdd::top<conf>);
    try
    {
      x + one;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x('a', {0}, one);
    ASSERT_THROW(one + x, sdd::top<conf>);
    try
    {
      one + x;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x('a', SDD('b', {1}, one), one);
    ASSERT_THROW(x + one, sdd::top<conf>);
    try
    {
      x + one;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x('a', SDD('b', {1}, one), one);
    SDD y('a', one, one);
    ASSERT_THROW(x + y, sdd::top<conf>);
    try
    {
      x + y;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(top_test, sum)
{
  {
    SDD x('a', {0}, one);
    SDD y('b', {1}, one);
    ASSERT_THROW(x + y, sdd::top<conf>);
    try
    {
      x + y;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x('a', {0}, SDD('b', {0}, one));
    SDD y('a', {0}, SDD('a', {1}, one));
    ASSERT_THROW(x + y, sdd::top<conf>);
    try
    {
      x + y;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x1('x', SDD('a', {0}, one), one);
    SDD x2('x', SDD('b', {0}, one), one);
    ASSERT_THROW(x1 + x2, sdd::top<conf>);
    try
    {
      x1 + x2;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x1('x', one, one);
    SDD x2('y', one, one);
    ASSERT_THROW(x1 + x2, sdd::top<conf>);
    try
    {
      x1 + x2;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(top_test, sum_different_node_types)
{
  SDD x('a', {0}, one);
  SDD y('a', SDD('b', {0}, one), one);

  ASSERT_THROW(x + y, sdd::top<conf>);
  try
  {
    x + y;
  }
  catch(sdd::top<conf>& t)
  {
    ASSERT_NE(t.lhs(), t.rhs());
    ASSERT_NE(nullptr, t.what());
  }

  ASSERT_THROW(y + x, sdd::top<conf>);
  try
  {
    y + x;
  }
  catch(sdd::top<conf>& t)
  {
    ASSERT_NE(t.lhs(), t.rhs());
    ASSERT_NE(nullptr, t.what());
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(top_test, difference_terminal)
{
  {
    SDD x('a', {0}, one);
    ASSERT_THROW(x - one, sdd::top<conf>);
    try
    {
      x - one;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x('a', {0}, one);
    ASSERT_THROW(one - x, sdd::top<conf>);
    try
    {
      one - x;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x('a', SDD('b', {1}, one), one);
    ASSERT_THROW(x - one, sdd::top<conf>);
    try
    {
      x - one;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x('a', SDD('b', {1}, one), one);
    SDD y('a', one, one);
    ASSERT_THROW(x - y, sdd::top<conf>);
    try
    {
      x - y;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
}


/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(top_test, difference)
{
  {
    SDD x('a', {0}, one);
    SDD y('b', {1}, one);
    ASSERT_THROW(x - y, sdd::top<conf>);
    try
    {
      x - y;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x('a', {0,1}, SDD('b', {0}, one));
    SDD y('a', {1}, SDD('a', {1}, one));
    ASSERT_THROW(x - y, sdd::top<conf>);
    try
    {
      x - y;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x1('x', SDD('a', {0}, one), one);
    SDD x2('x', SDD('b', {0}, one), one);
    ASSERT_THROW(x1 - x2, sdd::top<conf>);
    try
    {
      x1 - x2;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x1('x', one, one);
    SDD x2('y', one, one);
    ASSERT_THROW(x1 - x2, sdd::top<conf>);
    try
    {
      x1 - x2;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(top_test, difference_different_node_types)
{

  SDD x('a', {0}, one);
  SDD y('a', SDD('b', {0}, one), one);

  ASSERT_THROW(x - y, sdd::top<conf>);
  try
  {
    x - y;
  }
  catch(sdd::top<conf>& t)
  {
    ASSERT_NE(t.lhs(), t.rhs());
    ASSERT_NE(nullptr, t.what());
  }

  ASSERT_THROW(y - x, sdd::top<conf>);
  try
  {
    y - x;
  }
  catch(sdd::top<conf>& t)
  {
    ASSERT_NE(t.lhs(), t.rhs());
    ASSERT_NE(nullptr, t.what());
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(top_test, intersection_terminal)
{
  {
    SDD x('a', {0}, one);
    ASSERT_THROW(x & one, sdd::top<conf>);
    try
    {
      x & one;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x('a', {0}, one);
    ASSERT_THROW(one & x, sdd::top<conf>);
    try
    {
      one & x;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x('a', SDD('b', {1}, one), one);
    ASSERT_THROW(x & one, sdd::top<conf>);
    try
    {
      x & one;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x('a', SDD('b', {1}, one), one);
    SDD y('a', one, one);
    ASSERT_THROW(x & y, sdd::top<conf>);
    try
    {
      x & y;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
}


/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(top_test, intersection)
{
  {
    SDD x('a', {0}, one);
    SDD y('b', {1}, one);
    ASSERT_THROW(x - y, sdd::top<conf>);
    try
    {
      x & y;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x('a', {0,1}, SDD('b', {0}, one));
    SDD y('a', {1}, SDD('a', {1}, one));
    ASSERT_THROW(x & y, sdd::top<conf>);
    try
    {
      x & y;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x1('x', SDD('a', {0}, one), one);
    SDD x2('x', SDD('b', {0}, one), one);
    ASSERT_THROW(x1 & x2, sdd::top<conf>);
    try
    {
      x1 & x2;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
  {
    SDD x1('x', one, one);
    SDD x2('y', one, one);
    ASSERT_THROW(x1 & x2, sdd::top<conf>);
    try
    {
      x1 & x2;
    }
    catch(sdd::top<conf>& t)
    {
      ASSERT_NE(t.lhs(), t.rhs());
      ASSERT_NE(nullptr, t.what());
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(top_test, intersection_different_node_types)
{

  SDD x('a', {0}, one);
  SDD y('a', SDD('b', {0}, one), one);

  ASSERT_THROW(x & y, sdd::top<conf>);
  try
  {
    x & y;
  }
  catch(sdd::top<conf>& t)
  {
    ASSERT_NE(t.lhs(), t.rhs());
    ASSERT_NE(nullptr, t.what());
  }

  ASSERT_THROW(y & x, sdd::top<conf>);
  try
  {
    y & x;
  }
  catch(sdd::top<conf>& t)
  {
    ASSERT_NE(t.lhs(), t.rhs());
    ASSERT_NE(nullptr, t.what());
  }
}

/*------------------------------------------------------------------------------------------------*/
