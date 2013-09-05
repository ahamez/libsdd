#include <vector>

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
struct hom_sum_test
  : public testing::Test
{
  typedef C configuration_type;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_sum_test()
    : m(sdd::manager<C>::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::Id<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_sum_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_sum_test, construction)
{
  {
    const homomorphism h1 = Sum(order(order_builder()), {id});
    const homomorphism h2 = Sum(order(order_builder()), {id});
    ASSERT_EQ(h1, h2);
  }
  {
    const homomorphism h1 = Sum(order(order_builder()), {id});
    ASSERT_EQ(id, h1);
  }
  {

    std::vector<homomorphism> empty;
    ASSERT_THROW( Sum<conf>(order(order_builder()), empty.begin(), empty.end())
                , std::invalid_argument);
  }
  {
    order_builder ob {"0"};
    order o(ob);
    const homomorphism h1 = Sum(o, {Cons(o, values_type {0,1}, id)});
    ASSERT_EQ(Cons(o, values_type {0,1}, id), h1);
  }
  {
    const homomorphism h1 = Sum(order(order_builder()), {id});
    const homomorphism h2 = Sum(order(order_builder()), {id, id});
    ASSERT_EQ(h1, h2);
  }
  {
    order_builder ob {"0"};
    order o(ob);
    const homomorphism h1 = Sum(o, {id, Cons(o, values_type {0,1}, id)});
    const homomorphism h2 = Sum(o, {id, Cons(o, values_type {0,2}, id)});
    ASSERT_NE(h1, h2);
  }
  {
    order_builder ob {"0", "1", "2"};
    order o(ob);
    const homomorphism h1 = Sum(o, {id, Inductive<conf>(ind<conf>("0")), id});
    const homomorphism h2 = Sum(o, {Inductive<conf>(ind<conf>("1")), Inductive<conf>(ind<conf>("2"))});
    const homomorphism h3 = Sum(o, {h1, h2, id});
    const homomorphism h4 = Sum(o, { id, Inductive<conf>(ind<conf>("0")), Inductive<conf>(ind<conf>("1"))
                                    , Inductive<conf>(ind<conf>("2"))});
    ASSERT_EQ(h4, h3);
  }
  {
    order_builder ob {"0", "1", "2"};
    order o(ob);
    const homomorphism l1 = Local("0", o, Inductive<conf>(ind<conf>("0")));
    const homomorphism l2 = Local("0", o, Inductive<conf>(ind<conf>("1")));
    const homomorphism s1 = Sum(o, {l1, l2});
    const homomorphism l3 = Local("0", o, Sum(o, { Inductive<conf>(ind<conf>("0"))
                                        , Inductive<conf>(ind<conf>("1"))}));
    ASSERT_EQ(s1, l3);
  }
  {
    order_builder ob {"0", "1", "2"};
    order o(ob);
    const homomorphism l1 = Local("0", o, Inductive<conf>(ind<conf>("0")));
    const homomorphism l2 = Local("0", o, Inductive<conf>(ind<conf>("1")));
    const homomorphism l3 = Local("1", o, Inductive<conf>(ind<conf>("2")));
    const homomorphism s1 = Sum(o, {l1, l2, l3});
    const homomorphism s2 = Sum(o, { Local( "0", o
                                 , Sum(o, { Inductive<conf>(ind<conf>("0"))
                                          , Inductive<conf>(ind<conf>("1"))
                                          })
                                 )
                          , Local("1", o, Inductive<conf>(ind<conf>("2")))
                          });

    ASSERT_EQ(s1, s2);
  }

}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_sum_test, evaluation)
{
  {
    const homomorphism h = Sum(order(order_builder()), {id});
    ASSERT_EQ(one, h(order(order_builder()), one));
  }
  {
    const homomorphism h = Sum(order(order_builder()), {id});
    ASSERT_EQ(zero, h(order(order_builder()), zero));
  }
}

/*------------------------------------------------------------------------------------------------*/
