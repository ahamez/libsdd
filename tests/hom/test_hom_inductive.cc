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
struct hom_inductive_test
  : public testing::Test
{
  using configuration_type = C;

  sdd::manager<C> m;

  const sdd::SDD<C> zero;
  const sdd::SDD<C> one;
  const sdd::homomorphism<C> id;

  hom_inductive_test()
    : m(sdd::init(small_conf<C>()))
    , zero(sdd::zero<C>())
    , one(sdd::one<C>())
    , id(sdd::id<C>())
  {}
};

/*------------------------------------------------------------------------------------------------*/

// Increment by 1 the variable "0"
template <typename C>
struct f0
{
  bool
  skip(const std::string& var)
  const noexcept
  {
    return var != "0";
  }

  bool
  selector()
  const noexcept
  {
    return false;
  }

  sdd::homomorphism<C>
  operator()(const sdd::order<C>&, const sdd::SDD<C>&)
  const
  {
    return sdd::id<C>();
  }

  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const sdd::values::bitset<64>& val)
  const
  {
    return cons<C>(o, val << 1, sdd::id<C>());
  }

  template <typename T>
  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const T& val)
  const
  {
    T new_val;
    for (const auto& v : val)
    {
      new_val.insert(v + 1);
    }
    return cons<C>(o, new_val, sdd::id<C>());
  }

  sdd::SDD<C>
  operator()()
  const noexcept
  {
    return sdd::one<C>();
  }

  bool
  operator==(const f0&)
  const noexcept
  {
    return true;
  }
};

template <typename C>
std::ostream&
operator<<(std::ostream& os, const f0<C>&)
{
  return os << "f0";
}

/*------------------------------------------------------------------------------------------------*/

// Increment by 2 the variable "1"
template <typename C>
struct f1
{
  bool
  skip(const std::string& var)
  const noexcept
  {
    return var != "1";
  }

  bool
  selector()
  const noexcept
  {
    return false;
  }

  sdd::homomorphism<C>
  operator()(const sdd::order<C>&, const sdd::SDD<C>&)
  const
  {
    return sdd::id<C>();
  }

  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const sdd::values::bitset<64>& val)
  const
  {
    return cons<C>(o, val << 2, sdd::id<C>());
  }

  template <typename T>
  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const T& val)
  const
  {
    T new_val;
    for (const auto& v : val)
    {
      new_val.insert(v + 2);
    }
    return cons<C>(o, new_val, sdd::id<C>());
  }

  sdd::SDD<C>
  operator()()
  const noexcept
  {
    return sdd::one<C>();
  }

  bool
  operator==(const f1&)
  const noexcept
  {
    return true;
  }
};

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct cut
{
  bool
  skip(const std::string&)
  const noexcept
  {
    return false;
  }

  bool
  selector()
  const noexcept
  {
    return false;
  }

  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const sdd::SDD<C>&)
  const
  {
    return cons<C>(o, sdd::zero<C>(), sdd::id<C>());
  }

  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const typename C::Values&)
  const
  {
    return cons<C>(o, typename C::Values {}, sdd::id<C>());
  }

  sdd::SDD<C>
  operator()()
  const noexcept
  {
    return sdd::zero<C>();
  }

  bool
  operator==(const cut&)
  const noexcept
  {
    return true;
  }
};

template <typename C>
std::ostream&
operator<<(std::ostream& os, const cut<C>&)
{
  return os << "cut";
}

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct id_prime
{
  bool
  skip(const std::string&)
  const noexcept
  {
    return false;
  }

  bool
  selector()
  const noexcept
  {
    return true;
  }

  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const sdd::SDD<C>& x)
  const
  {
    return cons<C>(o, x, inductive<C>(*this));
  }

  sdd::homomorphism<C>
  operator()(const sdd::order<C>& o, const typename C::Values& val)
  const
  {
    return cons<C>(o, val, inductive<C>(*this));
  }

  sdd::SDD<C>
  operator()()
  const noexcept
  {
    return sdd::one<C>();
  }

  bool
  operator==(const id_prime&)
  const noexcept
  {
    return true;
  }
};

template <typename C>
std::ostream&
operator<<(std::ostream& os, const id_prime<C>&)
{
  return os << "id_prime";
}

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct consume
{
  constexpr bool
  skip(const std::string&)
  const noexcept
  {
    return false;
  }

  constexpr bool
  selector()
  const noexcept
  {
    return false;
  }

  template <typename T>
  sdd::homomorphism<C>
  operator()(const sdd::order<C>&, const T&)
  const
  {
    return inductive<C>(*this);
  }

  sdd::SDD<C>
  operator()()
  const noexcept
  {
    return sdd::one<C>();
  }

  bool
  operator==(const consume&)
  const noexcept
  {
    return true;
  }
};

template <typename C>
std::ostream&
operator<<(std::ostream& os, const consume<C>&)
{
  return os << "consume";
}

/*------------------------------------------------------------------------------------------------*/

namespace std {

template <typename C>
struct hash<f0<C>>
{
  std::size_t
  operator()(const f0<C>&)
  const noexcept
  {
    return 0;
  }
};

template <typename C>
struct hash<f1<C>>
{
  std::size_t
  operator()(const f1<C>&)
  const noexcept
  {
    return 1;
  }
};

template <typename C>
struct hash<cut<C>>
{
  std::size_t
  operator()(const cut<C>&)
  const noexcept
  {
    return 2;
  }
};

template <typename C>
struct hash<id_prime<C>>
{
  std::size_t
  operator()(const id_prime<C>&)
  const noexcept
  {
    return 3;
  }
};

template <typename C>
struct hash<consume<C>>
{
  std::size_t
  operator()(const consume<C>&)
  const noexcept
  {
    return 4;
  }
};

} // namespace std

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST_CASE(hom_inductive_test, configurations);
#include "tests/macros.hh"

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_inductive_test, construction)
{
  {
    const auto h1 = inductive<conf>(f0<conf>());
    const auto h2 = inductive<conf>(f0<conf>());
    ASSERT_EQ(h1, h2);
  }
  {
    const auto h1 = inductive<conf>(f0<conf>());
    const auto h2 = inductive<conf>(f1<conf>());
    ASSERT_NE(h1, h2);
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_inductive_test, evaluation_flat)
{
  {
    order o (order_builder {"0"});
    const auto h1 = inductive<conf>(f0<conf>());
    ASSERT_EQ(SDD(0, {1,2,3}, one), h1(o, SDD(0, {0,1,2}, one)));
  }
  {
    order o (order_builder {"0", "1"});
    const auto h1 = inductive<conf>(f0<conf>());
    const auto h2 = inductive<conf>(f1<conf>());
    ASSERT_EQ( SDD(1, {1,2,3}, SDD(0, {2,3,4}, one))
             , h2(o, h1(o, SDD(1, {0,1,2}, SDD(0, {0,1,2}, one)))));
    ASSERT_EQ( SDD(1, {1,2,3}, SDD(0, {2,3,4}, one))
             , h1(o, h2(o, SDD(1, {0,1,2}, SDD(0, {0,1,2}, one)))));
  }
  {
    order o (order_builder {"0", "1"});
    const SDD s0 = SDD(1, {0}, SDD(0, {0}, one)) + SDD(1, {1}, SDD(0, {1}, one));
    const SDD s1 = SDD(1, {1}, SDD(0, {0}, one)) + SDD(1, {2}, SDD(0, {1}, one));
    const auto h1 = inductive<conf>(f0<conf>());
    ASSERT_EQ(s1, h1(o, s0));
  }
  {
    order o (order_builder {"0"});
    const auto h1 = inductive<conf>(id_prime<conf>());
    ASSERT_EQ(SDD(0, {0,1,2}, one), h1(o, SDD(0, {0,1,2}, one)));
  }
  {
    order o (order_builder {"1", "0"});
    const auto h1 = inductive<conf>(consume<conf>());
    ASSERT_EQ(one, h1(o, SDD(1, {0,1,2}, SDD(0, one, one))));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_inductive_test, evaluation_hierarchical)
{
  {
    order o (order_builder {"0"});
    const auto h1 = inductive<conf>(f0<conf>());
    ASSERT_EQ(SDD(0, {1,2,3}, one), h1(o, SDD(0, {0,1,2}, one)));
  }
  {
    order o (order_builder {"1", "0"});
    const auto h0 = inductive<conf>(f0<conf>());
    const auto h1 = inductive<conf>(f1<conf>());
    ASSERT_NE(h0, h1);
    ASSERT_EQ(       SDD(1, {1,2,3}, SDD(0, {2,3,4}, one))
             , h0(o, SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))));
    ASSERT_EQ(       SDD(1, {3,4,5}, SDD(0, {1,2,3}, one))
             , h1(o, SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))));
  }
  {
    order o (order_builder {"1", "0"});
    const auto h1 = inductive<conf>(f0<conf>());
    const auto h2 = inductive<conf>(f1<conf>());
    ASSERT_NE(h1, h2);
    ASSERT_EQ(             SDD(1, {2,3,4}, SDD(0, {1,2,3}, one))
             , h2(o, h1(o, SDD(1, {0,1,2}, SDD(0, {0,1,2}, one)))));
    ASSERT_EQ(             SDD(1, {2,3,4}, SDD(0, {1,2,3}, one))
             , h1(o, h2(o, SDD(1, {0,1,2}, SDD(0, {0,1,2}, one)))));
  }
  {
    order o (order_builder {"0", "1"});
    const SDD s0 = SDD(1, {0}, SDD(0, {0}, one)) + SDD(1, {1}, SDD(0, {1}, one));
    const SDD s1 = SDD(1, {1}, SDD(0, {0}, one)) + SDD(1, {2}, SDD(0, {1}, one));
    const auto h1 = inductive<conf>(f0<conf>());
    ASSERT_EQ(s1, h1(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(hom_inductive_test, cut_path)
{
  {
    const auto h0 = inductive<conf>(cut<conf>());
    ASSERT_EQ(zero, h0(order(order_builder()), one));
  }
  {
    const auto h0 = inductive<conf>(cut<conf>());
    ASSERT_EQ(zero, h0(order(order_builder {"a"}), SDD(0, {0}, one)));
  }
  {
    const auto h0 = inductive<conf>(cut<conf>());
    ASSERT_EQ(zero, h0(order(order_builder {"a"}), SDD(0, one, one)));
  }
}

/*------------------------------------------------------------------------------------------------*/
