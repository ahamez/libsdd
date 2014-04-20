#include "gtest/gtest.h"

#include "sdd/dd/context.hh"
#include "sdd/tools/arcs.hh"
#include "sdd/manager.hh"

#include "tests/configuration.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct arcs_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;

  arcs_test()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(arcs_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(arcs_test, terminal_zero)
{
  ASSERT_EQ(0u, sdd::tools::arcs(zero).size());
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(arcs_test, terminal_one)
{
  ASSERT_EQ(0u, sdd::tools::arcs(one).size());
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(arcs_test, flat_sdd)
{
  {
    const SDD x(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))));
    const auto freq = sdd::tools::arcs(x);
    ASSERT_EQ(1u, freq.size());
    ASSERT_EQ(freq.end(), freq.find(0));
    ASSERT_EQ(4u, freq.find(1)->second.first);
    ASSERT_EQ(0u, freq.find(1)->second.second);
    ASSERT_EQ(std::make_pair(4u, 0u), sdd::tools::number_of_arcs(freq));
  }
  {
    const SDD x = SDD(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))))
                + SDD(3, {1}, SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one))));
    const auto freq = sdd::tools::arcs(x);
    ASSERT_EQ(2u, freq.size());
    ASSERT_EQ(freq.end(), freq.find(0));
    ASSERT_EQ(6u, freq.find(1)->second.first);
    ASSERT_EQ(0u, freq.find(1)->second.second);
    ASSERT_EQ(1u, freq.find(2)->second.first);
    ASSERT_EQ(0u, freq.find(2)->second.second);
    ASSERT_EQ(std::make_pair(8u, 0u), sdd::tools::number_of_arcs(freq));
  }
  {
    const SDD x = SDD(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))))
                + SDD(3, {1}, SDD(2, {1}, SDD(1, {1}, SDD(0, {0}, one))));
    const auto freq = sdd::tools::arcs(x);
    ASSERT_EQ(2u, freq.size());
    ASSERT_EQ(freq.end(), freq.find(0));
    ASSERT_EQ(5u, freq.find(1)->second.first);
    ASSERT_EQ(0u, freq.find(1)->second.second);
    ASSERT_EQ(1u, freq.find(2)->second.first);
    ASSERT_EQ(0u, freq.find(2)->second.second);
    ASSERT_EQ(std::make_pair(7u, 0u), sdd::tools::number_of_arcs(freq));
  }
  {
    const SDD x = SDD(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))))
                + SDD(3, {0}, SDD(2, {1}, SDD(1, {1}, SDD(0, {0}, one))))
                + SDD(3, {2}, SDD(2, {2}, SDD(1, {2}, SDD(0, {2}, one))))
                + SDD(3, {2}, SDD(2, {3}, SDD(1, {3}, SDD(0, {2}, one))));
    const auto freq = sdd::tools::arcs(x);
    ASSERT_EQ(freq.end(), freq.find(0));
    ASSERT_EQ(2u, freq.size());
    ASSERT_EQ(6u, freq.find(1)->second.first);
    ASSERT_EQ(0u, freq.find(1)->second.second);
    ASSERT_EQ(3u, freq.find(2)->second.first);
    ASSERT_EQ(0u, freq.find(2)->second.second);
    ASSERT_EQ(std::make_pair(12u, 0u), sdd::tools::number_of_arcs(freq));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(arcs_test, hierarchical_sdd)
{
  {
    const SDD x(0, one, one);
    const auto freq = sdd::tools::arcs(x);
    ASSERT_EQ(freq.end(), freq.find(0));
    ASSERT_EQ(1u, freq.size());
    ASSERT_EQ(0u, freq.find(1)->second.first);
    ASSERT_EQ(1u, freq.find(1)->second.second);
    ASSERT_EQ(std::make_pair(0u, 1u), sdd::tools::number_of_arcs(freq));
  }
  {
    const SDD x(1, one, SDD(0, one, one));
    const auto freq = sdd::tools::arcs(x);
    ASSERT_EQ(1u, freq.size());
    ASSERT_EQ(freq.end(), freq.find(0));
    ASSERT_EQ(0u, freq.find(1)->second.first);
    ASSERT_EQ(2u, freq.find(1)->second.second);
    ASSERT_EQ(std::make_pair(0u, 2u), sdd::tools::number_of_arcs(freq));
  }
  {
    const SDD nested(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))));
    const SDD x(1, nested, SDD(0, nested, one));
    const auto freq = sdd::tools::arcs(x);
    ASSERT_EQ(1u, freq.size());
    ASSERT_EQ(freq.end(), freq.find(0));
    ASSERT_EQ(4u, freq.find(1)->second.first);
    ASSERT_EQ(2u, freq.find(1)->second.second);
    ASSERT_EQ(std::make_pair(4u, 2u), sdd::tools::number_of_arcs(freq));
  }
  {
    const SDD nested0(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))));
    const SDD nested1(3, {1}, SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one))));
    const SDD x = SDD(1, nested0, SDD(0, nested0, one))
                + SDD(1, nested1, SDD(0, nested1, one));
    const auto freq = sdd::tools::arcs(x);
    ASSERT_EQ(2u, freq.size());
    ASSERT_EQ(freq.end(), freq.find(0));
    ASSERT_EQ(8u, freq.find(1)->second.first);
    ASSERT_EQ(2u, freq.find(1)->second.second);
    ASSERT_EQ(0u, freq.find(2)->second.first);
    ASSERT_EQ(1u, freq.find(2)->second.second);
    ASSERT_EQ(std::make_pair(8u, 4u), sdd::tools::number_of_arcs(freq));
  }
  {
    const SDD nested0(3, {0}, SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one))));
    const SDD nested1(1, nested0, SDD(0, nested0, one));
    const SDD x(1, nested1, SDD(0, nested1, one));
    const auto freq = sdd::tools::arcs(x);
    ASSERT_EQ(1u, freq.size());
    ASSERT_EQ(freq.end(), freq.find(0));
    ASSERT_EQ(4u, freq.find(1)->second.first);
    ASSERT_EQ(4u, freq.find(1)->second.second);
    ASSERT_EQ(std::make_pair(4u, 4u), sdd::tools::number_of_arcs(freq));
  }
}

/*------------------------------------------------------------------------------------------------*/

