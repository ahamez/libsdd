#include <vector>

#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"
#include "sdd/manager.hh"
#include "sdd/order/order.hh"

#include "tests/configuration.hh"
#include "tests/hom/common.hh"
#include "tests/hom/common_inductives.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct hom_xsum_test
  : public testing::Test
{
  typedef C configuration_type;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_xsum_test()
    : m(sdd::manager<C>::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::id<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_xsum_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_xsum_test, construction)
{
  {
    const homomorphism h1 = xsum(order(order_builder()), {id});
    const homomorphism h2 = xsum(order(order_builder()), {id});
    ASSERT_EQ(h1, h2);
  }
  {
    const homomorphism h1 = xsum(order(order_builder()), {id});
    ASSERT_EQ(id, h1);
  }
  {

    std::vector<homomorphism> empty;
    ASSERT_THROW( xsum(order(order_builder()), empty.begin(), empty.end())
                , std::invalid_argument);
  }
  {
    order o(order_builder {"a"});
    const homomorphism h1 = xsum(o, {cons(o, values_type {0,1}, id)});
    ASSERT_EQ(cons(o, values_type {0,1}, id), h1);
  }
  {
    const homomorphism h1 = xsum(order(order_builder()), {id});
    const homomorphism h2 = xsum(order(order_builder()), {id, id});
    ASSERT_EQ(h1, h2);
  }
  {
    order o(order_builder {"a"});
    const homomorphism h1 = xsum(o, {cons(o, values_type {0,1}, id)});
    const homomorphism h2 = xsum(o, {cons(o, values_type {0,2}, id)});
    ASSERT_NE(h1, h2);
  }
  {
    order o(order_builder {"0", "1", "2"});
    const homomorphism h1 = inductive<conf>(ind<conf>("0"));
    const homomorphism h2 = xsum(o, { inductive<conf>(ind<conf>("1"))
                                    , inductive<conf>(ind<conf>("2"))});
    const homomorphism h3 = xsum(o, {h1, h2});
    const homomorphism h4 = xsum(o, { inductive<conf>(ind<conf>("0"))
                                    , inductive<conf>(ind<conf>("1"))
                                    , inductive<conf>(ind<conf>("2"))});
    ASSERT_EQ(h4, h3);
  }
  {
    order o(order_builder {"0", "1", "2"});
    const homomorphism l1 = local("0", o, inductive<conf>(ind<conf>("0")));
    const homomorphism l2 = local("0", o, inductive<conf>(ind<conf>("1")));
    const homomorphism s1 = xsum(o, {l1, l2});
    const homomorphism l3 = local("0", o, xsum(o, { inductive<conf>(ind<conf>("0"))
                                                  , inductive<conf>(ind<conf>("1"))}));
    ASSERT_EQ(s1, l3);
  }
  {
    order o(order_builder {"0", "1", "2"});
    const homomorphism l1 = local("0", o, inductive<conf>(ind<conf>("0")));
    const homomorphism l2 = local("0", o, inductive<conf>(ind<conf>("1")));
    const homomorphism l3 = local("1", o, inductive<conf>(ind<conf>("2")));
    const homomorphism s1 = xsum(o, {l1, l2, l3});
    const homomorphism s2 = xsum(o, { local( "0", o
                                           , xsum(o, { inductive<conf>(ind<conf>("0"))
                                                     , inductive<conf>(ind<conf>("1"))
                                                     })
                                           )
                                    , local("1", o, inductive<conf>(ind<conf>("2")))
                                    });

    ASSERT_EQ(s1, s2);
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_xsum_test, evaluation)
{
  {
    const homomorphism h = xsum(order(order_builder()), {id});
    ASSERT_EQ(one, h(order(order_builder()), one));
  }
  {
    const homomorphism h = xsum(order(order_builder()), {id});
    ASSERT_EQ(zero, h(order(order_builder()), zero));
  }
  {
    order o(order_builder {"a", "b", "c"});
    const auto s0 = SDD(o, [](const std::string&){return values_type {0};});
    const auto s1 = SDD(o, [](const std::string& x)
                             {
                               return x == "b" ? values_type {1} : values_type {0};
                             });

    const auto i0 = sdd::inductive<conf>(targeted_incr<conf>("b", 1));
    const auto h0 = xsum(o, {i0, constant(zero)});
    ASSERT_EQ(s1, h0(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
