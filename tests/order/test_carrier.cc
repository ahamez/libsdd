#include "gtest/gtest.h"

#include "tests/hom/common_inductives.hh"
#include "sdd/order/carrier.hh"

/*------------------------------------------------------------------------------------------------*/

struct carrier_test
  : public testing::Test
{
  typedef sdd::conf0 conf;
  typedef sdd::order_builder<conf> order_builder;
  typedef sdd::order<conf> order;
  typedef sdd::SDD<conf> SDD;
  typedef sdd::homomorphism<conf> hom;

  sdd::manager<conf> m;

  const SDD zero;
  const SDD one;
  const hom id;

  carrier_test()
    : m(sdd::manager<conf>::init(small_conf()))
    , zero(sdd::zero<conf>())
    , one(sdd::one<conf>())
    , id(sdd::Id<conf>())
  {
  }
};

/*------------------------------------------------------------------------------------------------*/

TEST_F(carrier_test, order_builder)
{
  order_builder ob;
  ob.add("a");
  ob.add("b");
  ob.add("c", order_builder().add("d").add("e", order_builder().add("f").add("g")));
  order o (ob);
  {
    const auto h = Inductive<conf>(targeted_incr("a",1));
    ASSERT_EQ(h, carrier(o, "a", h));
  }
  {
    const auto h = Inductive<conf>(targeted_incr("d",1));
    const auto c = carrier(o, "d", h);
    const auto r = sdd::Local("c", o, h);
    ASSERT_EQ(r, c);
  }
  {
    const auto h = Inductive<conf>(targeted_incr("g",1));
    const auto c = carrier(o, "g", h);
    const auto r = sdd::Local("c", o, sdd::Local("e", o, h));
    ASSERT_EQ(r, c);
  }
  {
    const auto h = Inductive<conf>(targeted_incr("f",1));
    const auto c = carrier(o, "f", h);
    const auto r = sdd::Local("c", o, sdd::Local("e", o, h));
    ASSERT_EQ(r, c);
  }
}

/*------------------------------------------------------------------------------------------------*/
