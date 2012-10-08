#include "gtest/gtest.h"

#include "tests/hom/common.hh"
#include "tests/hom/common_inductives.hh"

/*-------------------------------------------------------------------------------------------*/

const SDD zero = sdd::zero<conf>();
const SDD one = sdd::one<conf>();
const hom id = sdd::hom::Id<conf>();

struct hom_composition_test
  : public testing::Test
{
};

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_composition_test, construction)
{
  {
    ASSERT_EQ(id, Composition(id, id));
  }
  {
    const hom h = Inductive<conf>(incr(0,1));
    ASSERT_EQ(h, Composition(id, h));
    ASSERT_EQ(h, Composition(h, id));
  }
  {
    const hom h1 = Inductive<conf>(incr(0,1));
    const hom h2 = Inductive<conf>(incr(0,2));
    ASSERT_EQ(Composition(h1, h2), Composition(h1, h2));
    ASSERT_NE(Composition(h2, h1), Composition(h1, h2));
    ASSERT_NE(Composition(h1, h2), Composition(h2, h1));
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_composition_test, evaluation)
{
  {
    const hom h1 = Inductive<conf>(incr(0,1));
    const hom h2 = Inductive<conf>(incr(0,1));
    const hom c0 = Composition(h1, h2);
    SDD s0(0, {0}, one);
    SDD s1(0, {2}, one);
    ASSERT_EQ(s1, c0(s0));
  }
}

/*-------------------------------------------------------------------------------------------*/
