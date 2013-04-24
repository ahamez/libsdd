#include "gtest/gtest.h"

#include "tests/hom/common.hh"

/*------------------------------------------------------------------------------------------------*/

struct hom_inductive_test
  : public testing::Test
{
  sdd::manager<conf> m;

  const SDD zero;
  const SDD one;
  const hom id;

  hom_inductive_test()
    : m(sdd::init<conf>())
    , zero(sdd::zero<conf>())
    , one(sdd::one<conf>())
    , id(sdd::Id<conf>())
  {
  }
};

/*------------------------------------------------------------------------------------------------*/

// Increment by 1 the variable "0"
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

  hom
  operator()(const order&, const SDD&)
  const
  {
    return sdd::Id<conf>();
  }

  hom
  operator()(const order& o, const bitset& val)
  const
  {
    return Cons<conf>(o, val << 1, sdd::Id<conf>());
  }

  SDD
  operator()()
  const noexcept
  {
    return sdd::one<conf>();
  }

  bool
  operator==(const f0&)
  const noexcept
  {
    return true;
  }
};

std::ostream&
operator<<(std::ostream& os, const f0&)
{
  return os << "f0";
}

// Increment by 2 the variable "1"
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

  hom
  operator()(const order&, const SDD&)
  const
  {
    return sdd::Id<conf>();
  }

  hom
  operator()(const order& o, const bitset& val)
  const
  {
    return Cons<conf>(o, val << 2, sdd::Id<conf>());
  }

  SDD
  operator()()
  const noexcept
  {
    return sdd::one<conf>();
  }

  bool
  operator==(const f1&)
  const noexcept
  {
    return true;
  }
};

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

  hom
  operator()(const order& o, const SDD&)
  const
  {
    return Cons<conf>(o, sdd::zero<conf>(), sdd::Id<conf>());
  }

  hom
  operator()(const order& o, const bitset&)
  const
  {
    return Cons<conf>(o, bitset {}, sdd::Id<conf>());
  }

  SDD
  operator()()
  const noexcept
  {
    return sdd::zero<conf>();
  }

  bool
  operator==(const cut&)
  const noexcept
  {
    return true;
  }
};

std::ostream&
operator<<(std::ostream& os, const cut&)
{
  return os << "cut";
}

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

  hom
  operator()(const order& o, const SDD& x)
  const
  {
    return Cons<conf>(o, x, Inductive<conf>(*this));
  }

  hom
  operator()(const order& o, const bitset& val)
  const
  {
    return Cons<conf>(o, val, Inductive<conf>(*this));
  }

  SDD
  operator()()
  const noexcept
  {
    return sdd::one<conf>();
  }

  bool
  operator==(const id_prime&)
  const noexcept
  {
    return true;
  }
};

std::ostream&
operator<<(std::ostream& os, const id_prime&)
{
  return os << "id_prime";
}

struct consume
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

  hom
  operator()(const order&, const SDD&)
  const
  {
    return Inductive<conf>(*this);
  }

  hom
  operator()(const order&, const bitset& val)
  const
  {
    return Inductive<conf>(*this);
  }

  SDD
  operator()()
  const noexcept
  {
    return sdd::one<conf>();
  }

  bool
  operator==(const consume&)
  const noexcept
  {
    return true;
  }
};

std::ostream&
operator<<(std::ostream& os, const consume&)
{
  return os << "consume";
}

namespace std {

template <>
struct hash<f0>
{
  std::size_t
  operator()(const f0&)
  const noexcept
  {
    return 0;
  }
};

template <>
struct hash<f1>
{
  std::size_t
  operator()(const f1&)
  const noexcept
  {
    return 1;
  }
};

template <>
struct hash<cut>
{
  std::size_t
  operator()(const cut&)
  const noexcept
  {
      return 2;
  }
};

template <>
struct hash<id_prime>
{
  std::size_t
  operator()(const id_prime&)
  const noexcept
  {
    return 3;
  }
};

template <>
struct hash<consume>
{
  std::size_t
  operator()(const consume&)
  const noexcept
  {
    return 4;
  }
};

}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_inductive_test, construction)
{
  {
    const hom h1 = Inductive<conf>(f0());
    const hom h2 = Inductive<conf>(f0());
    ASSERT_EQ(h1, h2);
  }
  {
    const hom h1 = Inductive<conf>(f0());
    const hom h2 = Inductive<conf>(f1());
    ASSERT_NE(h1, h2);
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_inductive_test, evaluation_flat)
{
  {
    order o (order_builder {"0"});
    const hom h1 = Inductive<conf>(f0());
    ASSERT_EQ(SDD(0, {1,2,3}, one), h1(o, SDD(0, {0,1,2}, one)));
  }
  {
    order o (order_builder {"0", "1"});
    const hom h1 = Inductive<conf>(f0());
    const hom h2 = Inductive<conf>(f1());
    ASSERT_EQ( SDD(1, {1,2,3}, SDD(0, {2,3,4}, one))
             , h2(o, h1(o, SDD(1, {0,1,2}, SDD(0, {0,1,2}, one)))));
    ASSERT_EQ( SDD(1, {1,2,3}, SDD(0, {2,3,4}, one))
             , h1(o, h2(o, SDD(1, {0,1,2}, SDD(0, {0,1,2}, one)))));
  }
  {
    order o (order_builder {"0", "1"});
    const SDD s0 = SDD(1, {0}, SDD(0, {0}, one)) + SDD(1, {1}, SDD(0, {1}, one));
    const SDD s1 = SDD(1, {1}, SDD(0, {0}, one)) + SDD(1, {2}, SDD(0, {1}, one));
    const hom h1 = Inductive<conf>(f0());
    ASSERT_EQ(s1, h1(o, s0));
  }
  {
    order o (order_builder {"0"});
    const hom h1 = Inductive<conf>(id_prime());
    ASSERT_EQ(SDD(0, {0,1,2}, one), h1(o, SDD(0, {0,1,2}, one)));
  }
  {
    order o (order_builder {"1", "0"});
    const hom h1 = Inductive<conf>(consume());
    ASSERT_EQ(one, h1(o, SDD(1, {0,1,2}, SDD(0, one, one))));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_inductive_test, evaluation_hierarchical)
{
  {
    order o (order_builder {"0"});
    const hom h1 = Inductive<conf>(f0());
    ASSERT_EQ(SDD(0, {1,2,3}, one), h1(o, SDD(0, {0,1,2}, one)));
  }
  {
    order o (order_builder {"1", "0"});
    const hom h0 = Inductive<conf>(f0());
    const hom h1 = Inductive<conf>(f1());
    ASSERT_NE(h0, h1);
    ASSERT_EQ(       SDD(1, {1,2,3}, SDD(0, {2,3,4}, one))
             , h0(o, SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))));
    ASSERT_EQ(       SDD(1, {3,4,5}, SDD(0, {1,2,3}, one))
             , h1(o, SDD(1, {1,2,3}, SDD(0, {1,2,3}, one))));
  }
  {
    order o (order_builder {"1", "0"});
    const hom h1 = Inductive<conf>(f0());
    const hom h2 = Inductive<conf>(f1());
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
    const hom h1 = Inductive<conf>(f0());
    ASSERT_EQ(s1, h1(o, s0));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_inductive_test, cut_path)
{
  {
    const hom h0 = Inductive<conf>(cut());
    ASSERT_EQ(zero, h0(order(order_builder()), one));
  }
  {
    const hom h0 = Inductive<conf>(cut());
    ASSERT_EQ(zero, h0(order(order_builder {"a"}), SDD(0, {0}, one)));
  }
  {
    const hom h0 = Inductive<conf>(cut());
    ASSERT_EQ(zero, h0(order(order_builder {"a"}), SDD(0, one, one)));
  }
}

/*------------------------------------------------------------------------------------------------*/
