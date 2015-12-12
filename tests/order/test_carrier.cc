#include "gtest/gtest.h"

#include "sdd/manager.hh"
#include "sdd/order/carrier.hh"

#include "tests/configuration.hh"
#include "tests/hom/common.hh"
#include "tests/hom/common_inductives.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct order_carrier_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;

  order_carrier_test()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(order_carrier_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(order_carrier_test, inductive)
{
  order_builder ob;
  ob.push("a");
  ob.push("b");
  ob.push("c", order_builder().push("d").push("e", order_builder().push("f").push("g")));
  order o (ob);
  {
    const auto h = inductive<conf>(targeted_incr<conf>("a",1));
    ASSERT_EQ(h, carrier(o, "a", h));
  }
  {
    const auto h = inductive<conf>(targeted_incr<conf>("d",1));
    const auto c = carrier(o, "d", h);
    const auto r = local("c", o, h);
    ASSERT_EQ(r, c);
  }
  {
    const auto h = inductive<conf>(targeted_incr<conf>("g",1));
    const auto c = carrier(o, "g", h);
    const auto r = local("c", o, local("e", o, h));
    ASSERT_EQ(r, c);
  }
  {
    const auto h = inductive<conf>(targeted_incr<conf>("f",1));
    const auto c = carrier(o, "f", h);
    const auto r = local("c", o, local("e", o, h));
    ASSERT_EQ(r, c);
  }
}

/*------------------------------------------------------------------------------------------------*/
