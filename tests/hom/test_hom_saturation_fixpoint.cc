#include <vector>

#include "gtest/gtest.h"

#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"
#include "sdd/hom/rewrite.hh"
#include "sdd/manager.hh"
#include "sdd/order/order.hh"

#include "tests/configuration.hh"
#include "tests/hom/common.hh"
#include "tests/hom/common_inductives.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct hom_saturation_fixpoint_test
  : public testing::Test
{
  typedef C configuration_type;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_saturation_fixpoint_test()
    : m(sdd::manager<C>::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::Id<C>())
  {}
};


/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_saturation_fixpoint_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_saturation_fixpoint_test, construction)
{
  {
    std::vector<homomorphism> g {id, Inductive<conf>(targeted_noop<conf>("0"))};
    ASSERT_EQ( SaturationFixpoint<conf>(0, id, g.begin(), g.end(), id)
             , SaturationFixpoint<conf>(0, id, g.begin(), g.end(), id));
  }
  {
    std::vector<homomorphism> g1 {id, Inductive<conf>(targeted_noop<conf>("0"))};
    std::vector<homomorphism> g2 {id, Inductive<conf>(targeted_noop<conf>("2"))};
    ASSERT_NE( SaturationFixpoint<conf>(0, id, g1.begin(), g1.end(), id)
             , SaturationFixpoint<conf>(0, id, g2.begin(), g2.end(), id));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_saturation_fixpoint_test, evaluation)
{
  {
    const order o(order_builder {"a", "b", "c"});
    SDD s0(2, {0}, SDD(1, {0}, SDD(0, {0}, one)));

    const auto f = Fixpoint(Sum<conf>(o, {Inductive<conf>(targeted_incr<conf>("c", 1)), id}));
    const std::vector<homomorphism> g{Inductive<conf>(targeted_incr<conf>("b", 2))};
    const auto h = SaturationFixpoint(1, f, g.begin(), g.end(), id);

    const auto r = Fixpoint(Sum<conf>(o, { Inductive<conf>(targeted_incr<conf>("c", 1))
                                        , Inductive<conf>(targeted_incr<conf>("b", 2))
                                        , id}));
    ASSERT_EQ(r(o, s0), h(o, s0));
    ASSERT_EQ( SDD(2, {0}, SDD(1, {0,2}, SDD(0, {0,1,2}, one)))
             , h(o, s0));
  }
  {
    order o(order_builder().add("c").add("b", order_builder {"x"}).add("a"));
    SDD s0(2, {0}, SDD(1, SDD(0, {0}, one), SDD(0, {0}, one)));

    const auto f = Fixpoint(Sum<conf>(o, {Inductive<conf>(targeted_incr<conf>("c", 1)), id}));
    const std::vector<homomorphism> g{};
    const auto l = Local("b", o, Sum<conf>(o, {Inductive<conf>(targeted_incr<conf>("x", 2)), id}));
    const auto h = SaturationFixpoint(1, f, g.begin(), g.end(), l);

    const auto r = Fixpoint(Sum<conf>(o, { Inductive<conf>(targeted_incr<conf>("c", 1))
                                        , Local("b", o, Inductive<conf>(targeted_incr<conf>("x", 2)))
                                        , id}));
    ASSERT_EQ(r(o, s0), h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
