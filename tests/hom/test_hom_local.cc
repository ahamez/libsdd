#include "gtest/gtest.h"

#include "tests/hom/common.hh"
#include "tests/hom/common_inductives.hh"

/*-------------------------------------------------------------------------------------------*/

const SDD zero = sdd::zero<conf>();
const SDD one = sdd::one<conf>();
const hom id = sdd::hom::Id<conf>();

struct hom_local_test
  : public testing::Test
{
};

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_local_test, construction)
{
  {
    ASSERT_EQ(id, Local(0, id));
  }
  {
    const hom h1 = Local(0, Inductive<conf>(incr(0,1)));
    const hom h2 = Local(0, Inductive<conf>(incr(0,1)));
    ASSERT_EQ(h1, h2);
  }
  {
    const hom h1 = Local(0, Inductive<conf>(incr(0,1)));
    const hom h2 = Local(0, Inductive<conf>(incr(0,2)));
    ASSERT_NE(h1, h2);
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_local_test, evaluation)
{
}

/*-------------------------------------------------------------------------------------------*/
