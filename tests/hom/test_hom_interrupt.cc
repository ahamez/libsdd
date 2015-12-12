#include "gtest/gtest.h"

#include <stdexcept>

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
struct interrupt_incr
{
  using values_type = typename C::Values;

  const std::string var_;
  const unsigned int value_;

  interrupt_incr(std::string var, unsigned int val)
    : var_{std::move(var)}
    , value_{val}
  {}

  bool
  skip(const std::string& var)
  const noexcept
  {
    return var != var_;
  }

  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const sdd::SDD<C>& x)
  const
  {
    return sdd::cons(o, x, sdd::inductive<C>(*this));
  }

  template <typename T>
  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const T& val)
  const
  {
    if (val.find(2) != val.end())
    {
      throw std::runtime_error("");
    }
    else
    {
      T new_val;
      for (const auto& v : val)
      {
        new_val.insert(v + value_);
      }
      return cons(o, new_val, sdd::id<C>());
    }
  }

  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const sdd::values::bitset<64>& val)
  const
  {
    if (val.content().test(2))
    {
      throw std::runtime_error("");
    }
    else
    {
      return cons(o, val << value_, sdd::id<C>());
    }
  }

  sdd::SDD<C>
  operator()()
  const noexcept
  {
    return sdd::one<C>();
  }

  bool
  operator==(const interrupt_incr& other)
  const noexcept
  {
    return var_ == other.var_ and value_ == other.value_;
  }
};

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct interrupt_incr_fun
{
  using values_type = typename C::Values;

  const unsigned int value_;

  interrupt_incr_fun(unsigned int val)
    : value_(val)
  {}

  template <typename T>
  values_type
  operator()(const T& val)
  const
  {
    if (val.find(2) != val.end())
    {
      throw std::runtime_error("");
    }
    else
    {
      T new_val;
      for (const auto& v : val)
      {
        new_val.insert(v + value_);
      }
      return new_val;
    }
  }

  sdd::values::bitset<64>
  operator()(const sdd::values::bitset<64>& val)
  const
  {
    if (val.content().test(2))
    {
      throw std::runtime_error("");
    }
    else
    {
      return val << value_;
    }
  }

  bool
  operator==(const interrupt_incr_fun& other)
  const noexcept
  {
    return value_ == other.value_;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const interrupt_incr_fun& i)
  {
    return os << "interrupt_incr_fun(" << i.value_ << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

namespace std {

template <typename C>
struct hash<interrupt_incr<C>>
{
  std::size_t
  operator()(const interrupt_incr<C>& i)
  const noexcept
  {
    return std::hash<std::string>()(i.var_) xor std::hash<unsigned int>()(i.value_);
  }
};

template <typename C>
struct hash<interrupt_incr_fun<C>>
{
  std::size_t
  operator()(const interrupt_incr_fun<C>& i)
  const noexcept
  {
    return std::hash<unsigned int>()(i.value_);
  }
};

} // namespace std

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct hom_interruption_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_interruption_test()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::id<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_interruption_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_interruption_test, function_fixpoint)
{
  {
    order o(order_builder {"2", "1", "0"});
    SDD s0(o, [](const std::string&){return values_type{0};});
    homomorphism h0 = fixpoint(sum(o, {function(o, "0", interrupt_incr_fun<conf>(1)), id}));
    ASSERT_THROW(h0(o, s0), std::runtime_error);
  }
  {
    order o(order_builder {"2", "1", "0"});
    SDD s0(o, [](const std::string&){return values_type{0};});
    homomorphism h0 = fixpoint(sum(o, { function(o, "0", interrupt_incr_fun<conf>(1))
                                      , function(o, "2", interrupt_incr_fun<conf>(1))
                                      , id
                                      }));
    ASSERT_THROW(h0(o, s0), std::runtime_error);
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_interruption_test, inductive_fixpoint)
{
  {
    order o(order_builder {"2", "1", "0"});
    SDD s0(o, [](const std::string&){return values_type{0};});
    homomorphism h0 = fixpoint(sum(o, {inductive<conf>(interrupt_incr<conf>("0", 1)), id}));
    ASSERT_THROW(h0(o, s0), std::runtime_error);
  }
  {
    order o(order_builder {"2", "1", "0"});
    SDD s0(o, [](const std::string&){return values_type{0};});
    homomorphism h0 = fixpoint(sum(o, { inductive<conf>(interrupt_incr<conf>("0", 1))
                                      , inductive<conf>(interrupt_incr<conf>("2", 1))
                                      , id
                                      }));
    ASSERT_THROW(h0(o, s0), std::runtime_error);
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_interruption_test, saturation)
{
  {
    order o(order_builder {"2", "1", "0"});
    SDD s0(o, [](const std::string&){return values_type{0};});
    homomorphism h0
      = sdd::rewrite(o, fixpoint(sum(o, {inductive<conf>(interrupt_incr<conf>("0", 1)), id})));
    ASSERT_THROW(h0(o, s0), std::runtime_error);
  }
  {
    order o(order_builder {"2", "1", "0"});
    SDD s0(o, [](const std::string&){return values_type{0};});
    homomorphism h0 = sdd::rewrite(o, fixpoint(sum( o
                                                  , { inductive<conf>(interrupt_incr<conf>("0", 1))
                                                    , inductive<conf>(interrupt_incr<conf>("2", 1))
                                                    , id
                                                    })));
    ASSERT_THROW(h0(o, s0), std::runtime_error);
  }
}

/*------------------------------------------------------------------------------------------------*/
