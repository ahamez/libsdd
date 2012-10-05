#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"

/*-------------------------------------------------------------------------------------------*/

typedef sdd::conf::conf0 conf;
typedef sdd::values::bitset<64> bitset;
typedef sdd::SDD<conf> SDD;
typedef sdd::hom::homomorphism<conf> hom;
const SDD zero = sdd::zero<conf>();
const SDD one = sdd::one<conf>();


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
    return sdd::hom::Cons<conf>(var, val << 1);
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

struct f1
{
  bool
  skip(unsigned char var)
  const noexcept
  {
    return var != 1;
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
    return sdd::hom::Cons<conf>(var, val << 2);
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
    return 0;
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

TEST_F(hom_inductive_test, evaluation)
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
