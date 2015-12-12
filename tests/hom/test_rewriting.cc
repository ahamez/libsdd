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
  using configuration_type = C;

  sdd::manager<C> m;
  sdd::hom::context<C>& cxt;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  rewriting_test()
    : m(sdd::init(small_conf<C>()))
    , cxt(sdd::global<C>().hom_context)
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::id<C>())
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
                          , inductive<conf>(targeted_incr<conf>("a", 0))
                          , inductive<conf>(targeted_incr<conf>("b", 0))
                          , local("a", o, inductive<conf>(targeted_incr<conf>("a", 0)))
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
    const order o(ob("a", ob {"x"}) << ob("b"));
    const homomorphism h0 = sum<conf>( o
                            , { id
                              , inductive<conf>(targeted_incr<conf>("b", 0))
                              , local("a", o, inductive<conf>(targeted_incr<conf>("x", 0)))
                              });
    const homomorphism h1 = sdd::rewrite(o, h0);
    ASSERT_NE(h1, h0);
    SDD s0(1, SDD(0, {0}, one), SDD(0, {0}, one));
    ASSERT_EQ(h0(o, s0), h1(o, s0));
  }
  {
    const order o(ob({"a", "b", "c"}));
    const homomorphism h0
      = sum<conf>( o
                 , { inductive<conf>(targeted_incr<conf>("a", 0))
                   , inductive<conf>(targeted_incr<conf>("b", 0))
                   , inductive<conf>(targeted_incr<conf>("c", 0))
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
  using optional = sdd::hom::optional_homomorphism<conf>;
  using ob = order_builder;
  const auto ia = inductive<conf>(targeted_incr<conf>("a", 0));
  const auto ib = inductive<conf>(targeted_incr<conf>("b", 0));
  const auto ic = inductive<conf>(targeted_incr<conf>("c", 0));

  {
    const order o(ob("a", ob {"x"}) << ob("b"));
    const homomorphism h0
      = intersection<conf>( o
                          , { inductive<conf>(targeted_incr<conf>("b", 0))
                            , local("a", o, inductive<conf>(targeted_incr<conf>("x", 0)))});
    const homomorphism h1 = sdd::rewrite(o, h0);
    ASSERT_NE(h1, h0);
    SDD s0(1, SDD(0, {0}, one), SDD(0, {0}, one));
    ASSERT_EQ(h0(o, s0), h1(o, s0));
  }
  {
    const order o(ob({"a", "b", "c"}));
    const homomorphism h0 = intersection<conf>(o, {ia, ib, ic});
    const homomorphism h1 = sdd::rewrite(o, h0);
    ASSERT_NE(h1, h0);
    SDD s0(2, {0}, SDD(1, {0}, SDD(0, {0}, one)));
    ASSERT_EQ(h0(o, s0), h1(o, s0));
  }
  {
    const order o(ob({"a", "b", "c"}));
    const homomorphism h0 = intersection<conf>(o, {ia, ib, ic});
    const homomorphism h1 = sdd::rewrite(o, h0);
    const auto ga = {ia};
    const auto gb = {ib};

    const auto r0 = saturation_intersection<conf>
      ( 2
      , /*F*/ saturation_intersection<conf>
        ( 1
        , /*F*/ ic
        , /*G*/ gb.begin(), gb.end()
        , /*L*/ optional())
      , /*G*/ ga.begin(), ga.end()
      , /*L*/ optional());

    ASSERT_NE(h1, h0);
    ASSERT_EQ(r0, h1);
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
      = fixpoint(sum<conf>( o
                          , { id
                            , inductive<conf>(targeted_incr<conf>("b", 0))
                            , local("a", o, inductive<conf>(targeted_incr<conf>("x", 0)))}));
    const auto h1 = sdd::rewrite(o, h0);
    ASSERT_NE(h1, h0);
    SDD s0(1, SDD(0, {0}, one), SDD(0, {0}, one));
    ASSERT_EQ(h0(o, s0), h1(o, s0));
  }
  {
    const order o(ob("a", ob {"x"}) << ob("b"));
    const auto h0
      = fixpoint(sum<conf>( o
                          , { inductive<conf>(targeted_incr<conf>("b", 0))
                            , local("a", o, inductive<conf>(targeted_incr<conf>("x", 0)))}));
    const auto h1 = sdd::rewrite(o, h0);
    ASSERT_EQ(h1, h0);
  }
}

/*------------------------------------------------------------------------------------------------*/
