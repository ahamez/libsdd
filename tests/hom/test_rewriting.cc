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
  {
    order o(order_builder().push("b").push("a", order_builder {"x"}));
    const homomorphism h0 = Sum<conf>( o
                            , { id
                              , Inductive<conf>(targeted_incr<conf>("b", 0))
                              , Local("a", o, Inductive<conf>(targeted_incr<conf>("x", 0)))
                              });
    const homomorphism h1 = sdd::rewrite(h0, o);
    ASSERT_NE(h1, h0);
    SDD s0(1, SDD(0, {0}, one), SDD(0, {0}, one));
    ASSERT_EQ(h0(o, s0), h1(o, s0));
  }
//  {
//    auto o = empty_order<conf>();
//    o = add_identifier("b", o);
//    auto nested_o = empty_order<conf>();
//    nested_o = add_identifier("x", nested_o);
//    o = add_identifier("a", o, nested_o);
//    const homomorphism h0 = Sum<conf>({ id
//                             , Inductive<conf>(targeted_incr<conf>('1', 0))
//                             , Inductive<conf>(targeted_incr<conf>('0', 0))
//                             , Local(1, Inductive<conf>(targeted_incr<conf>('0', 0)))
//                             });
//    const homomorphism h1 = sdd::hom::rewrite(h0, o);
//
//    SDD s0(1, SDD(0, {0}, one), SDD(0, {0}, one));
//    ASSERT_EQ(h0(s0), h1(s0));
//  }
}

/*------------------------------------------------------------------------------------------------*/

//TYPED_TEST(rewriting_test, fixpoint)
//{
//  {
//    auto o = sdd::order::empty_order<conf>();
//    o = add_identifier("b", o);
//    auto nested_o = sdd::order::empty_order<conf>();
//    nested_o = add_identifier("x", nested_o);
//    o = add_identifier("a", o, nested_o);
//    const hom h0 = Fixpoint(Sum<conf>({ id
//                                      , Inductive<conf>(targeted_incr(1, 0))
//                                      , Inductive<conf>(targeted_incr(0, 0))
//                                      , Local(1, Inductive<conf>(targeted_incr(0, 0)))
//                                      }));
//    const hom h1 = sdd::hom::rewrite(h0, o);
//
//    ASSERT_NE(h1, h0);
//
//    SDD s0(1, SDD(0, {0}, one), SDD(0, {0}, one));
//    ASSERT_EQ(h0(s0), h1(s0));
//  }
//
//  {
//    auto o = sdd::order::empty_order<conf>();
//    o = add_identifier("b", o);
//    auto nested_o = sdd::order::empty_order<conf>();
//    nested_o = add_identifier("x", nested_o);
//    o = add_identifier("a", o, nested_o);
//    const hom h0 = Fixpoint(Sum<conf>({ Inductive<conf>(targeted_incr(1, 0))
//                                      , Inductive<conf>(targeted_incr(0, 0))
//                                      , Local(1, Inductive<conf>(targeted_incr(0, 0)))
//                                      }));
//    const hom h1 = sdd::hom::rewrite(h0, o);
//
//    ASSERT_EQ(h1, h0);
//  }
//}

/*------------------------------------------------------------------------------------------------*/
