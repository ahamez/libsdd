#include "gtest/gtest.h"

#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"
#include "sdd/manager.hh"
#include "sdd/order/order.hh"

#include "tests/configuration.hh"
#include "tests/hom/common.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct hom_id_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;
  sdd::hom::context<C>& cxt;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;

  hom_id_test()
    : m(sdd::init(small_conf<C>()))
    , cxt(sdd::global<C>().hom_context)
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_id_test, configurations);

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_id_test, construction)
{
  using conf = typename TestFixture::configuration_type;
  const auto h1 = sdd::id<conf>();
  const auto h2 = sdd::id<conf>();
  ASSERT_EQ(h1, h2);
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_id_test, evaluation)
{
  using conf = typename TestFixture::configuration_type;
  const auto h = sdd::id<conf>();
  ASSERT_EQ(this->one, h(sdd::order<conf>(sdd::order_builder<conf>()), this->one));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_id_test, no_cache)
{
  using conf = typename TestFixture::configuration_type;
  const auto h = sdd::id<conf>();
  ASSERT_EQ(0u, this->cxt.cache().size());
  ASSERT_EQ(this->one, h(this->cxt, sdd::order<conf>(sdd::order_builder<conf>()), this->one));
  ASSERT_EQ(0u, this->cxt.cache().size());
}

/*------------------------------------------------------------------------------------------------*/
