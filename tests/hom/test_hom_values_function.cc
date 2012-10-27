#include <limits>

#include "gtest/gtest.h"

#include "tests/hom/common.hh"

/*-------------------------------------------------------------------------------------------*/

const SDD one = sdd::one<conf>();
const SDD zero = sdd::zero<conf>();
const hom id = sdd::hom::Id<conf>();

struct hom_values_function_test
  : public testing::Test
{
};

/*-------------------------------------------------------------------------------------------*/

struct threshold_fun
{

  const bitset mask_;

  threshold_fun(unsigned int t)
    : mask_(std::bitset<64>((ULONG_MAX << (std::numeric_limits<unsigned long>::digits - t - 1))
                             >> (std::numeric_limits<unsigned long>::digits - t - 1)))
  {
  }

  bool
  selector()
  const noexcept
  {
    return true;
  }

  bitset
  operator()(const bitset& val)
  const noexcept
  {
    return bitset(val.content() & mask_.content());
  }

  bool
  operator==(const threshold_fun& other)
  const noexcept
  {
    return mask_ == other.mask_;
  }
};

std::ostream&
operator<<(std::ostream& os, const threshold_fun& f)
{
  return os << "threshold_fun(" << f.mask_ << ")";
}

/*-------------------------------------------------------------------------------------------*/

struct threshold_fun2
{

  const bitset mask_;

  threshold_fun2(unsigned int t)
    : mask_(std::bitset<64>((ULONG_MAX << (std::numeric_limits<unsigned long>::digits - t - 1))
                             >> (std::numeric_limits<unsigned long>::digits - t - 1)))
  {
  }

  bool
  selector()
  const noexcept
  {
    return false; // <= only difference with threshold_fun
  }

  bitset
  operator()(const bitset& val)
  const noexcept
  {
    return bitset(val.content() & mask_.content());
  }

  bool
  operator==(const threshold_fun2& other)
  const noexcept
  {
    return mask_ == other.mask_;
  }
};

std::ostream&
operator<<(std::ostream& os, const threshold_fun2& f)
{
  return os << "threshold_fun2(" << f.mask_ << ")";
}

/*-------------------------------------------------------------------------------------------*/

namespace std {

template <>
struct hash<threshold_fun>
{
  std::size_t
  operator()(const threshold_fun& f)
  const noexcept
  {
    return std::hash<unsigned long>()(f.mask_.content().to_ulong());
  }
};

template <>
struct hash<threshold_fun2>
{
  std::size_t
  operator()(const threshold_fun2& f)
  const noexcept
  {
    return std::hash<unsigned long>()(f.mask_.content().to_ulong());
  }
};
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_values_function_test, construction)
{
<<<<<<< HEAD
  const order o(order_builder {"a", "b"});
  {
    const auto h0 = ValuesFunction<conf>(o, "a", threshold_fun(3));
    const auto h1 = ValuesFunction<conf>(o, "a", threshold_fun(3));
    ASSERT_EQ(h0, h1);
  }
  {
    const auto h0 = ValuesFunction<conf>(o, "a", threshold_fun(3));
    const auto h1 = ValuesFunction<conf>(o, "a", threshold_fun(0));
    ASSERT_NE(h0, h1);
  }
  {
    const auto h0 = ValuesFunction<conf>(o, "a", threshold_fun(3));
    const auto h1 = ValuesFunction<conf>(o, "b", threshold_fun(3));
    ASSERT_NE(h0, h1);
  }
  {
    ASSERT_THROW(ValuesFunction<conf>(o, "c", threshold_fun(2)), std::runtime_error);
  }
=======
  {
    const auto h0 = ValuesFunction<conf>(0, threshold_fun(3));
    const auto h1 = ValuesFunction<conf>(0, threshold_fun(3));
    ASSERT_EQ(h0, h1);
  }
  {
    const auto h0 = ValuesFunction<conf>(0, threshold_fun(3));
    const auto h1 = ValuesFunction<conf>(0, threshold_fun(0));
    ASSERT_NE(h0, h1);
  }
  {
    const auto h0 = ValuesFunction<conf>(0, threshold_fun(3));
    const auto h1 = ValuesFunction<conf>(1, threshold_fun(3));
    ASSERT_NE(h0, h1);
  }
>>>>>>> 18437b132e751aceeddc8777dbadc393980982c9
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_values_function_test, evaluation_selector)
{
  {
<<<<<<< HEAD
    const order o(order_builder {"a"});
    const SDD s0(0, {1,2,3}, one);
    const auto h0 = ValuesFunction<conf>(o, "a", threshold_fun(2));
    const SDD s1(0, {1,2}, one);
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    const order o(order_builder {"a"});
    const SDD s0(0, {0,1,3}, one);
    const auto h0 = ValuesFunction<conf>(o, "a", threshold_fun(2));
    const SDD s1(0, {0,1}, one);
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    const order o(order_builder {"a", "b"});
    const SDD s0 = SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))
                 + SDD(1, {4,5,6}, SDD(0, {4,5,6}, one));
    const auto h0 = ValuesFunction<conf>(o, "a", threshold_fun(2));
    const SDD s1 = SDD(1, {1,2}, SDD(0, {1,2,3}, one));
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    const order o(order_builder {"a", "b"});
    const SDD s0 = SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))
                 + SDD(1, {4,5,6}, SDD(0, {4,5,6}, one));
    const auto h0 = ValuesFunction<conf>(o, "b", threshold_fun(2));
    const SDD s1 = SDD(1, {1,2,3}, SDD(0, {1,2}, one));
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    const order o(order_builder {"a", "b"});
    const SDD s0 = SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))
                 + SDD(1, {4,5,6}, SDD(0, {4,5,6}, one));
    const auto h0 = ValuesFunction<conf>(o, "a", threshold_fun(0));
    ASSERT_EQ(zero, h0(o, s0));
  }
  {
    const order o(order_builder {"a", "b"});
    const SDD s0 = SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))
                 + SDD(1, {4,5,6}, SDD(0, {4,5,6}, one));
    const auto h0 = ValuesFunction<conf>(o, "b", threshold_fun(0));
    ASSERT_EQ(zero, h0(o, s0));
=======
    const SDD s0('a', {1,2,3}, one);
    const auto h0 = ValuesFunction<conf>('a', threshold_fun(2));
    const SDD s1('a', {1,2}, one);
    ASSERT_EQ(s1, h0(s0));
  }
  {
    const SDD s0('a', {0,1,3}, one);
    const auto h0 = ValuesFunction<conf>('a', threshold_fun(2));
    const SDD s1('a', {0,1}, one);
    ASSERT_EQ(s1, h0(s0));
  }
  {
    const SDD s0('a', {1,2,3}, one);
    const auto h0 = ValuesFunction<conf>('b', threshold_fun(2));
    ASSERT_EQ(s0, h0(s0));
  }
  {
    const SDD s0 = SDD('a', {1,2,3}, SDD('b', {1,2,3}, one))
                 + SDD('a', {4,5,6}, SDD('b', {4,5,6}, one));
    const auto h0 = ValuesFunction<conf>('a', threshold_fun(2));
    const SDD s1 = SDD('a', {1,2}, SDD('b', {1,2,3}, one));
    ASSERT_EQ(s1, h0(s0));
  }
  {
    const SDD s0 = SDD('a', {1,2,3}, SDD('b', {1,2,3}, one))
                 + SDD('a', {4,5,6}, SDD('b', {4,5,6}, one));
    const auto h0 = ValuesFunction<conf>('b', threshold_fun(2));
    const SDD s1 = SDD('a', {1,2,3}, SDD('b', {1,2}, one));
    ASSERT_EQ(s1, h0(s0));
  }
  {
    const SDD s0 = SDD('a', {1,2,3}, SDD('b', {1,2,3}, one))
                 + SDD('a', {4,5,6}, SDD('b', {4,5,6}, one));
    const auto h0 = ValuesFunction<conf>('a', threshold_fun(0));
    ASSERT_EQ(zero, h0(s0));
  }
  {
    const SDD s0 = SDD('a', {1,2,3}, SDD('b', {1,2,3}, one))
                 + SDD('a', {4,5,6}, SDD('b', {4,5,6}, one));
    const auto h0 = ValuesFunction<conf>('b', threshold_fun(0));
    ASSERT_EQ(zero, h0(s0));
>>>>>>> 18437b132e751aceeddc8777dbadc393980982c9
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_values_function_test, evaluation_no_selector)
{
  {
<<<<<<< HEAD
    const order o(order_builder {"a"});
    const SDD s0(0, {1,2,3}, one);
    const auto h0 = ValuesFunction<conf>(o, "a", threshold_fun2(2));
    const SDD s1(0, {1,2}, one);
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    const order o(order_builder {"a"});
    const SDD s0(0, {0,1,3}, one);
    const auto h0 = ValuesFunction<conf>(o, "a", threshold_fun2(2));
    const SDD s1(0, {0,1}, one);
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    const order o(order_builder {"a", "b"});
    const SDD s0 = SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))
                 + SDD(1, {4,5,6}, SDD(0, {4,5,6}, one));
    const auto h0 = ValuesFunction<conf>(o, "a", threshold_fun2(2));
    const SDD s1 = SDD(1, {1,2}, SDD(0, {1,2,3}, one));
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    const order o(order_builder {"a", "b"});
    const SDD s0 = SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))
                 + SDD(1, {4,5,6}, SDD(0, {4,5,6}, one));
    const auto h0 = ValuesFunction<conf>(o, "b", threshold_fun2(2));
    const SDD s1 = SDD(1, {1,2,3}, SDD(0, {1,2}, one));
    ASSERT_EQ(s1, h0(o, s0));
  }
  {
    const order o(order_builder {"a", "b"});
    const SDD s0 = SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))
                 + SDD(1, {4,5,6}, SDD(0, {4,5,6}, one));
    const auto h0 = ValuesFunction<conf>(o, "a", threshold_fun2(0));
    ASSERT_EQ(zero, h0(o, s0));
  }
  {
    const order o(order_builder {"a", "b"});
    const SDD s0 = SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))
                 + SDD(1, {4,5,6}, SDD(0, {4,5,6}, one));
    const auto h0 = ValuesFunction<conf>(o, "b", threshold_fun2(0));
    ASSERT_EQ(zero, h0(o, s0));
=======
    const SDD s0('a', {1,2,3}, one);
    const auto h0 = ValuesFunction<conf>('a', threshold_fun2(2));
    const SDD s1('a', {1,2}, one);
    ASSERT_EQ(s1, h0(s0));
  }
  {
    const SDD s0('a', {0,1,3}, one);
    const auto h0 = ValuesFunction<conf>('a', threshold_fun2(2));
    const SDD s1('a', {0,1}, one);
    ASSERT_EQ(s1, h0(s0));
  }
  {
    const SDD s0('a', {1,2,3}, one);
    const auto h0 = ValuesFunction<conf>('b', threshold_fun2(2));
    ASSERT_EQ(s0, h0(s0));
  }
  {
    const SDD s0 = SDD('a', {1,2,3}, SDD('b', {1,2,3}, one))
                 + SDD('a', {4,5,6}, SDD('b', {4,5,6}, one));
    const auto h0 = ValuesFunction<conf>('a', threshold_fun2(2));
    const SDD s1 = SDD('a', {1,2}, SDD('b', {1,2,3}, one));
    ASSERT_EQ(s1, h0(s0));
  }
  {
    const SDD s0 = SDD('a', {1,2,3}, SDD('b', {1,2,3}, one))
                 + SDD('a', {4,5,6}, SDD('b', {4,5,6}, one));
    const auto h0 = ValuesFunction<conf>('b', threshold_fun2(2));
    const SDD s1 = SDD('a', {1,2,3}, SDD('b', {1,2}, one));
    ASSERT_EQ(s1, h0(s0));
  }
  {
    const SDD s0 = SDD('a', {1,2,3}, SDD('b', {1,2,3}, one))
                 + SDD('a', {4,5,6}, SDD('b', {4,5,6}, one));
    const auto h0 = ValuesFunction<conf>('a', threshold_fun2(0));
    ASSERT_EQ(zero, h0(s0));
  }
  {
    const SDD s0 = SDD('a', {1,2,3}, SDD('b', {1,2,3}, one))
                 + SDD('a', {4,5,6}, SDD('b', {4,5,6}, one));
    const auto h0 = ValuesFunction<conf>('b', threshold_fun2(0));
    ASSERT_EQ(zero, h0(s0));
>>>>>>> 18437b132e751aceeddc8777dbadc393980982c9
  }
}

/*-------------------------------------------------------------------------------------------*/
