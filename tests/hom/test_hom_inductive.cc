#include "gtest/gtest.h"

#include "tests/hom/common.hh"

/*-------------------------------------------------------------------------------------------*/

const SDD zero = sdd::zero<conf>();
const SDD one = sdd::one<conf>();
const hom id = sdd::hom::Id<conf>();

struct hom_inductive_test
  : public testing::Test
{
};

/*-------------------------------------------------------------------------------------------*/

struct f0
{
  bool
  skip(unsigned char var)
  const noexcept
  {
    return var != 0;
  }

  bool
  selector()
  const noexcept
  {
    return false;
  }

  hom
  operator()(unsigned char var, const SDD&)
  const
  {
    return sdd::hom::Id<conf>();
  }

  hom
  operator()(unsigned char var, const bitset& val)
  const
  {
    return sdd::hom::Cons<conf>(var, val << 1, id);
  }

  SDD
  operator()()
  const noexcept
  {
    return one;
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

struct f1
{
  bool
  skip(unsigned char var)
  const noexcept
  {
    return var != 1;
  }

  bool
  selector()
  const noexcept
  {
    return false;
  }

  hom
  operator()(unsigned char var, const SDD&)
  const
  {
    return sdd::hom::Id<conf>();
  }

  hom
  operator()(unsigned char var, const bitset& val)
  const
  {
    return sdd::hom::Cons<conf>(var, val << 2, id);
  }

  SDD
  operator()()
  const noexcept
  {
    return one;
  }

  bool
  operator==(const f1&)
  const noexcept
  {
    return true;
  }
};

std::ostream&
operator<<(std::ostream& os, const f1&)
{
  return os << "f1";
}

struct cut
{
  bool
  skip(unsigned char var)
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
  operator()(unsigned char var, const SDD&)
  const
  {
    return sdd::hom::Cons<conf>(var, zero, id);
  }

  hom
  operator()(unsigned char var, const bitset&)
  const
  {
    return sdd::hom::Cons<conf>(var, bitset {}, id);
  }

  SDD
  operator()()
  const noexcept
  {
    return zero;
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
  skip(unsigned char var)
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
  operator()(unsigned char var, const SDD& x)
  const
  {
    return sdd::hom::Cons<conf>(var, x, sdd::hom::Inductive<conf>(*this));
  }

  hom
  operator()(unsigned char var, const bitset& val)
  const
  {
    return sdd::hom::Cons<conf>(var, val, sdd::hom::Inductive<conf>(*this));
  }

  SDD
  operator()()
  const noexcept
  {
    return one;
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
  skip(unsigned char var)
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
  operator()(unsigned char var, const SDD&)
  const
  {
    return sdd::hom::Inductive<conf>(*this);
  }

  hom
  operator()(unsigned char var, const bitset& val)
  const
  {
    return sdd::hom::Inductive<conf>(*this);
  }

  SDD
  operator()()
  const noexcept
  {
    return one;
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

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_inductive_test, construction)
{
  {
    const hom h1 = sdd::hom::Inductive<conf>(f0());
    const hom h2 = sdd::hom::Inductive<conf>(f0());
    ASSERT_EQ(h1, h2);
  }
  {
    const hom h1 = sdd::hom::Inductive<conf>(f0());
    const hom h2 = sdd::hom::Inductive<conf>(f1());
    ASSERT_NE(h1, h2);
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_inductive_test, evaluation_flat)
{
  {
    const hom h1 = sdd::hom::Inductive<conf>(f0());
    ASSERT_EQ(SDD(0, {1,2,3}, one), h1(SDD(0, {0,1,2}, one)));
  }
  {
    const hom h1 = sdd::hom::Inductive<conf>(f0());
    const hom h2 = sdd::hom::Inductive<conf>(f1());
    ASSERT_EQ( SDD(0, {1,2,3}, SDD(1, {2,3,4}, one))
             , h2(h1(SDD(0, {0,1,2}, SDD(1, {0,1,2}, one)))));
    ASSERT_EQ( SDD(0, {1,2,3}, SDD(1, {2,3,4}, one))
             , h1(h2(SDD(0, {0,1,2}, SDD(1, {0,1,2}, one)))));
  }
  {
    const SDD s0 = SDD(0, {0}, SDD(1, {0}, one)) + SDD(0, {1}, SDD(1, {1}, one));
    const SDD s1 = SDD(0, {1}, SDD(1, {0}, one)) + SDD(0, {2}, SDD(1, {1}, one));
    const hom h1 = sdd::hom::Inductive<conf>(f0());
    ASSERT_EQ(s1, h1(s0));
  }
  {
    const hom h1 = sdd::hom::Inductive<conf>(id_prime());
    ASSERT_EQ(SDD(0, {0,1,2}, one), h1(SDD(0, {0,1,2}, one)));
  }
  {
    const hom h1 = sdd::hom::Inductive<conf>(consume());
    ASSERT_EQ(one, h1(SDD(0, {0,1,2}, SDD(1, one, one))));
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_inductive_test, evaluation_hierarchical)
{
  {
    const hom h1 = sdd::hom::Inductive<conf>(f0());
    ASSERT_EQ(SDD(0, {1,2,3}, one), h1(SDD(0, {0,1,2}, one)));
  }
  {
    const hom h1 = sdd::hom::Inductive<conf>(f0());
    const hom h2 = sdd::hom::Inductive<conf>(f1());
    ASSERT_EQ( SDD(0, {1,2,3}, SDD(1, {2,3,4}, one))
              , h2(h1(SDD(0, {0,1,2}, SDD(1, {0,1,2}, one)))));
    ASSERT_EQ( SDD(0, {1,2,3}, SDD(1, {2,3,4}, one))
              , h1(h2(SDD(0, {0,1,2}, SDD(1, {0,1,2}, one)))));
  }
  {
    const SDD s0 = SDD(0, {0}, SDD(1, {0}, one)) + SDD(0, {1}, SDD(1, {1}, one));
    const SDD s1 = SDD(0, {1}, SDD(1, {0}, one)) + SDD(0, {2}, SDD(1, {1}, one));
    const hom h1 = sdd::hom::Inductive<conf>(f0());
    ASSERT_EQ(s1, h1(s0));
  }
}


/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_inductive_test, cut_path)
{
  {
    const hom h0 = sdd::hom::Inductive<conf>(cut());
    ASSERT_EQ(zero, h0(one));
  }
  {
    const hom h0 = sdd::hom::Inductive<conf>(cut());
    ASSERT_EQ(zero, h0(SDD(0, {0}, one)));
  }
  {
    const hom h0 = sdd::hom::Inductive<conf>(cut());
    ASSERT_EQ(zero, h0(SDD(0, one, one)));
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_inductive_test, skip_variable)
{
  {
    const hom h1 = sdd::hom::Inductive<conf>(f0());
    ASSERT_EQ(SDD(1, {0,1,2}, one), h1(SDD(1, {0,1,2}, one)));
  }
  {
    const hom h1 = sdd::hom::Inductive<conf>(f1());
    ASSERT_EQ(SDD(0, {0,1,2}, one), h1(SDD(0, {0,1,2}, one)));
  }
}


/*-------------------------------------------------------------------------------------------*/
