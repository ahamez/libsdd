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
    : m(sdd::manager<C>::init(small_conf<C>()))
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
  ASSERT_EQ(std::make_pair(0ul, 0ul), sdd::tools::nodes(zero));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(nodes_test, terminal_one)
{
  ASSERT_EQ(std::make_pair(0ul, 0ul), sdd::tools::nodes(one));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(nodes_test, flat_sdd)
{
  {
    const SDD x(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))));
    ASSERT_EQ(std::make_pair(4ul, 0ul), sdd::tools::nodes(x));
  }
  {
    const SDD x = SDD(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))))
                + SDD(3, {1}, SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one))));
    ASSERT_EQ(std::make_pair(7ul, 0ul), sdd::tools::nodes(x));
  }
  {
    const SDD x = SDD(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))))
                + SDD(3, {1}, SDD(2, {1}, SDD(1, {1}, SDD(0, {0}, one))));
    ASSERT_EQ(std::make_pair(6ul, 0ul), sdd::tools::nodes(x));
  }
  {
    const SDD x = SDD(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))))
                + SDD(3, {0}, SDD(2, {1}, SDD(1, {1}, SDD(0, {0}, one))))
                + SDD(3, {2}, SDD(2, {2}, SDD(1, {2}, SDD(0, {2}, one))))
                + SDD(3, {2}, SDD(2, {3}, SDD(1, {3}, SDD(0, {2}, one))));
    ASSERT_EQ(std::make_pair(9ul, 0ul), sdd::tools::nodes(x));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(nodes_test, hierarchical_sdd)
{
  {
    const SDD x(1, one, one);
    ASSERT_EQ(std::make_pair(0ul, 1ul), sdd::tools::nodes(x));
  }
  {
    const SDD nested(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))));
    const SDD x(1, nested, SDD(0, nested, one));
    ASSERT_EQ(std::make_pair(4ul, 2ul), sdd::tools::nodes(x));
  }
  {
    const SDD nested0(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))));
    const SDD nested1(3, {1}, SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one))));
    const SDD x = SDD(1, nested0, SDD(0, nested0, one))
                + SDD(1, nested1, SDD(0, nested1, one));
    ASSERT_EQ(std::make_pair(8ul, 3ul), sdd::tools::nodes(x));
  }
  {
    const SDD nested0(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))));
    const SDD nested1(1, nested0, SDD(0, nested0, one));
    const SDD x(1, nested1, SDD(0, nested1, one));
    ASSERT_EQ(std::make_pair(4ul, 4ul), sdd::tools::nodes(x));
  }
}

/*------------------------------------------------------------------------------------------------*/

