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
struct hom_com_composition_test
  : public testing::Test
{
  typedef C configuration_type;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_com_composition_test()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::id<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_com_composition_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_com_composition_test, construction)
{
  order empty_order(order_builder {});
  {
    ASSERT_EQ(id, com_composition(empty_order, {id}));
  }
  {
    ASSERT_EQ(id, com_composition(empty_order, {id, id, id}));
  }
  {
    const auto h = inductive<conf>(targeted_incr<conf>("0",1));
    ASSERT_EQ(h, com_composition(empty_order, {id, h, id}));
    ASSERT_EQ(h, com_composition(empty_order, {h, id, h}));
  }
  {
    const auto h1 = inductive<conf>(targeted_incr<conf>("0",1));
    const auto h2 = inductive<conf>(targeted_incr<conf>("0",2));
    ASSERT_EQ( com_composition(empty_order, {id, h1, h2, h1})
             , com_composition(empty_order, {h2, id, h1}));
  }
  {
    order o = order_builder("a", order_builder("0"));
    const auto n1 = inductive<conf>(targeted_incr<conf>("0",1));
    const auto n2 = inductive<conf>(targeted_incr<conf>("0",2));
    const auto h1 = local("a", o, n1);
    const auto h2 = local("a", o, n2);

    ASSERT_EQ( com_composition(o, {h1, h2})
             , local("a", o, com_composition(o, {n1, n2})));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_com_composition_test, evaluation)
{
  {
    order o = order_builder {"a", "b", "c"};
    const auto h1 = inductive<conf>(targeted_incr<conf>("a",1));
    const auto h2 = inductive<conf>(targeted_incr<conf>("b",1));
    const auto h3 = inductive<conf>(targeted_incr<conf>("c",1));
    SDD s0(o, [](const std::string&){return values_type {0};});
    const auto h = com_composition(o, {h1, h2, h3});
    SDD s1(o, [](const std::string&){return values_type {1};});
    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_com_composition_test, saturation)
{
  {
    order o = order_builder {"a", "b", "c"};
    const auto h1 = inductive<conf>(targeted_incr<conf>("a",1));
    const auto h2 = inductive<conf>(targeted_incr<conf>("b",1));
    const auto h3 = inductive<conf>(targeted_incr<conf>("c",1));
    SDD s0(o, [](const std::string&){return values_type {0};});
    const auto h = rewrite(o, com_composition(o, {h1, h2, h3}));
    SDD s1(o, [](const std::string&){return values_type {1};});
    ASSERT_EQ(s1, h(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
