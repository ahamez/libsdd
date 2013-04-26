#include "gtest/gtest.h"

#include "tests/hom/common.hh"

/*------------------------------------------------------------------------------------------------*/

struct hom_id_test
  : public testing::Test
{
  sdd::manager<conf> m;
  sdd::hom::context<conf>& cxt;

  const SDD zero;
  const SDD one;

  hom_id_test()
    : m(sdd::manager<conf>::init())
    , cxt(sdd::global<conf>().hom_context)
    , zero(sdd::zero<conf>())
    , one(sdd::one<conf>())
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
  ASSERT_EQ(0u, cxt.cache().size());
  ASSERT_EQ(one, h(cxt, order(order_builder()), one));
  ASSERT_EQ(0u, cxt.cache().size());
}

/*------------------------------------------------------------------------------------------------*/
