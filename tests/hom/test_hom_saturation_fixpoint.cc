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
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_saturation_fixpoint_test()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::id<C>())
  {}
};


/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_saturation_fixpoint_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_saturation_fixpoint_test, construction)
{
  {
    std::vector<homomorphism> g {id, inductive<conf>(targeted_noop<conf>("0"))};
    ASSERT_EQ( saturation_fixpoint<conf>(0, id, g.begin(), g.end(), id)
             , saturation_fixpoint<conf>(0, id, g.begin(), g.end(), id));
  }
  {
    std::vector<homomorphism> g1 {id, inductive<conf>(targeted_noop<conf>("0"))};
    std::vector<homomorphism> g2 {id, inductive<conf>(targeted_noop<conf>("2"))};
    ASSERT_NE( saturation_fixpoint<conf>(0, id, g1.begin(), g1.end(), id)
             , saturation_fixpoint<conf>(0, id, g2.begin(), g2.end(), id));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_saturation_fixpoint_test, evaluation)
{
  {
    const order o(order_builder {"a", "b", "c"});
    SDD s0(2, {0}, SDD(1, {0}, SDD(0, {0}, one)));

    const auto f = fixpoint(sum<conf>(o, {inductive<conf>(targeted_incr<conf>("c", 1)), id}));
    const std::vector<homomorphism> g{inductive<conf>(targeted_incr<conf>("b", 2))};
    const auto h = saturation_fixpoint(1, f, g.begin(), g.end(), id);

    const auto r = fixpoint(sum<conf>(o, { inductive<conf>(targeted_incr<conf>("c", 1))
                                        , inductive<conf>(targeted_incr<conf>("b", 2))
                                        , id}));
    ASSERT_EQ(r(o, s0), h(o, s0));
    ASSERT_EQ( SDD(2, {0}, SDD(1, {0,2}, SDD(0, {0,1,2}, one)))
             , h(o, s0));
  }
  {
    order o(order_builder().push("c").push("b", order_builder {"x"}).push("a"));
    SDD s0(2, {0}, SDD(1, SDD(0, {0}, one), SDD(0, {0}, one)));

    const auto f = fixpoint(sum<conf>(o, {inductive<conf>(targeted_incr<conf>("c", 1)), id}));
    const std::vector<homomorphism> g{};
    const auto l = local("b", o, sum<conf>(o, {inductive<conf>(targeted_incr<conf>("x", 2)), id}));
    const auto h = saturation_fixpoint(1, f, g.begin(), g.end(), l);

    const auto r = fixpoint(sum<conf>(o, { inductive<conf>(targeted_incr<conf>("c", 1))
                                        , local("b", o, inductive<conf>(targeted_incr<conf>("x", 2)))
                                        , id}));
    ASSERT_EQ(r(o, s0), h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
