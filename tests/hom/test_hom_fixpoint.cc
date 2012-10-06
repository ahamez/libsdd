#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"

/*-------------------------------------------------------------------------------------------*/

typedef sdd::conf::conf0 conf;
typedef sdd::SDD<conf> SDD;
typedef sdd::hom::homomorphism<conf> hom;
typedef sdd::values::bitset<64> bitset;
const SDD zero = sdd::zero<conf>();
const SDD one = sdd::one<conf>();
const hom id = sdd::hom::Id<conf>();

struct hom_fixpoint_test
  : public testing::Test
{
};

using sdd::hom::Fixpoint;
using sdd::hom::Cons;
using sdd::hom::Inductive;;
using sdd::hom::Sum;

/*-------------------------------------------------------------------------------------------*/

struct inc
{
  const unsigned char var_;
  const unsigned int value_;

  inc(unsigned char var, unsigned int val)
    : var_(var)
    , value_(val)
  {
  }

  bool
  skip(unsigned char var)
  const noexcept
  {
    return var != var_;
  }

  hom
  operator()(unsigned char var, const SDD& x)
  const
  {
    return Cons(var, x, Inductive<conf>(*this));
  }

  hom
  operator()(unsigned char var, const bitset& val)
  const
  {
    if (val.content().test(2))
    {
      return Cons(var, val, id);
    }
    else
    {
      return Cons(var, val << value_, id);
    }
  }

  SDD
  operator()()
  const noexcept
  {
    return one;
  }

  bool
  operator==(const inc& other)
  const noexcept
  {
    return var_ == other.var_ and value_ == other.value_;
  }
};

std::ostream&
operator<<(std::ostream& os, const inc& i)
{
  return os << "inc(" << static_cast<unsigned int>(i.var_) << ")";
}

namespace std {

  template <>
  struct hash<inc>
  {
    std::size_t
    operator()(const inc& i)
    const noexcept
    {
      return std::hash<unsigned char>()(i.var_) xor std::hash<unsigned char>()(i.value_);
    }
  };
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_fixpoint_test, construction)
{
  {
    ASSERT_EQ(id, Fixpoint(id));
  }
  {
    ASSERT_EQ( Fixpoint(Inductive<conf>(inc(0,1)))
             , Fixpoint(Inductive<conf>(inc(0,1)))
             );
  }
  {
    ASSERT_EQ( Fixpoint(Inductive<conf>(inc(0,1)))
             , Fixpoint(Fixpoint(Inductive<conf>(inc(0,1))))
             );
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_fixpoint_test, evaluation)
{
  SDD s0(0, {0}, one);
  hom h0 = Fixpoint(Sum({Inductive<conf>(inc(0, 1)), id}));
  ASSERT_EQ(SDD(0, {0,1,2}, one), h0(s0));
}

/*-------------------------------------------------------------------------------------------*/
