#include "gtest/gtest.h"

#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"
#include "sdd/manager.hh"
#include "sdd/order/order.hh"

#include "tests/configuration.hh"
#include "tests/hom/common.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct hom_id_test
  : public testing::Test
{
  typedef C configuration_type;

  sdd::manager<C> m;
  sdd::hom::context<C>& cxt;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;

  hom_id_test()
    : m(sdd::manager<C>::init(small_conf<C>()))
    , cxt(sdd::global<C>().hom_context)
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_id_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_id_test, construction)
{
//  homomorphism h1 = id<conf>();
//  homomorphism h2 = id<conf>();
//  ASSERT_EQ(h1, h2);
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_id_test, evaluation)
{
//  homomorphism h = id<conf>();
//  ASSERT_EQ(one, h(order(order_builder()), one));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_id_test, no_cache)
{
//  homomorphism h = id<conf>();
//  ASSERT_EQ(0u, cxt.cache().size());
//  ASSERT_EQ(one, h(cxt, order(order_builder()), one));
//  ASSERT_EQ(0u, cxt.cache().size());
}

/*------------------------------------------------------------------------------------------------*/
