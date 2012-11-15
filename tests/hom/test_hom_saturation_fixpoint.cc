#include <vector>

#include "gtest/gtest.h"

#include "tests/hom/common.hh"
#include "tests/hom/common_inductives.hh"

/*------------------------------------------------------------------------------------------------*/

const SDD one = sdd::one<conf>();
const hom id = sdd::Id<conf>();

struct hom_saturation_fixpoint_test
  : public testing::Test
{
  hom_saturation_fixpoint_test()
  {
  }
};

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_saturation_fixpoint_test, construction)
{
  {
    std::vector<hom> g {id, Inductive<conf>(targeted_noop("0"))};
    ASSERT_EQ( SaturationFixpoint<conf>(0, id, g.begin(), g.end(), id)
             , SaturationFixpoint<conf>(0, id, g.begin(), g.end(), id));
  }
  {
    std::vector<hom> g1 {id, Inductive<conf>(targeted_noop("0"))};
    std::vector<hom> g2 {id, Inductive<conf>(targeted_noop("2"))};
    ASSERT_NE( SaturationFixpoint<conf>(0, id, g1.begin(), g1.end(), id)
             , SaturationFixpoint<conf>(0, id, g2.begin(), g2.end(), id));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_saturation_fixpoint_test, evaluation)
{
  {
    const order o(order_builder {"a", "b", "c"});
    SDD s0(2, {0}, SDD(1, {0}, SDD(0, {0}, one)));

    const hom f = Fixpoint(Sum<conf>(o, {Inductive<conf>(targeted_incr("c", 1)), id}));
    const std::vector<hom> g{Inductive<conf>(targeted_incr("b", 2))};
    const hom h = SaturationFixpoint(1, f, g.begin(), g.end(), id);

    const hom r = Fixpoint(Sum<conf>(o, { Inductive<conf>(targeted_incr("c", 1))
                                        , Inductive<conf>(targeted_incr("b", 2))
                                        , id}));
    ASSERT_EQ(r(o, s0), h(o, s0));
    ASSERT_EQ( SDD(2, {0}, SDD(1, {0,2}, SDD(0, {0,1,2}, one)))
             , h(o, s0));
  }
  {
    order o(order_builder().add("c").add("b", order_builder {"x"}).add("a"));
    SDD s0(2, {0}, SDD(1, SDD(0, {0}, one), SDD(0, {0}, one)));

    const hom f = Fixpoint(Sum<conf>(o, {Inductive<conf>(targeted_incr("c", 1)), id}));
    const std::vector<hom> g{};
    const hom l = Local("b", o, Sum<conf>(o, {Inductive<conf>(targeted_incr("x", 2)), id}));
    const hom h = SaturationFixpoint(1, f, g.begin(), g.end(), l);

    const hom r = Fixpoint(Sum<conf>(o, { Inductive<conf>(targeted_incr("c", 1))
                                        , Local("b", o, Inductive<conf>(targeted_incr("x", 2)))
                                        , id}));
    ASSERT_EQ(r(o, s0), h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
