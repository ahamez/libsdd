#include <vector>

#include "gtest/gtest.h"

#include "tests/hom/common.hh"
#include "tests/hom/common_inductives.hh"

/*------------------------------------------------------------------------------------------------*/

const SDD one = sdd::one<conf>();
const hom id = sdd::Id<conf>();
typedef sdd::hom::saturation_sum<conf>::optional_type optional;

struct hom_saturation_sum_test
  : public testing::Test
{
};

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_saturation_sum_test, construction)
{
  {
    std::vector<hom> g {id, Inductive<conf>(targeted_noop("0"))};
    ASSERT_EQ( SaturationSum<conf>(0, optional(), g.begin(), g.end(), optional())
             , SaturationSum<conf>(0, optional(), g.begin(), g.end(), optional()));
  }
  {
    std::vector<hom> g1 {id, Inductive<conf>(targeted_noop("0"))};
    std::vector<hom> g2 {id, Inductive<conf>(targeted_noop("2"))};
    ASSERT_NE( SaturationSum<conf>(0, optional(), g1.begin(), g1.end(), optional())
             , SaturationSum<conf>(0, optional(), g2.begin(), g2.end(), optional()));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_saturation_sum_test, evaluation)
{
  {
    const order o(order_builder {"a", "b", "c"});
    SDD s0(2, {0}, SDD(1, {0}, SDD(0, {0}, one)));
    std::vector<hom> empty_g;
    const hom s = SaturationSum<conf>( 0, Inductive<conf>(targeted_incr("c", 1))
                                     , empty_g.begin(), empty_g.end(), optional());
    std::vector<hom> g {Inductive<conf>(targeted_incr("b", 1))};
    const hom h = SaturationSum<conf>(1, s, g.begin(), g.end(), optional());
    SDD ref = SDD(2, {0}, SDD(1, {1}, SDD(0, {0}, one)))
            + SDD(2, {0}, SDD(1, {0}, SDD(0, {1}, one)));
    ASSERT_EQ(ref, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
