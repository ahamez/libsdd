#include "gtest/gtest.h"

#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"
#include "sdd/hom/rewrite.hh"
#include "sdd/manager.hh"
#include "sdd/order/order.hh"

#include "tests/configuration.hh"
#include "tests/hom/common.hh"
#include "tests/hom/common_inductives.hh"
#include "tests/hom/expression.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_expression_test, flat_single_path)
{
  const auto l = {"a", "b"};
  const auto _ = 42; // don't care value
  const auto xx = 33; // don't care value
  const auto yy = 21; // don't care value
  const auto zz = 22; // don't care value
  const auto ww = 23; // don't care value
  {
    order o(order_builder {"c", "a", "b"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    SDD s0(2, {_}, SDD(1, {1}, SDD(0, {1}, one)));
    SDD s1(2, {2}, SDD(1, {1}, SDD(0, {1}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"x", "c", "a", "b"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(3, {xx}, SDD(2, {_}, SDD(1, {1}, SDD(0, {1}, one))));
    const auto s1 = SDD(3, {xx}, SDD(2, {2}, SDD(1, {1}, SDD(0, {1}, one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"x", "c", "y", "a", "b"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(4, {xx}, SDD(3, {_}, SDD(2, {yy}, SDD(1, {1}, SDD(0, {1}, one)))));
    const auto s1 = SDD(4, {xx}, SDD(3, {2}, SDD(2, {yy}, SDD(1, {1}, SDD(0, {1}, one)))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"x", "c", "y", "a", "z", "b"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(5,{xx},SDD(4,{_},SDD(3,{yy},SDD(2,{1},SDD(1,{zz},SDD(0, {1}, one))))));
    const auto s1 = SDD(5,{xx},SDD(4,{2},SDD(3,{yy},SDD(2,{1},SDD(1,{zz},SDD(0, {1}, one))))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"x", "c", "y", "a", "z", "b", "w"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(6,{xx},SDD(5,{_},SDD(4,{yy},SDD(3,{1},SDD(2,{zz},SDD(1,{1},SDD(0, {ww}
                                                                                       , one)))))));
    const auto s1 = SDD(6,{xx},SDD(5,{2},SDD(4,{yy},SDD(3,{1},SDD(2,{zz},SDD(1,{1},SDD(0, {ww}
                                                                                       , one)))))));
    ASSERT_EQ(s1, h(o, s0));
  }
  this->m.reset_hom_cache();
  {
    order o(order_builder {"a", "c", "b"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    SDD s0(2, {1}, SDD(1, {_}, SDD(0, {1}, one)));
    SDD s1(2, {1}, SDD(1, {2}, SDD(0, {1}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"x", "a", "c", "b"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(3, {xx}, SDD(2, {1}, SDD(1, {_}, SDD(0, {1}, one))));
    const auto s1 = SDD(3, {xx}, SDD(2, {1}, SDD(1, {2}, SDD(0, {1}, one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"x", "a", "y", "c", "b"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(4, {xx}, SDD(3, {1}, SDD(2, {yy}, SDD(1, {_}, SDD(0, {1}, one)))));
    const auto s1 = SDD(4, {xx}, SDD(3, {1}, SDD(2, {yy}, SDD(1, {2}, SDD(0, {1}, one)))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"x", "a", "y", "c", "z", "b"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(5,{xx},SDD(4,{1},SDD(3,{yy},SDD(2,{_},SDD(1,{zz},SDD(0, {1}, one))))));
    const auto s1 = SDD(5,{xx},SDD(4,{1},SDD(3,{yy},SDD(2,{2},SDD(1,{zz},SDD(0, {1}, one))))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"x", "a", "y", "c", "z", "b", "w"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(6,{xx},SDD(5,{1},SDD(4,{yy},SDD(3,{_},SDD(2,{zz},SDD(1,{1},SDD(0, {ww}
                                                                                       , one)))))));
    const auto s1 = SDD(6,{xx},SDD(5,{1},SDD(4,{yy},SDD(3,{2},SDD(2,{zz},SDD(1,{1},SDD(0, {ww}
                                                                                       , one)))))));
    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_expression_test, flat_single_path_self)
{
  const auto l = {"a", "b"};
  const auto xx = 33; // don't care value
  const auto yy = 21; // don't care value
  const auto zz = 22; // don't care value
  {
    order o(order_builder {"b", "a"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "b");
    const auto s0 = SDD(1, {1}, SDD(0, {1}, one));
    const auto s1 = SDD(1, {2}, SDD(0, {1}, one));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"x", "b", "a"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "b");
    const auto s0 = SDD(2, {xx}, SDD(1, {1}, SDD(0, {1}, one)));
    const auto s1 = SDD(2, {xx}, SDD(1, {2}, SDD(0, {1}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"x", "b", "y", "a"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "b");
    const auto s0 = SDD(3, {xx}, SDD(2, {1}, SDD(1, {yy}, SDD(0, {1}, one))));
    const auto s1 = SDD(3, {xx}, SDD(2, {2}, SDD(1, {yy}, SDD(0, {1}, one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"x", "b", "y", "a", "z"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "b");
    const auto s0 = SDD(4,{xx},SDD(3,{7},SDD(2,{yy},SDD(1,{1},SDD(0,{zz},one)))));
    const auto s1 = SDD(4,{xx},SDD(3,{8},SDD(2,{yy},SDD(1,{1},SDD(0,{zz},one)))));
    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_expression_test, flat)
{
  const auto l = {"a", "b"};
  const auto l2 = {"a", "b", "c"};
  const auto _ = 42; // don't care value
  {
    order o(order_builder {"c", "a", "b"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, {_}, SDD(1, {1}, SDD(0, {1}, one)))
                  + SDD(2, {_}, SDD(1, {2}, SDD(0, {2}, one)));
    const auto s1 = SDD(2, {2}, SDD(1, {1}, SDD(0, {1}, one)))
                  + SDD(2, {4}, SDD(1, {2}, SDD(0, {2}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"c", "a", "b"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one)))
                  + SDD(2, {2}, SDD(1, {2}, SDD(0, {2}, one)));
    const auto s1 = SDD(2, {2}, SDD(1, {1}, SDD(0, {1}, one)))
                  + SDD(2, {4}, SDD(1, {2}, SDD(0, {2}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"d", "a", "b", "c"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast2), l2.begin(), l2.end(), "d");
    const auto s0 = SDD(3, {_}, SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one))))
                  + SDD(3, {_}, SDD(2, {1}, SDD(1, {2}, SDD(0, {2}, one))))
                  + SDD(3, {_}, SDD(2, {3}, SDD(1, {3}, SDD(0, {3}, one))));
    const auto s1 = SDD(3, {3}, SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one))))
                  + SDD(3, {5}, SDD(2, {1}, SDD(1, {2}, SDD(0, {2}, one))))
                  + SDD(3, {9}, SDD(2, {3}, SDD(1, {3}, SDD(0, {3}, one))));
    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_expression_test, flat_self)
{
  const auto l2 = {"a", "b", "c"};
  const auto l3 = {"a", "b", "c", "d"};
  {
    order o(order_builder {"c", "a", "b"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast2), l2.begin(), l2.end(), "c");
    const auto s0 = SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one)))
                  + SDD(2, {2}, SDD(1, {2}, SDD(0, {2}, one)));
    const auto s1 = SDD(2, {3}, SDD(1, {1}, SDD(0, {1}, one)))
                  + SDD(2, {6}, SDD(1, {2}, SDD(0, {2}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"c", "a", "b"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast2), l2.begin(), l2.end(), "c");
    const auto s0 = SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one)))
                  + SDD(2, {1}, SDD(1, {2}, SDD(0, {2}, one)));
    const auto s1 = SDD(2, {3}, SDD(1, {1}, SDD(0, {1}, one)))
                  + SDD(2, {5}, SDD(1, {2}, SDD(0, {2}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c", "d"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast3), l3.begin(), l3.end(), "a");
    const auto s0 = SDD(3, {1}, SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one))))
                  + SDD(3, {1}, SDD(2, {1}, SDD(1, {2}, SDD(0, {2}, one))))
                  + SDD(3, {1}, SDD(2, {3}, SDD(1, {3}, SDD(0, {3}, one))));
    const auto s1 = SDD(3, {4}, SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one))))
                  + SDD(3, {6}, SDD(2, {1}, SDD(1, {2}, SDD(0, {2}, one))))
                  + SDD(3, {10}, SDD(2, {3}, SDD(1, {3}, SDD(0, {3}, one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c", "d"});
    const auto h = Expression<conf>(o, evaluator<conf>(ast3), l3.begin(), l3.end(), "a");
    const auto s0 = SDD(3, {1}, SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one))))
                  + SDD(3, {1}, SDD(2, {1}, SDD(1, {2}, SDD(0, {2}, one))))
                  + SDD(3, {2}, SDD(2, {3}, SDD(1, {3}, SDD(0, {3}, one))));
    const auto s1 = SDD(3, {4}, SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one))))
                  + SDD(3, {6}, SDD(2, {1}, SDD(1, {2}, SDD(0, {2}, one))))
                  + SDD(3, {11}, SDD(2, {3}, SDD(1, {3}, SDD(0, {3}, one))));
    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_expression_test, hierarchical_single_path)
{
  const auto l = {"a", "b"};
  const auto _  = 21; // don't care value
  using ob = order_builder;
  {
    order o(ob("i", ob ({"c", "a", "b"})));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(0, SDD(2, {_}, SDD(1, {1}, SDD(0, {1}, one))), one);
    const auto s1 = SDD(0, SDD(2, {2}, SDD(1, {1}, SDD(0, {1}, one))), one);
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder().push("j", order_builder().push("i", order_builder {"c", "a", "b"})));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(0, SDD(0, SDD(2, {_}, SDD(1, {1}, SDD(0, {1}, one))), one), one);
    const auto s1 = SDD(0, SDD(0, SDD(2, {2}, SDD(1, {1}, SDD(0, {1}, one))), one), one);
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("c")) << ob("j", ob("a")) << ob("k", ob("b")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one)));
    const auto s1 = SDD(2, SDD(0, {2}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("c")) << ob("j", ob("z")) << ob("k", ob("b")) << ob("l", ob("a")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(3, SDD(0, {_}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one))));
    const auto s1 = SDD(3, SDD(0, {2}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("b")) << ob("j", ob("z")) << ob("k", ob("c")) << ob("l", ob("a")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {_}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one))));
    const auto s1 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("b")) << ob("j", ob("z")) << ob("k",ob("c")) << ob("l",ob({"a","w"})));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {_}, one)
                  , SDD(0, SDD(1, {1}, SDD(0, {_}, one))
                  , one))));
    const auto s1 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(1, {1}, SDD(0, {_}, one))
                  , one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("b")) << ob("j", ob("z")) << ob("k",ob("c")) << ob("l",ob({"w","a"})));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {_}, one)
                  , SDD(0, SDD(1, {_}, SDD(0, {1}, one))
                  , one))));
    const auto s1 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(1, {_}, SDD(0, {1}, one))
                  , one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("b")) << ob("j", ob("z")) << ob("k",ob{"w","c"}) << ob("l",ob("a")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(1, {_}, SDD(0, {_}, one))
                  , SDD(0, SDD(0, {1}, one)
                  , one))));
    const auto s1 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(1, {_}, SDD(0, {2}, one))
                  , SDD(0, SDD(0, {1}, one)
                  , one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i",ob("b")) << ob("j",ob("z")) << ob("k",ob{"w","c","v"}) << ob("l",ob("a")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(2, {_}, SDD(1, {_}, SDD(0, {_}, one)))
                  , SDD(0, SDD(0, {1}, one)
                  , one))));
    const auto s1 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(2, {_}, SDD(1, {2}, SDD(0, {_}, one)))
                  , SDD(0, SDD(0, {1}, one)
                  , one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(  ob("i", ob("j", ob("c")))
                 << ob("k", ob("l", ob("m", ob("a"))))
                 << ob("n", ob("b")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0,SDD(0, {_}, one),one),
                    SDD(1, SDD(0, SDD(0, SDD(0, {2}, one),one),one),
                    SDD(0, SDD(0, {1}, one), one)
                    ));
    const auto s1 = SDD(2, SDD(0,SDD(0, {3}, one),one),
                    SDD(1, SDD(0, SDD(0, SDD(0, {2}, one),one),one),
                    SDD(0, SDD(0, {1}, one), one)
                    ));
    ASSERT_EQ(s1, h(o, s0));
  }
  this->m.reset_hom_cache();
  {
    const order o(  ob("i", ob("j", ob("a")))
                 << ob("k", ob("l", ob("m", ob("c"))))
                 << ob("n", ob("b")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0,SDD(0, {4}, one),one),
                    SDD(1, SDD(0, SDD(0, SDD(0, {_}, one),one),one),
                    SDD(0, SDD(0, {1}, one), one)
                    ));
    const auto s1 = SDD(2, SDD(0,SDD(0, {4}, one),one),
                    SDD(1, SDD(0, SDD(0, SDD(0, {5}, one),one),one),
                    SDD(0, SDD(0, {1}, one), one)
                    ));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("c")) << ob("a") << ob("k", ob("b")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0, {_}, one)
                  , SDD(1, {2}
                  , SDD(0, SDD(0, {3}, one)
                  , one)));
    const auto s1 = SDD(2, SDD(0, {5}, one)
                  , SDD(1, {2}
                  , SDD(0, SDD(0, {3}, one)
                  , one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  this->m.reset_hom_cache();
  {
    const order o(ob("i", ob("a")) << ob("c") << ob("k", ob("b")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0, {2}, one)
                  , SDD(1, {_}
                  , SDD(0, SDD(0, {5}, one)
                  , one)));
    const auto s1 = SDD(2, SDD(0, {2}, one)
                  , SDD(1, {7}
                  , SDD(0, SDD(0, {5}, one)
                  , one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("a")) << ob("j", ob({"c", "z"})) << ob("k", ob({"y", "b", "x"})));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0, {2}, one)
                  , SDD(1, SDD(1, {_}, SDD(0, {_}, one))
                  , SDD(0, SDD(2, {_}, SDD(1, {2}, SDD(0, {_}, one)))
                  , one)));
    const auto s1 = SDD(2, SDD(0, {2}, one)
                  , SDD(1, SDD(1, {4}, SDD(0, {_}, one))
                  , SDD(0, SDD(2, {_}, SDD(1, {2}, SDD(0, {_}, one)))
                  , one)));

    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("a")) << ob("j", ob("c")) << ob("b") << ob("x"));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, {1}
                  , SDD(0, {_}
                  , one))));
    const auto s1 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {2}, one)
                  , SDD(1, {1}
                  , SDD(0, {_}
                  , one))));
    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_expression_test, hierarchical)
{
  const auto l = {"a", "b"};
  const auto _  = 21; // don't care value
  const auto x = 42;  // don't care value
  const auto y = 33;  // don't care value
  using ob = order_builder;
  {
    order o(ob("i", ob ({"c", "a", "b"})));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(0, SDD(2, {_}, SDD(1, {1}, SDD(0, {1}, one))), one)
                  + SDD(0, SDD(2, {_}, SDD(1, {2}, SDD(0, {2}, one))), one);
    const auto s1 = SDD(0, SDD(2, {2}, SDD(1, {1}, SDD(0, {1}, one))), one)
                  + SDD(0, SDD(2, {4}, SDD(1, {2}, SDD(0, {2}, one))), one);
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder().push("j", order_builder().push("i", order_builder {"c", "a", "b"})));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(0, SDD(0, SDD(2, {_}, SDD(1, {1}, SDD(0, {1}, one))), one), one)
                  + SDD(0, SDD(0, SDD(2, {_}, SDD(1, {2}, SDD(0, {2}, one))), one), one);
    const auto s1 = SDD(0, SDD(0, SDD(2, {2}, SDD(1, {1}, SDD(0, {1}, one))), one), one)
                  + SDD(0, SDD(0, SDD(2, {4}, SDD(1, {2}, SDD(0, {2}, one))), one), one);
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder().push("j", order_builder().push("i", order_builder {"c", "a", "b"})));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(0, SDD(0, SDD(2, {_}, SDD(1, {1}, SDD(0, {2}, one))), one), one)
                  + SDD(0, SDD(0, SDD(2, {_}, SDD(1, {2}, SDD(0, {1}, one))), one), one);
    const auto s1 = SDD(0, SDD(0, SDD(2, {3}, SDD(1, {1}, SDD(0, {2}, one))), one), one)
                  + SDD(0, SDD(0, SDD(2, {3}, SDD(1, {2}, SDD(0, {1}, one))), one), one);
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("c")) << ob("j", ob("a")) << ob("k", ob("b")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, SDD(0, {4}, one)
                  , one)))
                  + SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {4}, one)
                  , SDD(0, SDD(0, {3}, one)
                  , one)));
    const auto s1 = SDD(2, SDD(0, {7}, one)
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, SDD(0, {4}, one)
                  , one)))
                  + SDD(2, SDD(0, {7}, one)
                  , SDD(1, SDD(0, {4}, one)
                  , SDD(0, SDD(0, {3}, one)
                  , one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("c")) << ob("j", ob("a")) << ob("k", ob("b")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, SDD(0, {4}, one)
                  , one)))
                  + SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {4}, one)
                  , SDD(0, SDD(0, {3}, one)
                  , one)))
                  + SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one)));
    const auto s1 = SDD(2, SDD(0, {7}, one)
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, SDD(0, {4}, one)
                  , one)))
                  + SDD(2, SDD(0, {7}, one)
                  , SDD(1, SDD(0, {4}, one)
                  , SDD(0, SDD(0, {3}, one)
                  , one)))
                  + SDD(2, SDD(0, {4}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("c")) << ob("j", ob("z")) << ob("k", ob("b")) << ob("l", ob("a")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(3, SDD(0, {_}, one)
                  , SDD(2, SDD(0, {x}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one))))
                  + SDD(3, SDD(0, {_}, one)
                  , SDD(2, SDD(0, {y}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one))));
    const auto s1 = SDD(3, SDD(0, {2}, one)
                  , SDD(2, SDD(0, {x}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one))))
                  + SDD(3, SDD(0, {4}, one)
                  , SDD(2, SDD(0, {y}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("c")) << ob("j", ob("a")) << ob("k", ob("b")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0, {x}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one)))
                  + SDD(2, SDD(0, {x}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one)))
                  + SDD(2, SDD(0, {y}, one)
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, SDD(0, {3}, one)
                  , one)));
    const auto s1 = SDD(2, SDD(0, {2}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one)))
                  + SDD(2, SDD(0, {4}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one)))
                  + SDD(2, SDD(0, {6}, one)
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, SDD(0, {3}, one)
                  , one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const auto l2 = {"a", "b", "c"};
    const order o(ob("i", ob("c")) << ob("j", ob("a")) << ob("k", ob("b")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast2), l2.begin(), l2.end(), "c");
    const auto s0 = SDD(2, SDD(0, {1}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one)))
                  + SDD(2, SDD(0, {1}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one)))
                  + SDD(2, SDD(0, {3}, one)
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, SDD(0, {3}, one)
                  , one)));
    const auto s1 = SDD(2, SDD(0, {3}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one)))
                  + SDD(2, SDD(0, {5}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one)))
                  + SDD(2, SDD(0, {9}, one)
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, SDD(0, {3}, one)
                  , one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const auto l2 = {"a", "b", "c", "d"};
    const order o(ob("d") << ob("i", ob("c")) << ob("j", ob("a")) << ob("k", ob("b")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast3), l2.begin(), l2.end(), "c");
    const auto s0 = SDD(3, {1}
                  , SDD(2, SDD(0, {1}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one))))
                  + SDD(3, {1}
                  , SDD(2, SDD(0, {1}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one))))
                  + SDD(3, {2}
                  , SDD(2, SDD(0, {3}, one)
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, SDD(0, {3}, one)
                  , one))));
    const auto s1 = SDD(3, {1}
                  , SDD(2, SDD(0, {4}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one))))
                  + SDD(3, {1}
                  , SDD(2, SDD(0, {6}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one))))
                  + SDD(3, {2}
                  , SDD(2, SDD(0, {11}, one)
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, SDD(0, {3}, one)
                  , one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const auto l2 = {"a", "b", "c", "d"};
    const order o(ob("l",ob("d")) << ob("i", ob("c")) << ob("j", ob("a")) << ob("k", ob("b")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast3), l2.begin(), l2.end(), "c");
    const auto s0 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {1}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one))))
                  + SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {1}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one))))
                  + SDD(3, SDD(0, {2}, one)
                  , SDD(2, SDD(0, {3}, one)
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, SDD(0, {3}, one)
                  , one))));
    const auto s1 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {4}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one))))
                  + SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {6}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one))))
                  + SDD(3, SDD(0, {2}, one)
                  , SDD(2, SDD(0, {11}, one)
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, SDD(0, {3}, one)
                  , one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const auto l2 = {"a", "b", "c", "d"};
    const order o(ob("l",ob("d")) << ob("c") << ob("j", ob("a")) << ob("k", ob("b")));
    const auto h = Expression<conf>(o, evaluator<conf>(ast3), l2.begin(), l2.end(), "c");
    const auto s0 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, {1}
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one))))
                  + SDD(3, SDD(0, {1}, one)
                  , SDD(2, {1}
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one))))
                  + SDD(3, SDD(0, {2}, one)
                  , SDD(2, {3}
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, SDD(0, {3}, one)
                  , one))));
    const auto s1 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, {4}
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {1}, one)
                  , one))))
                  + SDD(3, SDD(0, {1}, one)
                  , SDD(2, {6}
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one))))
                  + SDD(3, SDD(0, {2}, one)
                  , SDD(2, {11}
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, SDD(0, {3}, one)
                  , one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const auto l2 = {"a", "b", "c", "d"};
    const order o(ob("l",ob("d")) << ob("c") << ob("j", ob("a")) << ob("b"));
    const auto h = Expression<conf>(o, evaluator<conf>(ast3), l2.begin(), l2.end(), "c");
    const auto s0 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, {1}
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, {1}
                  , one))))
                  + SDD(3, SDD(0, {1}, one)
                  , SDD(2, {1}
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, {2}
                  , one))))
                  + SDD(3, SDD(0, {2}, one)
                  , SDD(2, {3}
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, {3}
                  , one))));
    const auto s1 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, {4}
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, {1}
                  , one))))
                  + SDD(3, SDD(0, {1}, one)
                  , SDD(2, {6}
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, {2}
                  , one))))
                  + SDD(3, SDD(0, {2}, one)
                  , SDD(2, {11}
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, {3}
                  , one))));
    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
