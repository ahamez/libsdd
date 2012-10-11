#include "gtest/gtest.h"

#include "tests/hom/common.hh"

/*-------------------------------------------------------------------------------------------*/

const SDD zero = sdd::zero<conf>();
const SDD one = sdd::one<conf>();
const hom id = sdd::hom::Id<conf>();

struct hom_id_test
  : public testing::Test
{
  hom_id_test()
  {
  }
};

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_id_test, construction)
{
  hom h1 = sdd::hom::Id<conf>();
  hom h2 = sdd::hom::Id<conf>();
  ASSERT_EQ(h1, h2);
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_id_test, evaluation)
{
  hom h = sdd::hom::Id<conf>();
  ASSERT_EQ(one, h(empty_order<conf>(), one));
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_id_test, no_cache)
{
  hom h = sdd::hom::Id<conf>();
  sdd::hom::context<conf> cxt;
  ASSERT_EQ(0, cxt.cache().size());
  ASSERT_EQ(one, h(cxt, empty_order<conf>(), one));
  ASSERT_EQ(0, cxt.cache().size());
}

/*-------------------------------------------------------------------------------------------*/
