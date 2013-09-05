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
  typedef C configuration_type;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_composition_test()
    : m(sdd::manager<C>::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::Id<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_composition_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_composition_test, construction)
{
  {
    ASSERT_EQ(id, Composition(id, id));
  }
  {
    const homomorphism h = Inductive<conf>(targeted_incr<conf>("0",1));
    ASSERT_EQ(h, Composition(id, h));
    ASSERT_EQ(h, Composition(h, id));
  }
  {
    const homomorphism h1 = Inductive<conf>(targeted_incr<conf>("0",1));
    const homomorphism h2 = Inductive<conf>(targeted_incr<conf>("0",2));
    ASSERT_EQ(Composition(h1, h2), Composition(h1, h2));
    ASSERT_NE(Composition(h2, h1), Composition(h1, h2));
    ASSERT_NE(Composition(h1, h2), Composition(h2, h1));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_composition_test, evaluation)
{
  {
    const order o(order_builder {"a"});
    const homomorphism h1 = Inductive<conf>(targeted_incr<conf>("a",1));
    const homomorphism h2 = Inductive<conf>(targeted_incr<conf>("a",1));
    const homomorphism c0 = Composition(h1, h2);
    SDD s0(0, {0}, one);
    SDD s1(0, {2}, one);
    ASSERT_EQ(s1, c0(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
