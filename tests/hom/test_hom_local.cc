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
struct hom_local_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_local_test()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::id<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_local_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_local_test, construction)
{
  order o(order_builder {"0"});;
  {
    ASSERT_EQ(id, local("0", o, id));
  }
  {
    const homomorphism h1 = local("0", o, inductive<conf>(targeted_incr<conf>("0",1)));
    const homomorphism h2 = local("0", o, inductive<conf>(targeted_incr<conf>("0",1)));
    ASSERT_EQ(h1, h2);
  }
  {
    const homomorphism h1 = local("0", o, inductive<conf>(targeted_incr<conf>("0",1)));
    const homomorphism h2 = local("0", o, inductive<conf>(targeted_incr<conf>("0",2)));
    ASSERT_NE(h1, h2);
  }
  /// @todo Test that construction with a flat identifier fails.
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_local_test, evaluation)
{
  using ob = order_builder;
  const order o(ob("x", ob("a")) << ob("y", ob("b")));

  const SDD s0 = SDD(1, SDD(0, {0}, one), SDD(0, SDD(0, {1}, one), one));

  const homomorphism h1 = local("x", o, inductive<conf>(targeted_incr<conf>("a",1)));
  ASSERT_EQ( SDD(1, SDD(0, {1}, one), SDD(0, SDD(0, {1}, one), one))
           , h1(o, s0));

  const homomorphism h2 = local("y", o, inductive<conf>(targeted_incr<conf>("b",1)));
  ASSERT_EQ( SDD(1, SDD(0, {0}, one), SDD(0, SDD(0, {2}, one), one))
           , h2(o, s0));
}

/*------------------------------------------------------------------------------------------------*/
