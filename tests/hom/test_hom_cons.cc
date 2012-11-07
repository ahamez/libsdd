#include "gtest/gtest.h"

#include "tests/hom/common.hh"

/*-------------------------------------------------------------------------------------------*/

const SDD zero = sdd::zero<conf>();
const SDD one = sdd::one<conf>();
const hom id = sdd::hom::Id<conf>();

struct hom_cons_test
  : public testing::Test
{
};

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_cons_test, construction)
{
  order_builder ob {"0"};
  order o(ob);
  {
    const hom h1 = sdd::hom::Cons<conf>(o, conf::Values {0,1,2}, id);
    const hom h2 = sdd::hom::Cons<conf>(o, conf::Values {0,1,2}, id);
    ASSERT_EQ(h1, h2);
  }
  {
    const hom h1 = sdd::hom::Cons<conf>(o, one, id);
    const hom h2 = sdd::hom::Cons<conf>(o, one, id);
    ASSERT_EQ(h1, h2);
  }
  {
    const hom h1 = sdd::hom::Cons<conf>(o, conf::Values {0,1,3}, id);
    const hom h2 = sdd::hom::Cons<conf>(o, conf::Values {0,1,2}, id);
    ASSERT_NE(h1, h2);
  }
  {
    const hom h1 = sdd::hom::Cons<conf>(o, one, id);
    const hom h2 = sdd::hom::Cons<conf>(o, zero, id);
    ASSERT_NE(h1, h2);
  }
  {
    const hom h1 = sdd::hom::Cons<conf>(o, one, id);
    const hom h2 = sdd::hom::Cons<conf>(o, conf::Values {0,1,2}, id);
    ASSERT_NE(h1, h2);
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_cons_test, evaluation)
{
  {
    order o(order_builder {"a"});
    const hom h = sdd::hom::Cons<conf>(o, conf::Values {0,1,2}, id);
    ASSERT_EQ(SDD(0, {0,1,2}, one), h(o, one));
  }
  {
    order o(order_builder {"a"});
    const hom h = sdd::hom::Cons<conf>(o, conf::Values {}, id);
    ASSERT_EQ(zero, h(o, one));
  }
  {
    order o(order_builder {"a"});
    const hom h = sdd::hom::Cons<conf>(o,  one, id);
    ASSERT_EQ(SDD(0, one, one), h(o, one));
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_cons_test, no_cache)
{
  order o(order_builder {"a"});
  const hom h = sdd::hom::Cons<conf>(o, conf::Values {0,1,2}, id);
  sdd::hom::context<conf> cxt;
  ASSERT_EQ(0, cxt.cache().size());
  ASSERT_EQ(SDD(0, {0,1,2}, one), h(cxt, o, one));
  ASSERT_EQ(0, cxt.cache().size());
}

/*-------------------------------------------------------------------------------------------*/
