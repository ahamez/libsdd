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
  typedef C configuration_type;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_local_test()
    : m(sdd::manager<C>::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::Id<C>())
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
    ASSERT_EQ(id, Local("0", o, id));
  }
  {
    const homomorphism h1 = Local("0", o, Inductive<conf>(targeted_incr<conf>("0",1)));
    const homomorphism h2 = Local("0", o, Inductive<conf>(targeted_incr<conf>("0",1)));
    ASSERT_EQ(h1, h2);
  }
  {
    const homomorphism h1 = Local("0", o, Inductive<conf>(targeted_incr<conf>("0",1)));
    const homomorphism h2 = Local("0", o, Inductive<conf>(targeted_incr<conf>("0",2)));
    ASSERT_NE(h1, h2);
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_local_test, evaluation)
{
  order_builder ob;
  order_builder q {"b"};
  ob.add("y", q);
  order_builder p {"a"};
  ob.add("x", p);

  order o(ob);

  const SDD s0 = SDD(1, SDD(0, {0}, one), SDD(0, SDD(0, {1}, one), one));

  const homomorphism h1 = Local("x", o, Inductive<conf>(targeted_incr<conf>("a",1)));
  ASSERT_EQ( SDD(1, SDD(0, {1}, one), SDD(0, SDD(0, {1}, one), one))
           , h1(o, s0));

  const homomorphism h2 = Local("y", o, Inductive<conf>(targeted_incr<conf>("b",1)));
  ASSERT_EQ( SDD(1, SDD(0, {0}, one), SDD(0, SDD(0, {2}, one), one))
           , h2(o, s0));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_local_test, error)
{
  const order o(order_builder {"a", "b"});

  const SDD s0 = SDD(1, {0}, SDD(0, {1}, one));
  const homomorphism h1 = Local("a", o, Inductive<conf>(targeted_incr<conf>("x",1)));
  ASSERT_THROW(h1(o, s0), sdd::evaluation_error<conf>);
  try
  {
    h1(o, s0);
  }
  catch (sdd::evaluation_error<conf>& e)
  {
    ASSERT_EQ(s0, e.operand());
    ASSERT_NE(nullptr, e.what());
  }
}

/*------------------------------------------------------------------------------------------------*/
