#include "gtest/gtest.h"

#include "tests/hom/common.hh"

/*------------------------------------------------------------------------------------------------*/

const SDD zero = sdd::zero<conf>();
const SDD one = sdd::one<conf>();
const hom id = sdd::Id<conf>();

struct hom_cons_test
  : public testing::Test
{
};

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_cons_test, construction)
{
  {
    const hom h1 = Cons<conf>(0, conf::Values {0,1,2}, id);
    const hom h2 = Cons<conf>(0, conf::Values {0,1,2}, id);
    ASSERT_EQ(h1, h2);
  }
  {
    const hom h1 = Cons<conf>(0, one, id);
    const hom h2 = Cons<conf>(0, one, id);
    ASSERT_EQ(h1, h2);
  }
  {
    const hom h1 = Cons<conf>(0, conf::Values {0,1,3}, id);
    const hom h2 = Cons<conf>(0, conf::Values {0,1,2}, id);
    ASSERT_NE(h1, h2);
  }
  {
    const hom h1 = Cons<conf>(0, one, id);
    const hom h2 = Cons<conf>(0, zero, id);
    ASSERT_NE(h1, h2);
  }
  {
    const hom h1 = Cons<conf>(0, one, id);
    const hom h2 = Cons<conf>(0, conf::Values {0,1,2}, id);
    ASSERT_NE(h1, h2);
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_cons_test, evaluation)
{
  {
    const hom h = Cons<conf>(0, conf::Values {0,1,2}, id);
    ASSERT_EQ(SDD(0, {0,1,2}, one), h(one));
  }
  {
    const hom h = Cons<conf>(0, conf::Values {}, id);
    ASSERT_EQ(zero, h(one));
  }
  {
    const hom h = Cons<conf>(0, one, id);
    ASSERT_EQ(SDD(0, one, one), h(one));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_cons_test, no_cache)
{
  const hom h = Cons<conf>(0, conf::Values {0,1,2}, id);
  sdd::hom::context<conf> cxt;
  ASSERT_EQ(0, cxt.cache().size());
  ASSERT_EQ(SDD(0, {0,1,2}, one), h(cxt, one));
  ASSERT_EQ(0, cxt.cache().size());
}

/*------------------------------------------------------------------------------------------------*/
