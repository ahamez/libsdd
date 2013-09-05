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
struct hom_fixpoint_test
  : public testing::Test
{
  typedef C configuration_type;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_fixpoint_test()
    : m(sdd::manager<C>::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::Id<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_fixpoint_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_fixpoint_test, construction)
{
  {
    ASSERT_EQ(id, Fixpoint(id));
  }
  {
    ASSERT_EQ( Fixpoint(Inductive<conf>(targeted_incr<conf>("0",1)))
             , Fixpoint(Inductive<conf>(targeted_incr<conf>("0",1)))
             );
  }
  {
    ASSERT_NE( Fixpoint(Inductive<conf>(targeted_incr<conf>("0",1)))
             , Fixpoint(Inductive<conf>(targeted_incr<conf>("0",2)))
             );
  }
  {
    ASSERT_EQ( Fixpoint(Inductive<conf>(targeted_incr<conf>("0",1)))
             , Fixpoint(Fixpoint(Inductive<conf>(targeted_incr<conf>("0",1))))
             );
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_fixpoint_test, evaluation)
{
  {
    order o(order_builder {"0"});
    SDD s0(0, {0}, one);
    homomorphism h0 = Fixpoint(Sum(o, {Inductive<conf>(targeted_incr<conf>("0", 1)), id}));
    ASSERT_EQ(SDD(0, {0,1,2}, one), h0(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
