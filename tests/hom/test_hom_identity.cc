#include "gtest/gtest.h"

#include "tests/hom/common.hh"

/*------------------------------------------------------------------------------------------------*/

const SDD zero = sdd::zero<conf>();
const SDD one = sdd::one<conf>();

struct hom_id_test
  : public testing::Test
{
  hom_id_test()
  {
  }
};

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_id_test, construction)
{
  hom h1 = sdd::Id<conf>();
  hom h2 = sdd::Id<conf>();
  ASSERT_EQ(h1, h2);
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_id_test, evaluation)
{
  hom h = sdd::Id<conf>();
  ASSERT_EQ(one, h(order(order_builder()), one));
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_id_test, no_cache)
{
  hom h = sdd::Id<conf>();
  sdd::hom::context<conf> cxt;
  ASSERT_EQ(0, cxt.cache().size());
  ASSERT_EQ(one, h(cxt, order(order_builder()), one));
  ASSERT_EQ(0, cxt.cache().size());
}

/*------------------------------------------------------------------------------------------------*/
