#include "gtest/gtest.h"

#include "sdd/dd/context.hh"
#include "sdd/tools/nodes.hh"
#include "sdd/manager.hh"

#include "tests/configuration.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct nodes_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;

  nodes_test()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(nodes_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(nodes_test, terminal_zero)
{
  ASSERT_EQ(std::make_pair(0u, 0u), sdd::tools::nodes(zero));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(nodes_test, terminal_one)
{
  ASSERT_EQ(std::make_pair(0u, 0u), sdd::tools::nodes(one));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(nodes_test, flat_sdd)
{
  {
    const SDD x(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))));
    ASSERT_EQ(std::make_pair(4u, 0u), sdd::tools::nodes(x));
  }
  {
    const SDD x = SDD(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))))
                + SDD(3, {1}, SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one))));
    ASSERT_EQ(std::make_pair(7u, 0u), sdd::tools::nodes(x));
  }
  {
    const SDD x = SDD(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))))
                + SDD(3, {1}, SDD(2, {1}, SDD(1, {1}, SDD(0, {0}, one))));
    ASSERT_EQ(std::make_pair(6u, 0u), sdd::tools::nodes(x));
  }
  {
    const SDD x = SDD(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))))
                + SDD(3, {0}, SDD(2, {1}, SDD(1, {1}, SDD(0, {0}, one))))
                + SDD(3, {2}, SDD(2, {2}, SDD(1, {2}, SDD(0, {2}, one))))
                + SDD(3, {2}, SDD(2, {3}, SDD(1, {3}, SDD(0, {2}, one))));
    ASSERT_EQ(std::make_pair(9u, 0u), sdd::tools::nodes(x));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(nodes_test, hierarchical_sdd)
{
  {
    const SDD x(0, one, one);
    ASSERT_EQ(std::make_pair(0u, 1u), sdd::tools::nodes(x));
  }
  {
    const SDD nested(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))));
    const SDD x(1, nested, SDD(0, nested, one));
    ASSERT_EQ(std::make_pair(4u, 2u), sdd::tools::nodes(x));
  }
  {
    const SDD nested0(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))));
    const SDD nested1(3, {1}, SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one))));
    const SDD x = SDD(1, nested0, SDD(0, nested0, one))
                + SDD(1, nested1, SDD(0, nested1, one));
    ASSERT_EQ(std::make_pair(8u, 3u), sdd::tools::nodes(x));
  }
  {
    const SDD nested0(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))));
    const SDD nested1(1, nested0, SDD(0, nested0, one));
    const SDD x(1, nested1, SDD(0, nested1, one));
    ASSERT_EQ(std::make_pair(4u, 4u), sdd::tools::nodes(x));
  }
}

/*------------------------------------------------------------------------------------------------*/

