#include <vector>

#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"
//#include "sdd/hom/rewrite.hh"
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
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_sum_test()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::id<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_sum_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_sum_test, construction)
{
  {
    const homomorphism h1 = sum(order(order_builder()), {id});
    const homomorphism h2 = sum(order(order_builder()), {id});
    ASSERT_EQ(h1, h2);
  }
  {
    const homomorphism h1 = sum(order(order_builder()), {id});
    ASSERT_EQ(id, h1);
  }
  {

    std::vector<homomorphism> empty;
    ASSERT_THROW( sum<conf>(order(order_builder()), empty.begin(), empty.end())
                , std::invalid_argument);
  }
  {
    order_builder ob {"0"};
    order o(ob);
    const homomorphism h1 = sum(o, {cons(o, values_type {0,1}, id)});
    ASSERT_EQ(cons(o, values_type {0,1}, id), h1);
  }
  {
    const homomorphism h1 = sum(order(order_builder()), {id});
    const homomorphism h2 = sum(order(order_builder()), {id, id});
    ASSERT_EQ(h1, h2);
  }
  {
    order_builder ob {"0"};
    order o(ob);
    const homomorphism h1 = sum(o, {id, cons(o, values_type {0,1}, id)});
    const homomorphism h2 = sum(o, {id, cons(o, values_type {0,2}, id)});
    ASSERT_NE(h1, h2);
  }
  {
    order_builder ob {"0", "1", "2"};
    order o(ob);
    const homomorphism h1 = sum(o, {id, inductive<conf>(ind<conf>("0")), id});
    const homomorphism h2 = sum(o, {inductive<conf>(ind<conf>("1")), inductive<conf>(ind<conf>("2"))});
    const homomorphism h3 = sum(o, {h1, h2, id});
    const homomorphism h4 = sum(o, { id, inductive<conf>(ind<conf>("0")), inductive<conf>(ind<conf>("1"))
                                    , inductive<conf>(ind<conf>("2"))});
    ASSERT_EQ(h4, h3);
  }
  {
    order_builder ob {"0", "1", "2"};
    order o(ob);
    const homomorphism l1 = local("0", o, inductive<conf>(ind<conf>("0")));
    const homomorphism l2 = local("0", o, inductive<conf>(ind<conf>("1")));
    const homomorphism s1 = sum(o, {l1, l2});
    const homomorphism l3 = local("0", o, sum(o, { inductive<conf>(ind<conf>("0"))
                                        , inductive<conf>(ind<conf>("1"))}));
    ASSERT_EQ(s1, l3);
  }
  {
    order_builder ob {"0", "1", "2"};
    order o(ob);
    const homomorphism l1 = local("0", o, inductive<conf>(ind<conf>("0")));
    const homomorphism l2 = local("0", o, inductive<conf>(ind<conf>("1")));
    const homomorphism l3 = local("1", o, inductive<conf>(ind<conf>("2")));
    const homomorphism s1 = sum(o, {l1, l2, l3});
    const homomorphism s2 = sum(o, { local( "0", o
                                 , sum(o, { inductive<conf>(ind<conf>("0"))
                                          , inductive<conf>(ind<conf>("1"))
                                          })
                                 )
                          , local("1", o, inductive<conf>(ind<conf>("2")))
                          });

    ASSERT_EQ(s1, s2);
  }

}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_sum_test, evaluation)
{
  {
    const homomorphism h = sum(order(order_builder()), {id});
    ASSERT_EQ(one, h(order(order_builder()), one));
  }
  {
    const homomorphism h = sum(order(order_builder()), {id});
    ASSERT_EQ(zero, h(order(order_builder()), zero));
  }
}

/*------------------------------------------------------------------------------------------------*/
