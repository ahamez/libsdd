#include "gtest/gtest.h"

#include "sdd/order/order.hh"
#include "sdd/order/strategies/flatten.hh"

#include "tests/configuration.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct order_strategy_test
  : public testing::Test
{
  using configuration_type = C;
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(order_strategy_test, configurations);
#include "tests/macros.hh"

#define flatten sdd::flatten<conf>
#define variables_per_level sdd::variables_per_level<conf>

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(order_strategy_test, strategy_flatten)
{
  {
    order_builder ob0;
    ASSERT_EQ(order(ob0), order(flatten()(ob0)));
  }
  {
    order_builder ob0({"i", "j", "k"});
    ASSERT_EQ(order(ob0), order(flatten()(ob0)));
  }
  {
    const auto ob0 = order_builder("x", order_builder("i")) << order_builder("j");
    const auto obr = order_builder({"i", "j"});
    ASSERT_EQ(order(obr), order(flatten()(ob0)));
  }
  {
    const auto ob0 = order_builder("i")
                  << order_builder("y", order_builder("j"));
    const auto obr = order_builder({"i", "j"});
    ASSERT_EQ(order(obr), order(flatten()(ob0)));
  }
  {
    const auto ob0 = order_builder("x", order_builder("i"))
                  << order_builder("y", order_builder("j"));
    const auto obr = order_builder({"i", "j"});
    ASSERT_EQ(order(obr), order(flatten()(ob0)));
  }
  {
    const auto ob0 = order_builder("x", order_builder("i"))
                  << order_builder("j")
                  << order_builder("z", order_builder("k"));
    const auto obr = order_builder({"i", "j", "k"});
    ASSERT_EQ(order(obr), order(flatten()(ob0)));
  }
}

/*-------------------------------------------------------------------------------------------*/
