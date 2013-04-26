#include "gtest/gtest.h"

#include "tests/hom/common.hh"

/*------------------------------------------------------------------------------------------------*/

struct hom_cons_test
  : public testing::Test
{
  sdd::manager<conf> m;
  sdd::hom::context<conf>& cxt;

  const SDD zero;
  const SDD one;
  const hom id;

  hom_cons_test()
    : m(sdd::manager<conf>::init())
    , cxt(sdd::global<conf>().hom_context)
    , zero(sdd::zero<conf>())
    , one(sdd::one<conf>())
    , id(sdd::Id<conf>())
  {
  }
};

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_cons_test, construction)
{
  order_builder ob {"0"};
  order o(ob);
  {
    const hom h1 = Cons<conf>(o, conf::Values {0,1,2}, id);
    const hom h2 = Cons<conf>(o, conf::Values {0,1,2}, id);
    ASSERT_EQ(h1, h2);
  }
  {
    const hom h1 = Cons<conf>(o, one, id);
    const hom h2 = Cons<conf>(o, one, id);
    ASSERT_EQ(h1, h2);
  }
  {
    const hom h1 = Cons<conf>(o, conf::Values {0,1,3}, id);
    const hom h2 = Cons<conf>(o, conf::Values {0,1,2}, id);
    ASSERT_NE(h1, h2);
  }
  {
    const hom h1 = Cons<conf>(o, one, id);
    const hom h2 = Cons<conf>(o, zero, id);
    ASSERT_NE(h1, h2);
  }
  {
    const hom h1 = Cons<conf>(o, one, id);
    const hom h2 = Cons<conf>(o, conf::Values {0,1,2}, id);
    ASSERT_NE(h1, h2);
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_cons_test, evaluation)
{
  {
    order o(order_builder {"a"});
    const hom h = Cons<conf>(o, conf::Values {0,1,2}, id);
    ASSERT_EQ(SDD(0, {0,1,2}, one), h(o, one));
  }
  {
    order o(order_builder {"a"});
    const hom h = Cons<conf>(o, conf::Values {}, id);
    ASSERT_EQ(zero, h(o, one));
  }
  {
    order o(order_builder {"a"});
    const hom h = Cons<conf>(o,  one, id);
    ASSERT_EQ(SDD(0, one, one), h(o, one));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_cons_test, no_cache)
{
  order o(order_builder {"a"});
  const hom h = Cons<conf>(o, conf::Values {0,1,2}, id);
  ASSERT_EQ(static_cast<std::size_t>(0), cxt.cache().size());
  ASSERT_EQ(SDD(0, {0,1,2}, one), h(cxt, o, one));
  ASSERT_EQ(static_cast<std::size_t>(0), cxt.cache().size());
}

/*------------------------------------------------------------------------------------------------*/
