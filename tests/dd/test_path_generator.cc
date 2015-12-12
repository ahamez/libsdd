#include <algorithm>
#include <iterator>

#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/path_generator.hh"
#include "sdd/dd/definition.hh"
#include "sdd/manager.hh"

#include "tests/configuration.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct path_generator_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;

  path_generator_test()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(path_generator_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(path_generator_test, terminal_zero)
{
  // an empty path is returned
  auto gen = zero.paths();
  ASSERT_EQ(0u, begin(gen)->size());
  ASSERT_EQ(1u, std::distance(begin(gen), end(gen)));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(path_generator_test, terminal_one)
{
  // an empty path is returned
  auto gen = one.paths();
  ASSERT_EQ(0u, begin(gen)->size());
  ASSERT_EQ(1u, std::distance(begin(gen), end(gen)));
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(path_generator_test, flat)
{
  {
    const auto x = SDD('a', {0,1}, SDD('b', {0,1}, one)) + SDD('a', {2,3}, SDD('b', {2,3}, one));
    auto gen = x.paths();
    std::vector<sdd::path<conf>> v(begin(gen), end(gen));
    std::sort(v.begin(), v.end());
    ASSERT_EQ(2u, v.size());
    std::vector<sdd::path<conf>> r {sdd::path<conf>{{0,1}, {0,1}}, sdd::path<conf>{{2,3}, {2,3}}};
    std::sort(r.begin(), r.end());

    ASSERT_EQ(r, v);
  }
  {
    const auto x = SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one)))
                 + SDD(2, {1}, SDD(1, {1}, SDD(0, {0}, one)))
                 + SDD(2, {2}, SDD(1, {2}, SDD(0, {2}, one)));
    auto gen = x.paths();
    std::vector<sdd::path<conf>> v(begin(gen), end(gen));
    std::sort(v.begin(), v.end());

    ASSERT_EQ(3u, v.size());
    std::vector<sdd::path<conf>> r { sdd::path<conf>{{0}, {0}, {0}}
                                   , sdd::path<conf>{{1}, {1}, {0}}
                                   , sdd::path<conf>{{2}, {2}, {2}}
                                   };
    std::sort(r.begin(), r.end());

    ASSERT_EQ(r, v);
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(path_generator_test, hierarchical)
{
  {
    const auto x = SDD('1', {0}, SDD('0', {0}, one)) + SDD('1', {1}, SDD('0', {1}, one));
    const auto z = SDD(10, x, SDD(11, x, one));
    auto gen = z.paths();
    std::vector<sdd::path<conf>> v(begin(gen), end(gen));
    std::sort(v.begin(), v.end());

    ASSERT_EQ(4u, v.size());
    std::vector<sdd::path<conf>> r { sdd::path<conf>{{0}, {0}, {0}, {0}}
                                   , sdd::path<conf>{{0}, {0}, {1}, {1}}
                                   , sdd::path<conf>{{1}, {1}, {1}, {1}}
                                   , sdd::path<conf>{{1}, {1}, {0}, {0}}
                                   };
    std::sort(r.begin(), r.end());
    ASSERT_EQ(r, v);
  }
  {
    const auto x1 = SDD('1', {0}, SDD('0', {0}, one)) + SDD('1', {1}, SDD('0', {1}, one));
    const auto x2 = SDD('1', {2}, SDD('0', {2}, one)) + SDD('1', {3}, SDD('0', {3}, one));
    const auto z = SDD(10, x1, SDD(11, x1, one)) + SDD(10, x2, SDD(11, x2, one));
    auto gen = z.paths();
    std::vector<sdd::path<conf>> v(begin(gen), end(gen));
    std::sort(v.begin(), v.end());

    ASSERT_EQ(8u, v.size());
    std::vector<sdd::path<conf>> r { sdd::path<conf>{{0}, {0}, {0}, {0}}
                                   , sdd::path<conf>{{0}, {0}, {1}, {1}}
                                   , sdd::path<conf>{{1}, {1}, {1}, {1}}
                                   , sdd::path<conf>{{1}, {1}, {0}, {0}}
                                   , sdd::path<conf>{{2}, {2}, {2}, {2}}
                                   , sdd::path<conf>{{2}, {2}, {3}, {3}}
                                   , sdd::path<conf>{{3}, {3}, {2}, {2}}
                                   , sdd::path<conf>{{3}, {3}, {3}, {3}}
                                   };
    std::sort(r.begin(), r.end());
    ASSERT_EQ(r, v);
  }
  {
    const auto x = SDD('1', {0}, SDD('0', {0}, one)) + SDD('1', {1}, SDD('0', {1}, one));
    const auto z = SDD(100, SDD(10, x, one), SDD(11, x, one));
    auto gen = z.paths();
    std::vector<sdd::path<conf>> v(begin(gen), end(gen));
    std::sort(v.begin(), v.end());

    ASSERT_EQ(4u, v.size());
    std::vector<sdd::path<conf>> r { sdd::path<conf>{{0}, {0}, {0}, {0}}
                                   , sdd::path<conf>{{0}, {0}, {1}, {1}}
                                   , sdd::path<conf>{{1}, {1}, {1}, {1}}
                                   , sdd::path<conf>{{1}, {1}, {0}, {0}}
                                   };
    std::sort(r.begin(), r.end());
    ASSERT_EQ(r, v);
  }
}

/*------------------------------------------------------------------------------------------------*/
