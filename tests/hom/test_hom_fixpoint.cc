#include "gtest/gtest.h"

#include "tests/hom/common.hh"
#include "tests/hom/common_inductives.hh"

/*-------------------------------------------------------------------------------------------*/

const SDD one = sdd::one<conf>();
const hom id = sdd::hom::Id<conf>();

struct hom_fixpoint_test
  : public testing::Test
{
};

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_fixpoint_test, construction)
{
  {
    ASSERT_EQ(id, Fixpoint(id));
  }
  {
    ASSERT_EQ( Fixpoint(Inductive<conf>(targeted_incr(0,1)))
             , Fixpoint(Inductive<conf>(targeted_incr(0,1)))
             );
  }
  {
    ASSERT_NE( Fixpoint(Inductive<conf>(targeted_incr(0,1)))
             , Fixpoint(Inductive<conf>(targeted_incr(0,2)))
             );
  }
  {
    ASSERT_EQ( Fixpoint(Inductive<conf>(targeted_incr(0,1)))
             , Fixpoint(Fixpoint(Inductive<conf>(targeted_incr(0,1))))
             );
  }
  {
    ASSERT_EQ( Fixpoint(Local<conf>(0, Inductive<conf>(targeted_incr(0,1))))
             , Local<conf>(0, Fixpoint(Inductive<conf>(targeted_incr(0,1))))
             );
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_fixpoint_test, evaluation)
{
  SDD s0(0, {0}, one);
  hom h0 = Fixpoint(Sum({Inductive<conf>(targeted_incr(0, 1)), id}));
  ASSERT_EQ(SDD(0, {0,1,2}, one), h0(s0));
}

/*-------------------------------------------------------------------------------------------*/
