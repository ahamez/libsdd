#include "gtest/gtest.h"

#include <stdexcept>

#include "sdd/mem/cache.hh"

using namespace sdd::mem;

/*------------------------------------------------------------------------------------------------*/

struct context {};
context cxt;

struct operation
{
  const std::size_t i_;

  operation(std::size_t i)
  	: i_(i)
  {}

  std::size_t
  operator()(context&)
  const
  {
    if (i_ == 6666)
    {
      throw std::runtime_error("");
    }
    return i_ + 1;
  }

  std::string
  print()
  const
  {
    return {};
  }

  bool
  operator==(const operation& op)
  const
  {
    return i_ == op.i_;
  }
};

namespace std {

/*------------------------------------------------------------------------------------------------*/

template <>
struct hash<operation>
{
  std::size_t
  operator()(const operation& op)
  const noexcept
  {
    return std::hash<std::size_t>()(op.i_);
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

/*------------------------------------------------------------------------------------------------*/

TEST(cache, creation)
{
  cache<context, operation> c(cxt, 100);
  const auto& stats = c.statistics();

  ASSERT_EQ(0u, stats.hits);
  ASSERT_EQ(0u, stats.misses);
  ASSERT_EQ(0u, stats.filtered);
  ASSERT_EQ(0u, stats.discarded);
}

/*------------------------------------------------------------------------------------------------*/

TEST(cache, insertion)
{
  cache<context, operation> c(cxt, 100);
  const auto& stats = c.statistics();

  ASSERT_EQ(2u, c(operation(1)));

  ASSERT_EQ(0u, stats.hits);
  ASSERT_EQ(1u, stats.misses);
  ASSERT_EQ(0u, stats.filtered);

  ASSERT_EQ(2u, c(operation(1)));
  ASSERT_EQ(1u, stats.hits);
  ASSERT_EQ(1u, stats.misses);
  ASSERT_EQ(0u, stats.filtered);

  ASSERT_EQ(3u, c(operation(2)));
  ASSERT_EQ(1u, stats.hits);
  ASSERT_EQ(2u, stats.misses);
  ASSERT_EQ(0u, stats.filtered);

  ASSERT_EQ(3u, c(operation(2)));
  ASSERT_EQ(2u, stats.hits);
  ASSERT_EQ(2u, stats.misses);
  ASSERT_EQ(0u, stats.filtered);

  ASSERT_EQ(4u, c(operation(3)));
  ASSERT_EQ(2u, stats.hits);
  ASSERT_EQ(3u, stats.misses);
  ASSERT_EQ(0u, stats.filtered);

  ASSERT_EQ(4u, c(operation(3)));
  ASSERT_EQ(3u, stats.hits);
  ASSERT_EQ(3u, stats.misses);
  ASSERT_EQ(0u, stats.filtered);
}

/*------------------------------------------------------------------------------------------------*/

struct filter_0
{
  bool
  operator()(const operation& op)
  const noexcept
  {
    return op.i_ != 0;
  }
};

struct filter_1
{
  bool
  operator()(const operation& op)
  const noexcept
  {
    return op.i_ != 1;
  }
};

struct filter_6666
{
  bool
  operator()(const operation& op)
  const noexcept
  {
    return op.i_ != 6666;
  }
};

struct dummy {};

struct true_filter_1
{
  static bool used;

  bool
  operator()(const dummy&)
  {
    used = true;
    return true;
  }
};

struct true_filter_2
{
  static bool used;

  bool
  operator()(const dummy&)
  {
    used = true;
    return true;
  }
};


struct false_filter_1
{
  static bool used;

  bool
  operator()(const dummy&)
  {
    used = true;
    return false;
  }
};

struct false_filter_2
{
  static bool used;

  bool
  operator()(const dummy&)
  {
    used = true;
    return false;
  }
};


bool true_filter_1::used = false;
bool true_filter_2::used = false;
bool false_filter_1::used = false;
bool false_filter_2::used = false;

void reset_used()
{
  true_filter_1::used = false;
  true_filter_2::used = false;
  false_filter_1::used = false;
  false_filter_2::used = false;
}

/*------------------------------------------------------------------------------------------------*/

TEST(cache, filters)
{
  {
    dummy d;
    reset_used();
    ASSERT_TRUE(apply_filters<dummy>()(d));

    reset_used();
    ASSERT_TRUE((apply_filters<dummy, true_filter_1>()(d)));
    ASSERT_TRUE((true_filter_1::used));

    reset_used();
    ASSERT_TRUE((apply_filters<dummy, true_filter_1, true_filter_2>()(d)));
    ASSERT_TRUE((true_filter_1::used));
    ASSERT_TRUE((true_filter_2::used));

    reset_used();
    ASSERT_FALSE((apply_filters<dummy, false_filter_1>()(d)));
    ASSERT_TRUE((false_filter_1::used));

    reset_used();
    ASSERT_FALSE((apply_filters<dummy, false_filter_1, false_filter_2>()(d)));
    ASSERT_TRUE((false_filter_1::used));
    ASSERT_FALSE((false_filter_2::used));

    reset_used();
    ASSERT_FALSE((apply_filters<dummy, false_filter_1, true_filter_1>()(d)));
    ASSERT_TRUE((false_filter_1::used));
    ASSERT_FALSE((true_filter_1::used));

    reset_used();
    ASSERT_FALSE((apply_filters<dummy, true_filter_1, false_filter_1, true_filter_2>()(d)));
    ASSERT_TRUE((true_filter_1::used));
    ASSERT_TRUE((false_filter_1::used));
    ASSERT_FALSE((true_filter_2::used));
  }
  {
    cache<context, operation, filter_0> c(cxt, 100);
    const auto& stats = c.statistics();

    ASSERT_EQ(2u, c(operation(1)));
    ASSERT_EQ(0u, stats.hits);
    ASSERT_EQ(1u, stats.misses);
    ASSERT_EQ(0u, stats.filtered);

    ASSERT_EQ(1u, c(operation(0)));
    ASSERT_EQ(0u, stats.hits);
    ASSERT_EQ(1u, stats.misses);
    ASSERT_EQ(1u, stats.filtered);

    ASSERT_EQ(1u, c(operation(0)));
    ASSERT_EQ(0u, stats.hits);
    ASSERT_EQ(1u, stats.misses);
    ASSERT_EQ(2u, stats.filtered);
  }
  {
    cache<context, operation, filter_0, filter_1> c(cxt, 100);
    const auto& stats = c.statistics();

    ASSERT_EQ(2u, c(operation(1)));
    ASSERT_EQ(0u, stats.hits);
    ASSERT_EQ(0u, stats.misses);
    ASSERT_EQ(1u, stats.filtered);

    ASSERT_EQ(1u, c(operation(0)));
    ASSERT_EQ(0u, stats.hits);
    ASSERT_EQ(0u, stats.misses);
    ASSERT_EQ(2u, stats.filtered);

    ASSERT_EQ(1u, c(operation(0)));
    ASSERT_EQ(0u, stats.hits);
    ASSERT_EQ(0u, stats.misses);
    ASSERT_EQ(3u, stats.filtered);

    ASSERT_EQ(3u, c(operation(2)));
    ASSERT_EQ(0u, stats.hits);
    ASSERT_EQ(1u, stats.misses);
    ASSERT_EQ(3u, stats.filtered);

    ASSERT_EQ(3u, c(operation(2)));
    ASSERT_EQ(1u, stats.hits);
    ASSERT_EQ(1u, stats.misses);
    ASSERT_EQ(3u, stats.filtered);
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST(cache, exception)
{
  {
    cache<context, operation> c(cxt, 100);
    ASSERT_THROW(c(operation(6666)), std::runtime_error);
  }
  {
    cache<context, operation, filter_6666> c(cxt, 100);
    ASSERT_THROW(c(operation(6666)), std::runtime_error);
  }
}

/*------------------------------------------------------------------------------------------------*/
