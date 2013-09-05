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
  typedef C configuration_type;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;

  order_carrier_test()
    : m(sdd::manager<C>::init(small_conf<C>()))
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
  ob.add("a");
  ob.add("b");
  ob.add("c", order_builder().add("d").add("e", order_builder().add("f").add("g")));
  order o (ob);
  {
    const auto h = Inductive<conf>(targeted_incr<conf>("a",1));
    ASSERT_EQ(h, carrier(o, "a", h));
  }
  {
    const auto h = Inductive<conf>(targeted_incr<conf>("d",1));
    const auto c = carrier(o, "d", h);
    const auto r = Local("c", o, h);
    ASSERT_EQ(r, c);
  }
  {
    const auto h = Inductive<conf>(targeted_incr<conf>("g",1));
    const auto c = carrier(o, "g", h);
    const auto r = Local("c", o, Local("e", o, h));
    ASSERT_EQ(r, c);
  }
  {
    const auto h = Inductive<conf>(targeted_incr<conf>("f",1));
    const auto c = carrier(o, "f", h);
    const auto r = Local("c", o, Local("e", o, h));
    ASSERT_EQ(r, c);
  }
}

/*------------------------------------------------------------------------------------------------*/
