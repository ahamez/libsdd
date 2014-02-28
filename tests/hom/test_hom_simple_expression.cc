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

TYPED_TEST(hom_expression_test, simple_construction)
{
  {
    const auto l = {"a", "b"};
    order o(order_builder {"a", "b"});
    const auto h1 = expression(o, evaluator<conf>(ast1), l.begin(), l.end(), "a");
    const auto h2 = expression(o, evaluator<conf>(ast1), l.begin(), l.end(), "a");
    ASSERT_EQ(h1, h2);
  }
  {
    const auto l = {"a", "b"};
    order o(order_builder {"a", "b"});
    ASSERT_EQ(expression(o, evaluator<conf>(ast1), l.begin(), l.begin(), "a"), id);
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_expression_test, simple_flat_one_path)
{
  const auto _ = 42; // don't care value
  {
    const auto l = {"a", "b"};
    order o(order_builder {"a", "b", "c"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    SDD s0(2, {1}, SDD(1, {1}, SDD(0, {_}, one)));
    SDD s1(2, {1}, SDD(1, {1}, SDD(0, {2}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const auto l = {"a", "b"};
    order o(order_builder {"a", "b", "c"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    SDD s0(2, {1,2}, SDD(1, {2,3}, SDD(0, {_}    , one)));
    SDD s1(2, {1,2}, SDD(1, {2,3}, SDD(0, {3,4,5}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const auto l = {"a", "b"};
    order o(order_builder {"b", "a", "c"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    SDD s0(2, {1}, SDD(1, {1}, SDD(0, {_}, one)));
    SDD s1(2, {1}, SDD(1, {1}, SDD(0, {2}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const auto l = {"a", "b"};
    order o(order_builder {"a", "b", "c", "y"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    SDD s0(3, {1}, SDD(2, {1}, SDD(1, {_}, SDD(0, {_}, one))));
    SDD s1(3, {1}, SDD(2, {1}, SDD(1, {2}, SDD(0, {_}, one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const auto l = {"a", "b"};
    order o(order_builder {"a", "b", "x", "y", "c"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    SDD s0(4, {1}, SDD(3, {1}, SDD(2, {_}, SDD(1, {_}, SDD(0, {_}, one)))));
    SDD s1(4, {1}, SDD(3, {1}, SDD(2, {_}, SDD(1, {_}, SDD(0, {2}, one)))));
    ASSERT_EQ(s1, h(o, s0));
  }
  // Order changes (identifiers), but not variables. We must reset the homomorphism cache.
  this->m.reset_hom_cache();
  {
    const auto l = {"a", "b"};
    order o(order_builder {"a", "b", "x", "c", "y"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    SDD s0(4, {1}, SDD(3, {1}, SDD(2, {_}, SDD(1, {_}, SDD(0, {_}, one)))));
    SDD s1(4, {1}, SDD(3, {1}, SDD(2, {_}, SDD(1, {2}, SDD(0, {_}, one)))));
    ASSERT_EQ(s1, h(o, s0));
  }
  this->m.reset_hom_cache();
  {
    const auto l = {"a", "b"};
    order o(order_builder {"z", "a", "b", "x", "c", "y"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    SDD s0(5, {_}, SDD(4, {1}, SDD(3, {1}, SDD(2, {_}, SDD(1, {_}, SDD(0, {_}, one))))));
    SDD s1(5, {_}, SDD(4, {1}, SDD(3, {1}, SDD(2, {_}, SDD(1, {2}, SDD(0, {_}, one))))));
    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_expression_test, simple_flat_one_path_self)
{
  const auto _ = 42; // don't care value
  {
    const auto l = {"a", "b"};
    order o(order_builder {"a", "b"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "b");
    SDD s0(1, {1}, SDD(0, {1}, one));
    SDD s1(1, {1}, SDD(0, {2}, one));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const auto l = {"a", "b"};
    order o(order_builder {"a", "b","y"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "b");
    SDD s0(2, {1}, SDD(1, {2}, SDD(0, {_}, one)));
    SDD s1(2, {1}, SDD(1, {3}, SDD(0, {_}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const auto l = {"a", "b"};
    order o(order_builder {"a", "x", "y", "b"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "b");
    SDD s0(3, {1}, SDD(2, {_}, SDD(1, {_}, SDD(0, {7}, one))));
    SDD s1(3, {1}, SDD(2, {_}, SDD(1, {_}, SDD(0, {8}, one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  // Order changes (identifiers), but not variables. We must reset the homomorphism cache.
  this->m.reset_hom_cache();
  {
    const auto l = {"a", "b"};
    order o(order_builder {"a", "x", "b", "y"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "b");
    SDD s0(3, {1}, SDD(2, {_}, SDD(1, {4}, SDD(0, {_}, one))));
    SDD s1(3, {1}, SDD(2, {_}, SDD(1, {5}, SDD(0, {_}, one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const auto l = {"a", "b"};
    order o(order_builder {"z", "a", "x", "b", "y"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "b");
    SDD s0(4, {1}, SDD(3, {1}, SDD(2, {_}, SDD(1, {5}, SDD(0, {_}, one)))));
    SDD s1(4, {1}, SDD(3, {1}, SDD(2, {_}, SDD(1, {6}, SDD(0, {_}, one)))));
    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_expression_test, simple_flat)
{
  const auto l = {"a", "b"};
  const auto _  = 21; // don't care value
  const auto xx = 42; // don't care value
  const auto yy = 33; // don't care value
  {
    order o(order_builder {"a", "b", "c"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, {1}, SDD(1, {1}, SDD(0, {xx}, one)))
                  + SDD(2, {2}, SDD(1, {2}, SDD(0, {yy}, one)));
    const auto s1 = SDD(2, {1}, SDD(1, {1}, SDD(0, {2}, one)))
                  + SDD(2, {2}, SDD(1, {2}, SDD(0, {4}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, {1}, SDD(1, {2}, SDD(0, {xx}, one)))
                  + SDD(2, {2}, SDD(1, {1}, SDD(0, {yy}, one)));
    const auto s1 = SDD(2, {1}, SDD(1, {2}, SDD(0, {3}, one)))
                  + SDD(2, {2}, SDD(1, {1}, SDD(0, {3}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, {1}, SDD(1, {2}, SDD(0, {_}, one)))
                  + SDD(2, {2}, SDD(1, {1}, SDD(0, {_}, one)));
    const auto s1 = SDD(2, {1}, SDD(1, {2}, SDD(0, {3}, one)))
                  + SDD(2, {2}, SDD(1, {1}, SDD(0, {3}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, {1}, SDD(1, {1,2}, SDD(0, {_}, one)));
    const auto s1 = SDD(2, {1}, SDD(1, {1,2}, SDD(0, {2,3}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, {1}, SDD(1, {1}, SDD(0, {_}, one)))
                  + SDD(2, {2}, SDD(1, {2}, SDD(0, {_}, one)));
    const auto s1 = SDD(2, {1}, SDD(1, {1}, SDD(0, {2}, one)))
                  + SDD(2, {2}, SDD(1, {2}, SDD(0, {4}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c", "y"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(3, {1}, SDD(2, {1}, SDD(1, {_}, SDD(0, {_}, one))))
                  + SDD(3, {2}, SDD(2, {2}, SDD(1, {_}, SDD(0, {_}, one))));
    const auto s1 = SDD(3, {1}, SDD(2, {1}, SDD(1, {2}, SDD(0, {_}, one))))
                  + SDD(3, {2}, SDD(2, {2}, SDD(1, {4}, SDD(0, {_}, one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"a", "b", "c", "y"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(3, {1}, SDD(2, {1}, SDD(1, {xx}, SDD(0, {_}, one))))
                  + SDD(3, {2}, SDD(2, {2}, SDD(1, {yy}, SDD(0, {_}, one))));
    const auto s1 = SDD(3, {1}, SDD(2, {1}, SDD(1, {2},  SDD(0, {_}, one))))
                  + SDD(3, {2}, SDD(2, {2}, SDD(1, {4},  SDD(0, {_}, one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  this->m.reset_hom_cache();
  {
    order o(order_builder {"a", "b", "y", "c"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(3, {1}, SDD(2, {1}, SDD(1, {_}, SDD(0, {_}, one))))
                  + SDD(3, {2}, SDD(2, {2}, SDD(1, {_}, SDD(0, {_}, one))));
    const auto s1 = SDD(3, {1}, SDD(2, {1}, SDD(1, {_}, SDD(0, {2}, one))))
                  + SDD(3, {2}, SDD(2, {2}, SDD(1, {_}, SDD(0, {4}, one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  this->m.reset_hom_cache();
  {
    order o(order_builder {"a", "b", "x", "c", "y"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(4, {1}, SDD(3, {1}, SDD(2, {_}, SDD(1, {_}, SDD(0, {_}, one)))))
                  + SDD(4, {2}, SDD(3, {2}, SDD(2, {_}, SDD(1, {_}, SDD(0, {_}, one)))));
    const auto s1 = SDD(4, {1}, SDD(3, {1}, SDD(2, {_}, SDD(1, {2}, SDD(0, {_}, one)))))
                  + SDD(4, {2}, SDD(3, {2}, SDD(2, {_}, SDD(1, {4}, SDD(0, {_}, one)))));
    ASSERT_EQ(s1, h(o, s0));
  }
  this->m.reset_hom_cache();
  {
    order o(order_builder {"z", "a", "b", "x", "c", "y"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(5,{_},SDD(4,{1},SDD(3, {1}, SDD(2, {_}, SDD(1, {_}, SDD(0, {_}, one))))))
                  + SDD(5,{_},SDD(4,{2},SDD(3, {2}, SDD(2, {_}, SDD(1, {_}, SDD(0, {_}, one))))));
    const auto s1 = SDD(5,{_},SDD(4,{1},SDD(3, {1}, SDD(2, {_}, SDD(1, {2}, SDD(0, {_}, one))))))
                  + SDD(5,{_},SDD(4,{2},SDD(3, {2}, SDD(2, {_}, SDD(1, {4}, SDD(0, {_}, one))))));
    ASSERT_EQ(s1, h(o, s0));
  }
  this->m.reset_hom_cache();
  {
    order o(order_builder {"a", "b", "c"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one)))
                  + SDD(2, {0}, SDD(1, {1}, SDD(0, {1}, one)))
                  + SDD(2, {2}, SDD(1, {2}, SDD(0, {1}, one)));
    const auto s1 = SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one)))
                  + SDD(2, {0}, SDD(1, {1}, SDD(0, {1}, one)))
                  + SDD(2, {2}, SDD(1, {2}, SDD(0, {4}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_expression_test, simple_flat_self)
{
  const auto l = {"a", "b"};
  const auto _  = 21; // don't care value
  const auto xx = 42; // don't care value
  const auto yy = 33; // don't care value
  {
    order o(order_builder {"a", "b"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "b");
    const auto s0 = SDD(1, {1}, SDD(0, {1}, one))
                  + SDD(1, {2}, SDD(0, {2}, one));
    const auto s1 = SDD(1, {1}, SDD(0, {2},  one))
                  + SDD(1, {2}, SDD(0, {4},  one));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"a", "b", "y"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "b");
    const auto s0 = SDD(2, {1}, SDD(1, {1}, SDD(0, {_}, one)))
                  + SDD(2, {2}, SDD(1, {2}, SDD(0, {_}, one)));
    const auto s1 = SDD(2, {1}, SDD(1, {2}, SDD(0, {_}, one)))
                  + SDD(2, {2}, SDD(1, {4}, SDD(0, {_}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"a", "b", "y"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "b");
    const auto s0 = SDD(2, {1}, SDD(1, {1}, SDD(0, {xx}, one)))
                  + SDD(2, {2}, SDD(1, {2}, SDD(0, {yy}, one)));
    const auto s1 = SDD(2, {1}, SDD(1, {2}, SDD(0, {xx}, one)))
                  + SDD(2, {2}, SDD(1, {4}, SDD(0, {yy}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  this->m.reset_hom_cache();
  {
    order o(order_builder {"a", "y", "b"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "b");
    const auto s0 = SDD(2, {1}, SDD(1, {_}, SDD(0, {1}, one)))
                  + SDD(2, {2}, SDD(1, {_}, SDD(0, {2}, one)));
    const auto s1 = SDD(2, {1}, SDD(1, {_}, SDD(0, {2}, one)))
                  + SDD(2, {2}, SDD(1, {_}, SDD(0, {4}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"a", "x", "b", "y"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "b");
    const auto s0 = SDD(3, {1}, SDD(2, {_}, SDD(1, {1}, SDD(0, {_}, one))))
                  + SDD(3, {2}, SDD(2, {_}, SDD(1, {2}, SDD(0, {_}, one))));
    const auto s1 = SDD(3, {1}, SDD(2, {_}, SDD(1, {2}, SDD(0, {_}, one))))
                  + SDD(3, {2}, SDD(2, {_}, SDD(1, {4}, SDD(0, {_}, one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder {"z", "a", "x", "b", "y"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "b");
    const auto s0 = SDD(4, {xx}, SDD(3, {1}, SDD(2, {_}, SDD(1, {1}, SDD(0, {yy}, one)))))
                  + SDD(4, {yy}, SDD(3, {2}, SDD(2, {_}, SDD(1, {2}, SDD(0, {xx}, one)))));
    const auto s1 = SDD(4, {xx}, SDD(3, {1}, SDD(2, {_}, SDD(1, {2}, SDD(0, {yy}, one)))))
                  + SDD(4, {yy}, SDD(3, {2}, SDD(2, {_}, SDD(1, {4}, SDD(0, {xx}, one)))));
    ASSERT_EQ(s1, h(o, s0));
  }
  this->m.reset_hom_cache();
  {
    order o(order_builder {"a", "x", "b"});
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "b");
    const auto s0 = SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one)))
                  + SDD(2, {0}, SDD(1, {1}, SDD(0, {1}, one)))
                  + SDD(2, {2}, SDD(1, {2}, SDD(0, {1}, one)));
    const auto s1 = SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one)))
                  + SDD(2, {0}, SDD(1, {1}, SDD(0, {1}, one)))
                  + SDD(2, {2}, SDD(1, {2}, SDD(0, {3}, one)));
    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_expression_test, simple_hierarchical_one_path)
{
  const auto l = {"a", "b"};
  const auto _  = 21; // don't care value
  using ob = order_builder;
  {
    order o(ob("i", ob ({"a", "b", "c"})));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(0, SDD(2, {1}, SDD(1, {1}, SDD(0, {_}, one))), one);
    const auto s1 = SDD(0, SDD(2, {1}, SDD(1, {1}, SDD(0, {2}, one))), one);
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(ob("z") << ob("i", ob ({"a", "b", "c"})));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(1, {_}, SDD(0, SDD(2, {1}, SDD(1, {1}, SDD(0, {_}, one))), one));
    const auto s1 = SDD(1, {_}, SDD(0, SDD(2, {1}, SDD(1, {1}, SDD(0, {2}, one))), one));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(ob("i", ob("z")) << ob("j", ob({"a", "b", "c"})));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(1, SDD(0,{_},one),SDD(0,SDD(2,{1},SDD(1, {1},SDD(0,{_},one))),one));
    const auto s1 = SDD(1, SDD(0,{_},one),SDD(0,SDD(2,{1},SDD(1, {1},SDD(0,{2},one))),one));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder().push("j", order_builder().push("i", order_builder {"a", "b", "c"})));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(0, SDD(0, SDD(2, {1}, SDD(1, {1}, SDD(0, {_}, one))), one), one);
    const auto s1 = SDD(0, SDD(0, SDD(2, {1}, SDD(1, {1}, SDD(0, {2}, one))), one), one);
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("a")) << ob("j", ob("b")) << ob("k", ob("c")));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0, {1}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {_}, one)
                  , one)));
    const auto s1 = SDD(2, SDD(0, {1}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one)));

    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("a")) << ob("j", ob("z")) << ob("k", ob("b")) << ob("l", ob("c")));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {_}, one)
                  , one))));
    const auto s1 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one))));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(  ob("i", ob("j", ob("a")))
                 << ob("k", ob("l", ob("m", ob("b"))))
                 << ob("n", ob("c")));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0,SDD(0, {1}, one),one),
                    SDD(1, SDD(0, SDD(0, SDD(0, {2}, one),one),one),
                    SDD(0, SDD(0, {_}, one), one)
                    ));
    const auto s1 = SDD(2, SDD(0,SDD(0, {1}, one),one),
                    SDD(1, SDD(0, SDD(0, SDD(0, {2}, one),one),one),
                    SDD(0, SDD(0, {3}, one), one)
                    ));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("a")) << ob("b") << ob("k", ob("c")));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0, {1}, one)
                  , SDD(1, {1}
                  , SDD(0, SDD(0, {_}, one)
                  , one)));
    const auto s1 = SDD(2, SDD(0, {1}, one)
                  , SDD(1, {1}
                  , SDD(0, SDD(0, {2}, one)
                  , one)));

    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("a")) << ob("j", ob("b")) << ob("k", ob({"c", "x"})));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0, {2}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(1, {_}, SDD(0, {_}, one))
                  , one)));
    const auto s1 = SDD(2, SDD(0, {2}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(1, {3}, SDD(0, {_}, one))
                  , one)));

    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("a")) << ob("j", ob("b")) << ob("k", ob({"y", "c", "x"})));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0, {2}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(2, {_}, SDD(1, {_}, SDD(0, {_}, one)))
                  , one)));
    const auto s1 = SDD(2, SDD(0, {2}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(2, {_}, SDD(1, {3}, SDD(0, {_}, one)))
                  , one)));

    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("a")) << ob("j", ob({"b", "z"})) << ob("k", ob({"y", "c", "x"})));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0, {2}, one)
                  , SDD(1, SDD(1, {1}, SDD(0, {_}, one))
                  , SDD(0, SDD(2, {_}, SDD(1, {_}, SDD(0, {_}, one)))
                  , one)));
    const auto s1 = SDD(2, SDD(0, {2}, one)
                  , SDD(1, SDD(1, {1}, SDD(0, {_}, one))
                  , SDD(0, SDD(2, {_}, SDD(1, {3}, SDD(0, {_}, one)))
                  , one)));

    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("a")) << ob("j", ob("b")) << ob("c"));

    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0, {1}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, {_}
                  , one)));
    const auto s1 = SDD(2, SDD(0, {1}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, {2}
                  , one)));

    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_expression_test, simple_hierarchical)
{
  const auto l = {"a", "b"};
  const auto _  = 21; // don't care value
  const auto xx = 42;
  using ob = order_builder;
  {
    order o(ob("i", ob ({"a", "b", "c"})));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(0, SDD(2, {1}, SDD(1, {1}, SDD(0, {_}, one))), one)
                  + SDD(0, SDD(2, {2}, SDD(1, {2}, SDD(0, {_}, one))), one);
    const auto s1 = SDD(0, SDD(2, {1}, SDD(1, {1}, SDD(0, {2}, one))), one)
                  + SDD(0, SDD(2, {2}, SDD(1, {2}, SDD(0, {4}, one))), one);
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(ob("z") << ob("i", ob ({"a", "b", "c"})));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(1, {_}, SDD(0, SDD(2, {1}, SDD(1, {1}, SDD(0, {_}, one))), one))
                  + SDD(1, {_}, SDD(0, SDD(2, {2}, SDD(1, {2}, SDD(0, {_}, one))), one));
    const auto s1 = SDD(1, {_}, SDD(0, SDD(2, {1}, SDD(1, {1}, SDD(0, {2}, one))), one))
                  + SDD(1, {_}, SDD(0, SDD(2, {2}, SDD(1, {2}, SDD(0, {4}, one))), one));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(ob("z") << ob("i", ob ({"a", "b", "c"})));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(1, {_},  SDD(0, SDD(2, {1}, SDD(1, {1}, SDD(0, {_}, one))), one))
                  + SDD(1, {_},  SDD(0, SDD(2, {2}, SDD(1, {2}, SDD(0, {_}, one))), one))
                  + SDD(1, {xx}, SDD(0, SDD(2, {3}, SDD(1, {2}, SDD(0, {_}, one))), one));
    const auto s1 = SDD(1, {_},  SDD(0, SDD(2, {1}, SDD(1, {1}, SDD(0, {2}, one))), one))
                  + SDD(1, {_},  SDD(0, SDD(2, {2}, SDD(1, {2}, SDD(0, {4}, one))), one))
                  + SDD(1, {xx}, SDD(0, SDD(2, {3}, SDD(1, {2}, SDD(0, {5}, one))), one));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    order o(order_builder().push("j", order_builder().push("i", order_builder {"a", "b", "c"})));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(0, SDD(0, SDD(2, {1}, SDD(1, {1}, SDD(0, {_}, one))), one), one)
                  + SDD(0, SDD(0, SDD(2, {2}, SDD(1, {2}, SDD(0, {_}, one))), one), one);
    const auto s1 = SDD(0, SDD(0, SDD(2, {1}, SDD(1, {1}, SDD(0, {2}, one))), one), one)
                  + SDD(0, SDD(0, SDD(2, {2}, SDD(1, {2}, SDD(0, {4}, one))), one), one);
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("a")) << ob("j", ob("b")) << ob("k", ob("c")));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0, {1}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {_}, one)
                  , one)))
                  + SDD(2, SDD(0, {2}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {_}, one)
                  , one)));
    const auto s1 = SDD(2, SDD(0, {1}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one)))
                  + SDD(2, SDD(0, {2}, one)
                  , SDD(1, SDD(0, {2}, one)
                  , SDD(0, SDD(0, {4}, one)
                  , one)));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(  ob("i", ob("j", ob("a")))
                 << ob("k", ob("l", ob("m", ob("b"))))
                 << ob("n", ob("c")));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0,SDD(0, {1}, one),one),
                    SDD(1, SDD(0, SDD(0, SDD(0, {2}, one),one),one),
                    SDD(0, SDD(0, {_}, one), one)
                    ))
                  + SDD(2, SDD(0,SDD(0, {3}, one),one),
                    SDD(1, SDD(0, SDD(0, SDD(0, {3}, one),one),one),
                    SDD(0, SDD(0, {_}, one), one)
                    ));
    const auto s1 = SDD(2, SDD(0,SDD(0, {1}, one),one),
                    SDD(1, SDD(0, SDD(0, SDD(0, {2}, one),one),one),
                    SDD(0, SDD(0, {3}, one), one)
                    ))
                  + SDD(2, SDD(0,SDD(0, {3}, one),one),
                    SDD(1, SDD(0, SDD(0, SDD(0, {3}, one),one),one),
                    SDD(0, SDD(0, {6}, one), one)
                    ));
    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("a")) << ob("j", ob({"b", "z"})) << ob("k", ob({"y", "c", "x"})));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(2, SDD(0, {2}, one)
                  , SDD(1, SDD(1, {1}, SDD(0, {_}, one))
                  , SDD(0, SDD(2, {_}, SDD(1, {_}, SDD(0, {_}, one)))
                  , one)))
                  + SDD(2, SDD(0, {3}, one)
                  , SDD(1, SDD(1, {4}, SDD(0, {_}, one))
                  , SDD(0, SDD(2, {_}, SDD(1, {_}, SDD(0, {_}, one)))
                  , one)));
    const auto s1 = SDD(2, SDD(0, {2}, one)
                  , SDD(1, SDD(1, {1}, SDD(0, {_}, one))
                  , SDD(0, SDD(2, {_}, SDD(1, {3}, SDD(0, {_}, one)))
                  , one)))
                  + SDD(2, SDD(0, {3}, one)
                  , SDD(1, SDD(1, {4}, SDD(0, {_}, one))
                  , SDD(0, SDD(2, {_}, SDD(1, {7}, SDD(0, {_}, one)))
                  , one)));

    ASSERT_EQ(s1, h(o, s0));
  }
  {
    const order o(ob("i", ob("a")) << ob("j", ob("z")) << ob("k", ob("b")) << ob("l", ob("c")));
    const auto h = expression<conf>(o, evaluator<conf>(ast1), l.begin(), l.end(), "c");
    const auto s0 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {_}, one)
                  , one))))
                  + SDD(3, SDD(0, {2}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, SDD(0, {_}, one)
                  , one))));
    const auto s1 = SDD(3, SDD(0, {1}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {1}, one)
                  , SDD(0, SDD(0, {2}, one)
                  , one))))
                  + SDD(3, SDD(0, {2}, one)
                  , SDD(2, SDD(0, {_}, one)
                  , SDD(1, SDD(0, {3}, one)
                  , SDD(0, SDD(0, {5}, one)
                  , one))));
    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
