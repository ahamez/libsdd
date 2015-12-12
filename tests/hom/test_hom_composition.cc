#include "gtest/gtest.h"

#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"
#include "sdd/hom/rewrite.hh"
#include "sdd/manager.hh"
#include "sdd/order/order.hh"

#include "tests/configuration.hh"
#include "tests/hom/common.hh"
#include "tests/hom/common_inductives.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct hom_composition_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_composition_test()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::id<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_composition_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_composition_test, construction)
{
  {
    ASSERT_EQ(id, composition(id, id));
  }
  {
    const homomorphism h = inductive<conf>(targeted_incr<conf>("0",1));
    ASSERT_EQ(h, composition(id, h));
    ASSERT_EQ(h, composition(h, id));
  }
  {
    const homomorphism h1 = inductive<conf>(targeted_incr<conf>("0",1));
    const homomorphism h2 = inductive<conf>(targeted_incr<conf>("0",2));
    ASSERT_EQ(composition(h1, h2), composition(h1, h2));
    ASSERT_NE(composition(h2, h1), composition(h1, h2));
    ASSERT_NE(composition(h1, h2), composition(h2, h1));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_composition_test, evaluation)
{
  {
    const order o(order_builder {"a"});
    const homomorphism h1 = inductive<conf>(targeted_incr<conf>("a",1));
    const homomorphism h2 = inductive<conf>(targeted_incr<conf>("a",1));
    const homomorphism c0 = composition(h1, h2);
    SDD s0(0, {0}, one);
    SDD s1(0, {2}, one);
    ASSERT_EQ(s1, c0(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
