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
struct hom_saturation_sum_test
  : public testing::Test
{
  typedef C configuration_type;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_saturation_sum_test()
    : m(sdd::manager<C>::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::Id<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_saturation_sum_test, configurations);
#include "tests/macros.hh"
#define optional typename sdd::hom::saturation_sum<conf>::optional_type

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_saturation_sum_test, construction)
{
  {
    std::vector<homomorphism> g {id, Inductive<conf>(targeted_noop<conf>("0"))};
    ASSERT_EQ( SaturationSum<conf>(0, optional(), g.begin(), g.end(), optional())
             , SaturationSum<conf>(0, optional(), g.begin(), g.end(), optional()));
  }
  {
    std::vector<homomorphism> g1 {id, Inductive<conf>(targeted_noop<conf>("0"))};
    std::vector<homomorphism> g2 {id, Inductive<conf>(targeted_noop<conf>("2"))};
    ASSERT_NE( SaturationSum<conf>(0, optional(), g1.begin(), g1.end(), optional())
             , SaturationSum<conf>(0, optional(), g2.begin(), g2.end(), optional()));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_saturation_sum_test, evaluation)
{
  {
    const order o(order_builder {"a", "b", "c"});
    SDD s0(2, {0}, SDD(1, {0}, SDD(0, {0}, one)));
    std::vector<homomorphism> empty_g;
    const auto s = SaturationSum<conf>( 0, Inductive<conf>(targeted_incr<conf>("c", 1))
                                     , empty_g.begin(), empty_g.end(), optional());
    std::vector<homomorphism> g {Inductive<conf>(targeted_incr<conf>("b", 1))};
    const auto h = SaturationSum<conf>(1, s, g.begin(), g.end(), optional());
    SDD ref = SDD(2, {0}, SDD(1, {1}, SDD(0, {0}, one)))
            + SDD(2, {0}, SDD(1, {0}, SDD(0, {1}, one)));
    ASSERT_EQ(ref, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
