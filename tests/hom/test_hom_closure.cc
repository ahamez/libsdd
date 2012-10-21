#include "gtest/gtest.h"

#include "tests/hom/common.hh"
#include "tests/hom/common_inductives.hh"

/*-------------------------------------------------------------------------------------------*/

const SDD one = sdd::one<conf>();
const hom id = sdd::hom::Id<conf>();

struct hom_closure_test
  : public testing::Test
{
};

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_closure_test, construction)
{
  {
    const auto h0 = Closure<conf>(order(order_builder {"0","1","2"}), {"0","1","2"});
    const auto h1 = Closure<conf>(order(order_builder {"0","1","2"}), {"0","1","2"});
    ASSERT_EQ(h0, h1);
  }
  {
    const auto h0 = Closure<conf>(order(order_builder {"0","1","2"}), {"0","1","2"});
    const auto h1 = Closure<conf>(order(order_builder {"0","1","3"}), {"0","1","3"});
    ASSERT_NE(h0, h1);
  }
  {
    order o(order_builder {"a", "b", "c"});
    ASSERT_THROW(Closure<conf>(o, {"d","a"}), std::runtime_error);
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_closure_test, evaluation_flat)
{
  {
    order o(order_builder {"a", "b", "c"});
    const SDD s0(2, {0,1}, SDD(1, {0,1}, SDD(0, {0,1}, one)));
    const auto h0 = Closure<conf>(o, {"a", "b", "c"});
    ASSERT_EQ(s0, h0(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const SDD s0(2, {0,1}, SDD(1, {2,3}, SDD(0, {4,5}, one)));
    const SDD s1(2, {0,1}, SDD(0, {4,5}, one));
    const auto h0 = Closure<conf>(o, {"a", "c"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const SDD s0(2, {0,1}, SDD(1, {0,1}, SDD(0, {0,1}, one)));
    const SDD s1(1, {0,1}, SDD(0, {0,1}, one));
    const auto h0 = Closure<conf>(o, {"b", "c"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const SDD s0(2, {0,1}, SDD(1, {0,1}, SDD(0, {0,1}, one)));
    const SDD s1(2, {0,1}, SDD(1, {0,1}, one));
    const auto h0 = Closure<conf>(o, {"a", "b"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const SDD s0(2, {0,1}, SDD(1, {0,1}, SDD(0, {0,1}, one)));
    const SDD s1(2, {0,1}, one);
    const auto h0 = Closure<conf>(o, {"a"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const SDD s0(2, {0,1}, SDD(1, {0,1}, SDD(0, {0,1}, one)));
    const SDD s1(1, {0,1}, one);
    const auto h0 = Closure<conf>(o, {"b"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const SDD s0(2, {0,1}, SDD(1, {0,1}, SDD(0, {0,1}, one)));
    const SDD s1(0, {0,1}, one);
    const auto h0 = Closure<conf>(o, {"c"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const SDD s0(2, {0,1}, SDD(1, {0,1}, SDD(0, {0,1}, one)));
    const auto h0 = Closure<conf>(o, {"d"});
    ASSERT_THROW(h0(o, s0), std::runtime_error);
  }
  {
    order o(order_builder {"a", "b", "c"});
    const SDD s0 = SDD(2, {0,1}, SDD(1, {0,1}, SDD(0, {0,1}, one)))
                 + SDD(2, {0,1}, SDD(1, {2,3}, SDD(0, {2,3}, one)));
    const auto h0 = Closure<conf>(o, {"a", "b", "c"});
    ASSERT_EQ(s0, h0(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const SDD s0 = SDD(2, {0,1}, SDD(1, {0,1}, SDD(0, {0,1}, one)))
                 + SDD(2, {0,1}, SDD(1, {2,3}, SDD(0, {2,3}, one)));
    const SDD s1 = SDD(2, {0,1}, SDD(0, {0,1,2,3}, one));
    const auto h0 = Closure<conf>(o, {"a", "c"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const SDD s0 = SDD(2, {0,1}, SDD(1, {0,1}, SDD(0, {0,1}, one)))
                 + SDD(2, {0,1}, SDD(1, {2,3}, SDD(0, {2,3}, one)));
    const SDD s1 = SDD(1, {0,1,2,3}, one);
    const auto h0 = Closure<conf>(o, {"b"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const SDD s0 = SDD(2, {0,1}, SDD(1, {0,1}, SDD(0, {0,1}, one)))
                 + SDD(2, {0,1}, SDD(1, {2,3}, SDD(0, {2,3}, one)));
    const SDD s1 = SDD(2, {0,1}, one);
    const auto h0 = Closure<conf>(o, {"a"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const SDD s0 = SDD(2, {0,1}, SDD(1, {0,1}, SDD(0, {0,1}, one)))
                 + SDD(2, {0,1}, SDD(1, {2,3}, SDD(0, {2,3}, one)));
    const SDD s1 = SDD(0, {0,1,2,3}, one);
    const auto h0 = Closure<conf>(o, {"c"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const SDD s0 = SDD(2, {0,1}, SDD(1, {0,1}, SDD(0, {0,1}, one)))
                 + SDD(2, {2,3}, SDD(1, {2,3}, SDD(0, {2,3}, one)));
    const SDD s1 = SDD(2, {0,1}, SDD(0, {0,1}, one))
                 + SDD(2, {2,3}, SDD(0, {2,3}, one));
    const auto h0 = Closure<conf>(o, {"a", "c"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const auto h0 = Closure<conf>(o, {"a", "c"});
    ASSERT_EQ(one, h0(o, one));
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_closure_test, evaluation_hierarchical)
{
  {
    order o(order_builder().add("c").add("x", order_builder {"b"}).add("a"));
    const SDD s0(2, {0,1}, SDD(1, SDD(0, {0,1}, one), SDD(0, {0,1}, one)));
    const SDD s1(2, {0,1}, SDD(0, {0,1}, SDD(0, {0,1}, one)));
    const auto h0 = Closure<conf>(o, {"a", "b", "c"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder().add("c").add("x", order_builder {"b"}).add("a"));
    const SDD s0(2, {0,1}, SDD(1, SDD(0, {0,1}, one), SDD(0, {0,1}, one)));
    const SDD s1(2, {0,1}, SDD(0, {0,1}, SDD(0, {0,1}, one)));
    const auto h0 = Closure<conf>(o, {"c", "b", "a"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder().add("c").add("x", order_builder {"b"}).add("a"));
    const SDD s0(2, {0,1}, SDD(1, SDD(0, {0,1}, one), SDD(0, {0,1}, one)));
    const SDD s1(2, {0,1}, SDD(0, {0,1}, SDD(0, {0,1}, one)));
    const auto h0 = Closure<conf>(o, {"b", "c", "a"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder().add("c").add("x", order_builder {"b"}).add("a"));
    const SDD s0(2, {0,1}, SDD(1, SDD(0, {2,3}, one), SDD(0, {4,5}, one)));
    const SDD s1(0, {2,3}, one);
    const auto h0 = Closure<conf>(o, {"b"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder().add("y", order_builder {"c"}).add("x", order_builder {"b"}).add("a"));
    const SDD s0(2, {0,1}
        , SDD(1, SDD(0, {0,1}, one)
        , SDD(0, SDD(0, {0,1}, one), one)));
    const SDD s1(2, {0,1}, SDD(0, {0,1}, SDD(0, {0,1}, one)));
    const auto h0 = Closure<conf>(o, {"a", "b", "c"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder().add("y", order_builder {"c"}).add("x", order_builder {"b"}).add("a"));
    const SDD s0(2, {0,1}
        , SDD(1, SDD(0, {0,1}, one)
        , SDD(0, SDD(0, {0,1}, one), one)));
    const auto h0 = Closure<conf>(o, {"d"});
    ASSERT_THROW(h0(o, s0), std::runtime_error);
  }
  {
    order o(order_builder().add("y", order_builder {"c"}).add("x", order_builder {"b"}).add("a"));
    const SDD s0 = SDD(2, {0,1}
                 , SDD(1, SDD(0, {0,1}, one)
                 , SDD(0, SDD(0, {0,1}, one), one)))

                 + SDD(2, {0,1}
                 , SDD(1, SDD(0, {2,3}, one)
                 , SDD(0, SDD(0, {2,3}, one), one)));

    const SDD s1 = SDD(2, {0,1}, SDD(0, {0,1}, SDD(0, {0,1}, one)))
                 + SDD(2, {0,1}, SDD(0, {2,3}, SDD(0, {2,3}, one)));

    const auto h0 = Closure<conf>(o, {"a", "b", "c"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder().add("y", order_builder {"c"}).add("x", order_builder {"b"}).add("a"));
    const SDD s0 = SDD(2, {0,1}
                 , SDD(1, SDD(0, {0,1}, one)
                 , SDD(0, SDD(0, {0,1}, one), one)))

                 + SDD(2, {0,1}
                 , SDD(1, SDD(0, {2,3}, one)
                 , SDD(0, SDD(0, {2,3}, one), one)))

                 + SDD(2, {0,1}
                 , SDD(1, SDD(0, {4,5}, one)
                 , SDD(0, SDD(0, {4,5}, one), one)));


    const SDD s1 = SDD(2, {0,1}, SDD(0, {0,1}, SDD(0, {0,1}, one)))
                 + SDD(2, {0,1}, SDD(0, {2,3}, SDD(0, {2,3}, one)))
                 + SDD(2, {0,1}, SDD(0, {4,5}, SDD(0, {4,5}, one)));

    const auto h0 = Closure<conf>(o, {"a", "c", "b"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder().add("y", order_builder {"c"}).add("x", order_builder {"b"}).add("a"));
    const SDD s0 = SDD(2, {0,1}
                 , SDD(1, SDD(0, {0,1}, one)
                 , SDD(0, SDD(0, {0,1}, one), one)))

                 + SDD(2, {0,1}
                 , SDD(1, SDD(0, {2,3}, one)
                 , SDD(0, SDD(0, {2,3}, one), one)))

                 + SDD(2, {0,1}
                 , SDD(1, SDD(0, {4,5}, one)
                 , SDD(0, SDD(0, {4,5}, one), one)));


    const SDD s1 = SDD(2, {0,1}, SDD(0, {0,1,2,3,4,5}, one));

    const auto h0 = Closure<conf>(o, {"a", "c"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder().add("y", order_builder {"c"}).add("x", order_builder {"b"}).add("a"));
    const SDD s0 = SDD(2, {0,1}
                 , SDD(1, SDD(0, {0,1}, one)
                 , SDD(0, SDD(0, {0,1}, one), one)))

                 + SDD(2, {0,1}
                 , SDD(1, SDD(0, {2,3}, one)
                 , SDD(0, SDD(0, {2,3}, one), one)))

                 + SDD(2, {0,1}
                 , SDD(1, SDD(0, {4,5}, one)
                 , SDD(0, SDD(0, {4,5}, one), one)));

    const SDD s1 = SDD(0, {0,1}, SDD(0, {0,1}, one))
                 + SDD(0, {2,3}, SDD(0, {2,3}, one))
                 + SDD(0, {4,5}, SDD(0, {4,5}, one));

    const auto h0 = Closure<conf>(o, {"b", "c"});
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    order o(order_builder().add("y", order_builder {"c"}).add("x", order_builder().add("z", order_builder {"b"})).add("a"));

    const SDD s0 = SDD(2, {0,1}
                 , SDD(1, SDD(0, SDD(0, {0,1}, one), one)
                 , SDD(0, SDD(0, {0,1}, one), one)))

                 + SDD(2, {0,1}
                 , SDD(1, SDD(0, SDD(0, {2,3}, one), one)
                 , SDD(0, SDD(0, {2,3}, one), one)))

                 + SDD(2, {0,1}
                 , SDD(1, SDD(0, SDD(0, {4,5}, one), one)
                 , SDD(0, SDD(0, {4,5}, one), one)));


    const SDD s1 = SDD(0, {0,1}, SDD(0, {0,1}, one))
                 + SDD(0, {2,3}, SDD(0, {2,3}, one))
                 + SDD(0, {4,5}, SDD(0, {4,5}, one));

    const auto h0 = Closure<conf>(o, {"b", "c"});
    ASSERT_EQ(s1, h0(o, s0));
  }
}

/*-------------------------------------------------------------------------------------------*/
