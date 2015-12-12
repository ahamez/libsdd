#include "gtest/gtest.h"

#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"
#include "sdd/hom/rewrite.hh"
#include "sdd/manager.hh"
#include "sdd/order/order.hh"

#include "tests/configuration.hh"
#include "tests/hom/common.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct hom_cons_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;
  sdd::hom::context<C>& cxt;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_cons_test()
    : m(sdd::init(small_conf<C>()))
    , cxt(sdd::global<C>().hom_context)
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::id<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_cons_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_cons_test, construction)
{
  order_builder ob {"0"};
  order o(ob);
  {
    const homomorphism h1 = cons<conf>(o, values_type {0,1,2}, id);
    const homomorphism h2 = cons<conf>(o, values_type {0,1,2}, id);
    ASSERT_EQ(h1, h2);
  }
  {
    const homomorphism h1 = cons<conf>(o, one, id);
    const homomorphism h2 = cons<conf>(o, one, id);
    ASSERT_EQ(h1, h2);
  }
  {
    const homomorphism h1 = cons<conf>(o, values_type {0,1,3}, id);
    const homomorphism h2 = cons<conf>(o, values_type {0,1,2}, id);
    ASSERT_NE(h1, h2);
  }
  {
    const homomorphism h1 = cons<conf>(o, one, id);
    const homomorphism h2 = cons<conf>(o, zero, id);
    ASSERT_NE(h1, h2);
  }
  {
    const homomorphism h1 = cons<conf>(o, one, id);
    const homomorphism h2 = cons<conf>(o, values_type {0,1,2}, id);
    ASSERT_NE(h1, h2);
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_cons_test, evaluation)
{
  {
    order o(order_builder {"a"});
    const homomorphism h = cons<conf>(o, values_type {0,1,2}, id);
    ASSERT_EQ(SDD(0, {0,1,2}, one), h(o, one));
  }
  {
    order o(order_builder {"a"});
    const homomorphism h = cons<conf>(o, values_type {}, id);
    ASSERT_EQ(zero, h(o, one));
  }
  {
    order o(order_builder {"a"});
    const homomorphism h = cons<conf>(o,  one, id);
    ASSERT_EQ(SDD(0, one, one), h(o, one));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_cons_test, no_cache)
{
  order o(order_builder {"a"});
  const homomorphism h = cons<conf>(o, values_type {0,1,2}, id);
  ASSERT_EQ(0u, cxt.cache().size());
  ASSERT_EQ(SDD(0, {0,1,2}, one), h(cxt, o, one));
  ASSERT_EQ(0u, cxt.cache().size());
}

/*------------------------------------------------------------------------------------------------*/
