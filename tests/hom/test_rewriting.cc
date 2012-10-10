#include <vector>

#include "gtest/gtest.h"

#include "tests/hom/common.hh"
#include "tests/hom/common_inductives.hh"
#include "sdd/hom/rewriting.hh"

/*-------------------------------------------------------------------------------------------*/

const SDD zero = sdd::zero<conf>();
const SDD one = sdd::one<conf>();
const hom id = sdd::hom::Id<conf>();

struct rewriting_test
  : public testing::Test
{
};

/*-------------------------------------------------------------------------------------------*/

TEST_F(rewriting_test, partition)
{
  {
    std::vector<hom> homs { id
                          , Inductive<conf>(targeted_incr('a', 0))
                          , Inductive<conf>(targeted_incr('b', 0))
                          , Local('a', Inductive<conf>(targeted_incr('a', 0)))
                          };

    const auto&& p = sdd::hom::rewriter<conf>::partition('a', homs.begin(), homs.end());
    ASSERT_EQ(1, std::get<0>(p).size()); // F size
    ASSERT_EQ(1, std::get<1>(p).size()); // G size
    ASSERT_EQ(1, std::get<2>(p).size()); // L size
    ASSERT_EQ(true, std::get<3>(p)); // has_id
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(rewriting_test, sum)
{
  {
    auto o = sdd::order::empty_order<conf>();
    o = add_identifier("b", o);
    auto nested_o = sdd::order::empty_order<conf>();
    nested_o = add_identifier("x", nested_o);
    o = add_identifier("a", o, nested_o);
    const hom h0 = Sum<conf>({ id
                             , Inductive<conf>(targeted_incr(1, 0))
                             , Inductive<conf>(targeted_incr(0, 0))
                             , Local(1, Inductive<conf>(targeted_incr(0, 0)))
                             });
    const hom h1 = sdd::hom::rewrite(h0, o);

    SDD s0(1, SDD(0, {0}, one), SDD(0, {0}, one));
    ASSERT_EQ(h0(s0), h1(s0));
  }
  {
    auto o = sdd::order::empty_order<conf>();
    o = add_identifier("b", o);
    auto nested_o = sdd::order::empty_order<conf>();
    nested_o = add_identifier("x", nested_o);
    o = add_identifier("a", o, nested_o);
    const hom h0 = Sum<conf>({ id
                             , Inductive<conf>(targeted_incr('1', 0))
                             , Inductive<conf>(targeted_incr('0', 0))
                             , Local(1, Inductive<conf>(targeted_incr('0', 0)))
                             });
    const hom h1 = sdd::hom::rewrite(h0, o);

    SDD s0(1, SDD(0, {0}, one), SDD(0, {0}, one));
    ASSERT_EQ(h0(s0), h1(s0));
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(rewriting_test, fixpoint)
{
  {
    auto o = sdd::order::empty_order<conf>();
    o = add_identifier("b", o);
    auto nested_o = sdd::order::empty_order<conf>();
    nested_o = add_identifier("x", nested_o);
    o = add_identifier("a", o, nested_o);
    const hom h0 = Fixpoint(Sum<conf>({ id
                                      , Inductive<conf>(targeted_incr(1, 0))
                                      , Inductive<conf>(targeted_incr(0, 0))
                                      , Local(1, Inductive<conf>(targeted_incr(0, 0)))
                                      }));
    const hom h1 = sdd::hom::rewrite(h0, o);

    ASSERT_NE(h1, h0);

    SDD s0(1, SDD(0, {0}, one), SDD(0, {0}, one));
    ASSERT_EQ(h0(s0), h1(s0));
  }

  {
    auto o = sdd::order::empty_order<conf>();
    o = add_identifier("b", o);
    auto nested_o = sdd::order::empty_order<conf>();
    nested_o = add_identifier("x", nested_o);
    o = add_identifier("a", o, nested_o);
    const hom h0 = Fixpoint(Sum<conf>({ Inductive<conf>(targeted_incr(1, 0))
                                      , Inductive<conf>(targeted_incr(0, 0))
                                      , Local(1, Inductive<conf>(targeted_incr(0, 0)))
                                      }));
    const hom h1 = sdd::hom::rewrite(h0, o);

    ASSERT_EQ(h1, h0);
  }
}

/*-------------------------------------------------------------------------------------------*/
