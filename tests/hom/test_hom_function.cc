#include <limits>

#include "gtest/gtest.h"

#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"
#include "sdd/hom/rewrite.hh"
#include "sdd/manager.hh"
#include "sdd/order/order.hh"

#include "tests/configuration.hh"
#include "tests/hom/common.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct hom_function_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_function_test()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::id<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

template <typename C, bool Selector>
struct threshold_fun
{
  using bitset = sdd::values::bitset<64>;
  const bitset mask_;
  const unsigned int max_;

  threshold_fun(unsigned int t)
    : mask_(std::bitset<64>((ULONG_MAX << (std::numeric_limits<unsigned long>::digits - t - 1))
                             >> (std::numeric_limits<unsigned long>::digits - t - 1)))
    , max_(t)
  {}

  bool
  selector()
  const noexcept
  {
    return Selector;
  }

  typename C::Values
  operator()(const bitset& val)
  const noexcept
  {
    return bitset(val.content() & mask_.content());
  }

  template <typename T>
  typename C::Values
  operator()(const T& val)
  const
  {
    T new_val;
    for (const auto& v : val)
    {
      if (v > max_)
      {
        break;
      }
      new_val.insert(v);
    }
    return new_val;
  }

  bool
  operator==(const threshold_fun& other)
  const noexcept
  {
    return mask_ == other.mask_ and max_ == other.max_;
  }
};

template <typename C, bool Selector>
std::ostream&
operator<<(std::ostream& os, const threshold_fun<C, Selector>& f)
{
  return os << "threshold_fun(" << f.mask_ << ")";
}

/*------------------------------------------------------------------------------------------------*/

namespace std {

template <typename C, bool Selector>
struct hash<threshold_fun<C, Selector>>
{
  std::size_t
  operator()(const threshold_fun<C, Selector>& f)
  const noexcept
  {
    return std::hash<unsigned long>()(f.mask_.content().to_ulong());
  }
};

} // namespace std

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_function_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_function_test, construction)
{
  const order o(order_builder {"a", "b"});
  {
    const auto h0 = function<conf>(o, "a", threshold_fun<conf, true>(3));
    const auto h1 = function<conf>(o, "a", threshold_fun<conf, true>(3));
    ASSERT_EQ(h0, h1);
  }
  {
    const auto h0 = function<conf>(o, "a", threshold_fun<conf, true>(3));
    const auto h1 = function<conf>(o, "a", threshold_fun<conf, true>(0));
    ASSERT_NE(h0, h1);
  }
  {
    const auto h0 = function<conf>(o, "a", threshold_fun<conf, true>(3));
    const auto h1 = function<conf>(o, "b", threshold_fun<conf, true>(3));
    ASSERT_NE(h0, h1);
  }
  /// @todo Test that construction with a hierarchical identifier fails.
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_function_test, evaluation_selector)
{
  {
    const order o(order_builder {"a"});
    const SDD s0(0, {1,2,3}, one);
    const auto h0 = function<conf>(o, "a", threshold_fun<conf, true>(2));
    const SDD s1(0, {1,2}, one);
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    const order o(order_builder {"a"});
    const SDD s0(0, {0,1,3}, one);
    const auto h0 = function<conf>(o, "a", threshold_fun<conf, true>(2));
    const SDD s1(0, {0,1}, one);
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    const order o(order_builder {"a", "b"});
    const SDD s0 = SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))
                 + SDD(1, {4,5,6}, SDD(0, {4,5,6}, one));
    const auto h0 = function<conf>(o, "a", threshold_fun<conf, true>(2));
    const SDD s1 = SDD(1, {1,2}, SDD(0, {1,2,3}, one));
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    const order o(order_builder {"a", "b"});
    const SDD s0 = SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))
                 + SDD(1, {4,5,6}, SDD(0, {4,5,6}, one));
    const auto h0 = function<conf>(o, "b", threshold_fun<conf, true>(2));
    const SDD s1 = SDD(1, {1,2,3}, SDD(0, {1,2}, one));
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    const order o(order_builder {"a", "b"});
    const SDD s0 = SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))
                 + SDD(1, {4,5,6}, SDD(0, {4,5,6}, one));
    const auto h0 = function<conf>(o, "a", threshold_fun<conf, true>(0));
    ASSERT_EQ(zero, h0(o, s0));
  }
  {
    const order o(order_builder {"a", "b"});
    const SDD s0 = SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))
                 + SDD(1, {4,5,6}, SDD(0, {4,5,6}, one));
    const auto h0 = function<conf>(o, "b", threshold_fun<conf, true>(0));
    ASSERT_EQ(zero, h0(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_function_test, evaluation_no_selector)
{
  {
    const order o(order_builder {"a"});
    const SDD s0(0, {1,2,3}, one);
    const auto h0 = function<conf>(o, "a", threshold_fun<conf, false>(2));
    const SDD s1(0, {1,2}, one);
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    const order o(order_builder {"a"});
    const SDD s0(0, {0,1,3}, one);
    const auto h0 = function<conf>(o, "a", threshold_fun<conf, false>(2));
    const SDD s1(0, {0,1}, one);
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    const order o(order_builder {"a", "b"});
    const SDD s0 = SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))
                 + SDD(1, {4,5,6}, SDD(0, {4,5,6}, one));
    const auto h0 = function<conf>(o, "a", threshold_fun<conf, false>(2));
    const SDD s1 = SDD(1, {1,2}, SDD(0, {1,2,3}, one));
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    const order o(order_builder {"a", "b"});
    const SDD s0 = SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))
                 + SDD(1, {4,5,6}, SDD(0, {4,5,6}, one));
    const auto h0 = function<conf>(o, "b", threshold_fun<conf, false>(2));
    const SDD s1 = SDD(1, {1,2,3}, SDD(0, {1,2}, one));
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    const order o(order_builder {"a", "b"});
    const SDD s0 = SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))
                 + SDD(1, {4,5,6}, SDD(0, {4,5,6}, one));
    const auto h0 = function<conf>(o, "a", threshold_fun<conf, false>(0));
    ASSERT_EQ(zero, h0(o, s0));
  }
  {
    const order o(order_builder {"a", "b"});
    const SDD s0 = SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))
                 + SDD(1, {4,5,6}, SDD(0, {4,5,6}, one));
    const auto h0 = function<conf>(o, "b", threshold_fun<conf, false>(0));
    ASSERT_EQ(zero, h0(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
