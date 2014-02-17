#include <vector>

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
struct rewriting_test
  : public testing::Test
{
  typedef C configuration_type;

  sdd::manager<C> m;
  sdd::hom::context<C>& cxt;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  rewriting_test()
    : m(sdd::manager<C>::init(small_conf<C>()))
    , cxt(sdd::global<C>().hom_context)
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::Id<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(rewriting_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(rewriting_test, partition)
{
  {
    const order o(order_builder {"a"});
    std::vector<homomorphism> homs { id
                          , Inductive<conf>(targeted_incr<conf>("a", 0))
                          , Inductive<conf>(targeted_incr<conf>("b", 0))
                          , Local("a", o, Inductive<conf>(targeted_incr<conf>("a", 0)))
                          };

    const auto&& p = sdd::hom::template rewriter<conf>::partition(o, homs.begin(), homs.end());
    ASSERT_EQ(1u, std::get<0>(p).size()); // F size
    ASSERT_EQ(1u, std::get<1>(p).size()); // G size
    ASSERT_EQ(1u, std::get<2>(p).size()); // L size
    ASSERT_EQ(true, std::get<3>(p)); // has_id
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(rewriting_test, sum)
{
  using ob = order_builder;
  {
//    const order o(ob().push("b").push("a", ob {"x"}));
    const order o(ob("a", ob {"x"}) << ob("b"));
    const homomorphism h0 = Sum<conf>( o
                            , { id
                              , Inductive<conf>(targeted_incr<conf>("b", 0))
                              , Local("a", o, Inductive<conf>(targeted_incr<conf>("x", 0)))
                              });
    const homomorphism h1 = sdd::rewrite(o, h0);
    ASSERT_NE(h1, h0);
    SDD s0(1, SDD(0, {0}, one), SDD(0, {0}, one));
    ASSERT_EQ(h0(o, s0), h1(o, s0));
  }
  {
    const order o(ob({"a", "b", "c"}));
    const homomorphism h0
      = Sum<conf>( o
                 , { Inductive<conf>(targeted_incr<conf>("a", 0))
                   , Inductive<conf>(targeted_incr<conf>("b", 0))
                   , Inductive<conf>(targeted_incr<conf>("c", 0))
                   }
                 );
    const homomorphism h1 = sdd::rewrite(o, h0);
    ASSERT_NE(h1, h0);
    SDD s0(2, {0}, SDD(1, {0}, SDD(0, {0}, one)));
    ASSERT_EQ(h0(o, s0), h1(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(rewriting_test, intersection)
{
  using ob = order_builder;
  {
    const order o(ob("a", ob {"x"}) << ob("b"));
    const homomorphism h0
      = Intersection<conf>( o
                          , { Inductive<conf>(targeted_incr<conf>("b", 0))
                            , Local("a", o, Inductive<conf>(targeted_incr<conf>("x", 0)))});
    const homomorphism h1 = sdd::rewrite(o, h0);
    ASSERT_NE(h1, h0);
    SDD s0(1, SDD(0, {0}, one), SDD(0, {0}, one));
    ASSERT_EQ(h0(o, s0), h1(o, s0));
  }
  {
    const order o(ob({"a", "b", "c"}));
    const homomorphism h0
      = Intersection<conf>( o
                         , { Inductive<conf>(targeted_incr<conf>("a", 0))
                           , Inductive<conf>(targeted_incr<conf>("b", 0))
                           , Inductive<conf>(targeted_incr<conf>("c", 0))});
    const homomorphism h1 = sdd::rewrite(o, h0);
    ASSERT_NE(h1, h0);
    SDD s0(2, {0}, SDD(1, {0}, SDD(0, {0}, one)));
    ASSERT_EQ(h0(o, s0), h1(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(rewriting_test, transitive_closure)
{
  using ob = order_builder;
  {
    const order o(ob("a", ob {"x"}) << ob("b"));
    const auto h0
      = Fixpoint(Sum<conf>( o
                          , { id
                            , Inductive<conf>(targeted_incr<conf>("b", 0))
                            , Local("a", o, Inductive<conf>(targeted_incr<conf>("x", 0)))}));
    const auto h1 = sdd::rewrite(o, h0);
    ASSERT_NE(h1, h0);
    SDD s0(1, SDD(0, {0}, one), SDD(0, {0}, one));
    ASSERT_EQ(h0(o, s0), h1(o, s0));
  }
  {
    const order o(ob("a", ob {"x"}) << ob("b"));
    const auto h0
      = Fixpoint(Sum<conf>( o
                          , { Inductive<conf>(targeted_incr<conf>("b", 0))
                            , Local("a", o, Inductive<conf>(targeted_incr<conf>("x", 0)))}));
    const auto h1 = sdd::rewrite(o, h0);
    ASSERT_EQ(h1, h0);
  }
}

/*------------------------------------------------------------------------------------------------*/
