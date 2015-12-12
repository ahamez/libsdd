#include "gtest/gtest.h"

#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"
#include "sdd/hom/rewrite.hh"
#include "sdd/manager.hh"
#include "sdd/order/order.hh"

#include "tests/configuration.hh"
#include "tests/hom/common.hh"
#include "tests/hom/common_inductives.hh"

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct hom_if_then_else
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_if_then_else()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::id<C>())
  {}
};


/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_if_then_else, ::testing::Types<conf1>);
#include "tests/macros.hh"

using sdd::if_then_else;

/*------------------------------------------------------------------------------------------------*/

namespace /* anonymous */{

struct filter
{
  unsigned int value;

  filter(unsigned int v)
    : value(v)
  {}

  bool
  selector()
  const noexcept
  {
    return true;
  }

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>& val)
  const noexcept
  {
    sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;
    for (auto cit = val.lower_bound(value); cit != val.cend(); ++cit)
    {
      builder.insert(*cit);
    }
    return std::move(builder);
  }

  friend
  bool
  operator==(const filter& lhs, const filter& rhs)
  noexcept
  {
    return lhs.value == rhs.value;
  }
};

struct no_selector
{
  bool
  selector()
  const noexcept
  {
    return false;
  }

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>&)
  const
  {
    throw std::runtime_error("Should't happen");
  }

  friend
  bool
  operator==(const no_selector&, const no_selector&)
  noexcept
  {
    return true;
  }
};

struct incr_fun
{
  unsigned int value;

  incr_fun(unsigned int v)
    : value(v)
  {}

  bool
  selector()
  const noexcept
  {
    return true;
  }

  sdd::values::flat_set<unsigned int>
  operator()(const sdd::values::flat_set<unsigned int>& val)
  const noexcept
  {
    sdd::values::values_traits<sdd::values::flat_set<unsigned int>>::builder builder;
    for (auto cit = val.cbegin(); cit != val.cend(); ++cit)
    {
      builder.insert(*cit + value);
    }
    return std::move(builder);
  }

  friend
  bool
  operator==(const incr_fun& lhs, const incr_fun& rhs)
  noexcept
  {
    return lhs.value == rhs.value;
  }
};

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

namespace std {

template <>
struct hash<filter>
{
  std::size_t
  operator()(const filter& f)
  const noexcept
  {
    return std::hash<decltype(f.value)>()(f.value);
  }
};

template <>
struct hash<no_selector>
{
  std::size_t
  operator()(const no_selector&)
  const noexcept
  {
    return 0;
  }
};

template <>
struct hash<incr_fun>
{
  std::size_t
  operator()(const incr_fun& f)
  const noexcept
  {
    return std::hash<decltype(f.value)>()(f.value);
  }
};

} // namespace std

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_if_then_else, construction)
{
  {
    const auto i0 = inductive<conf>(targeted_incr<conf>("a", 1));
    const auto i1 = inductive<conf>(targeted_incr<conf>("b", 1));
    ASSERT_EQ(i0, sdd::if_then_else(id, i0, i1));
  }
  {
    const auto i0 = inductive<conf>(targeted_incr<conf>("a", 1));
    const auto i1 = inductive<conf>(targeted_incr<conf>("b", 1));
    const auto ite0 = sdd::if_then_else(id, i0, i1);
    const auto ite1 = sdd::if_then_else(id, i0, i1);
    ASSERT_EQ(ite0, ite1);
  }
  {
    const auto i0 = inductive<conf>(targeted_incr<conf>("a", 1));
    const auto i1 = inductive<conf>(targeted_incr<conf>("b", 1));
    const auto ite0 = sdd::if_then_else(id, i1, i0);
    const auto ite1 = sdd::if_then_else(id, i0, i1);
    ASSERT_NE(ite0, ite1);
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_if_then_else, construction_error)
{
  const order o(order_builder{"a"});
  ASSERT_THROW(if_then_else(function(o, "a", no_selector{}), id, id), std::invalid_argument);
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_if_then_else, evaluation)
{
  const order o(order_builder {"a", "b", "c"});

  const auto s0 = SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one)))
                + SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one)))
                + SDD(2, {2}, SDD(1, {2}, SDD(0, {2}, one)))
                ;
  {
    const auto pred = sdd::function(o, "a", filter(1));
    const auto ite0 = sdd::if_then_else(pred, id, id);
    ASSERT_EQ(s0, ite0(o, s0));
  }
  {
    const auto pred = function(o, "b", filter(1));
    const auto incr0 = function(o, "a", incr_fun(1));
    const auto ite0 = sdd::if_then_else(pred, incr0, id);
    const auto r0 = SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, one)))
                  + SDD(2, {2}, SDD(1, {1}, SDD(0, {1}, one)))
                  + SDD(2, {3}, SDD(1, {2}, SDD(0, {2}, one)))
                  ;
    ASSERT_EQ(r0, ite0(o, s0));
  }
  {
    const auto pred = function(o, "b", filter(1));
    const auto incr0 = function(o, "a", incr_fun(1));
    const auto ite0 = sdd::if_then_else(pred, id, incr0);
    const auto r0 = SDD(2, {1}, SDD(1, {0}, SDD(0, {0}, one)))
                  + SDD(2, {1}, SDD(1, {1}, SDD(0, {1}, one)))
                  + SDD(2, {2}, SDD(1, {2}, SDD(0, {2}, one)))
                  ;
    ASSERT_EQ(r0, ite0(o, s0));
  }
  {
    const auto pred = function(o, "b", filter(1));
    const auto incr0 = function(o, "a", incr_fun(1));
    const auto incr1 = function(o, "c", incr_fun(2));
    const auto ite0 = sdd::if_then_else(pred, incr1, incr0);
    const auto r0 = SDD(2, {1}, SDD(1, {0}, SDD(0, {0}, one)))
                  + SDD(2, {1}, SDD(1, {1}, SDD(0, {3}, one)))
                  + SDD(2, {2}, SDD(1, {2}, SDD(0, {4}, one)))
                  ;
    ASSERT_EQ(r0, ite0(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/
