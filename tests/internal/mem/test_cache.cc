#include "gtest/gtest.h"

#include "sdd/mem/cache.hh"

using namespace sdd::mem;

/*------------------------------------------------------------------------------------------------*/

struct error
  : public std::exception
{
  const char*
  what()
  const noexcept
  {
    return std::string().c_str();
  }

  template <typename Operation>
  void
  add_step(Operation&&)
  {
  }
};


struct operation
{
  typedef std::size_t result_type;

  const std::size_t i_;

  operation(std::size_t i)
  	: i_(i)
  {
  }

  std::size_t
  operator()()
  const
  {
    if (i_ == 6666)
    {
      throw error();
    }
    return i_ + 1;
  }

  std::string
  print()
  const
  {
    return "";
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
  cache<operation, error> c("c", 100);
  const auto& stats = c.statistics().rounds.front();

  ASSERT_EQ(0, stats.hits);
  ASSERT_EQ(0, stats.misses);
  ASSERT_EQ(0, stats.filtered);
}

/*------------------------------------------------------------------------------------------------*/

TEST(cache, insertion)
{
  cache<operation, error> c("c", 100);
  const auto& stats = c.statistics().rounds.front();

  ASSERT_EQ(2, c(operation(1)));

  ASSERT_EQ(0, stats.hits);
  ASSERT_EQ(stats.misses, 1);
  ASSERT_EQ(stats.filtered, 0);

  ASSERT_EQ(c(operation(1)), 2);
  ASSERT_EQ(stats.hits, 1);
  ASSERT_EQ(stats.misses, 1);
  ASSERT_EQ(stats.filtered, 0);

  ASSERT_EQ(c(operation(2)), 3);
  ASSERT_EQ(stats.hits, 1);
  ASSERT_EQ(stats.misses, 2);
  ASSERT_EQ(stats.filtered, 0);

  ASSERT_EQ(c(operation(2)), 3);
  ASSERT_EQ(stats.hits, 2);
  ASSERT_EQ(stats.misses, 2);
  ASSERT_EQ(stats.filtered, 0);

  ASSERT_EQ(c(operation(3)), 4);
  ASSERT_EQ(stats.hits, 2);
  ASSERT_EQ(stats.misses, 3);
  ASSERT_EQ(stats.filtered, 0);

  ASSERT_EQ(c(operation(3)), 4);
  ASSERT_EQ(stats.hits, 3);
  ASSERT_EQ(stats.misses, 3);
  ASSERT_EQ(stats.filtered, 0);
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

/*------------------------------------------------------------------------------------------------*/

TEST(cache, filters)
{
  {
    cache<operation, error, filter_0> c("c", 100);
    const auto& stats = c.statistics().rounds.front();

    ASSERT_EQ(2, c(operation(1)));
    ASSERT_EQ(0, stats.hits);
    ASSERT_EQ(1, stats.misses);
    ASSERT_EQ(0, stats.filtered);

    ASSERT_EQ(1, c(operation(0)));
    ASSERT_EQ(0, stats.hits);
    ASSERT_EQ(1, stats.misses);
    ASSERT_EQ(1, stats.filtered);

    ASSERT_EQ(1, c(operation(0)));
    ASSERT_EQ(0, stats.hits);
    ASSERT_EQ(1, stats.misses);
    ASSERT_EQ(2, stats.filtered);
  }
  {
    cache<operation, error, filter_0, filter_1> c("c", 100);
    const auto& stats = c.statistics().rounds.front();

    ASSERT_EQ(2, c(operation(1)));
    ASSERT_EQ(0, stats.hits);
    ASSERT_EQ(0, stats.misses);
    ASSERT_EQ(1, stats.filtered);

    ASSERT_EQ(1, c(operation(0)));
    ASSERT_EQ(0, stats.hits);
    ASSERT_EQ(0, stats.misses);
    ASSERT_EQ(2, stats.filtered);

    ASSERT_EQ(1, c(operation(0)));
    ASSERT_EQ(0, stats.hits);
    ASSERT_EQ(0, stats.misses);
    ASSERT_EQ(3, stats.filtered);

    ASSERT_EQ(3, c(operation(2)));
    ASSERT_EQ(0, stats.hits);
    ASSERT_EQ(1, stats.misses);
    ASSERT_EQ(3, stats.filtered);

    ASSERT_EQ(3, c(operation(2)));
    ASSERT_EQ(1, stats.hits);
    ASSERT_EQ(1, stats.misses);
    ASSERT_EQ(3, stats.filtered);
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST(cache, exception)
{
  {
    cache<operation, error> c("c", 100);
    ASSERT_THROW(c(operation(6666)), error);
  }
  {
    cache<operation, error, filter_6666> c("c", 100);
    ASSERT_THROW(c(operation(6666)), error);
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST(cache, cleanup)
{
  cache<operation, error> c("c", 1024);
  for (std::size_t i = 0; i < 2048; ++i)
  {
    ASSERT_EQ(i + 1, c(operation(i)));
  }
  ASSERT_EQ(1, c.statistics().cleanups());
}

/*------------------------------------------------------------------------------------------------*/
