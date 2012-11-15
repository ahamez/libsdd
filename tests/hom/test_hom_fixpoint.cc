#include "gtest/gtest.h"

#include "tests/hom/common.hh"
#include "tests/hom/common_inductives.hh"

/*------------------------------------------------------------------------------------------------*/

const SDD one = sdd::one<conf>();
const hom id = sdd::Id<conf>();

struct hom_fixpoint_test
  : public testing::Test
{
  hom_fixpoint_test()
  {
  }
};

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_fixpoint_test, construction)
{
  {
    ASSERT_EQ(id, Fixpoint(id));
  }
  {
    ASSERT_EQ( Fixpoint(Inductive<conf>(targeted_incr("0",1)))
             , Fixpoint(Inductive<conf>(targeted_incr("0",1)))
             );
  }
  {
    ASSERT_NE( Fixpoint(Inductive<conf>(targeted_incr("0",1)))
             , Fixpoint(Inductive<conf>(targeted_incr("0",2)))
             );
  }
  {
    ASSERT_EQ( Fixpoint(Inductive<conf>(targeted_incr("0",1)))
             , Fixpoint(Fixpoint(Inductive<conf>(targeted_incr("0",1))))
             );
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_fixpoint_test, evaluatione)
{
  {
    order o(order_builder {"0"});
    SDD s0(0, {0}, one);
    hom h0 = Fixpoint(Sum(o, {Inductive<conf>(targeted_incr("0", 1)), id}));
    ASSERT_EQ(SDD(0, {0,1,2}, one), h0(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
