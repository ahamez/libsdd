#include <vector>

#include "gtest/gtest.h"

#include "tests/hom/common.hh"

/*------------------------------------------------------------------------------------------------*/

const SDD zero = sdd::zero<conf>();
const SDD one = sdd::one<conf>();
const hom id = sdd::Id<conf>();

struct hom_sum_test
  : public testing::Test
{
};

/*------------------------------------------------------------------------------------------------*/

struct ind
{
  const unsigned char var_;

  ind(unsigned char v)
    : var_(v)
  {
  }

  bool
  skip(unsigned char var)
  const noexcept
  {
    return var != var_;
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
    return id;
  }

  hom
  operator()(unsigned char var, const bitset& val)
  const
  {
    return id;
  }

  SDD
  operator()()
  const noexcept
  {
    return one;
  }

  bool
  operator==(const ind& other)
  const noexcept
  {
    return var_ == other.var_;
  }
};

std::ostream&
operator<<(std::ostream& os, const ind& i)
{
  return os << "ind(" << static_cast<unsigned int>(i.var_) << ")";
}

namespace std {

template <>
struct hash<ind>
{
  std::size_t
  operator()(const ind& i)
  const noexcept
  {
    return std::hash<unsigned char>()(i.var_);
  }
};

}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_sum_test, construction)
{
  {
    const hom h1 = Sum({id});
    const hom h2 = Sum({id});
    ASSERT_EQ(h1, h2);
  }
  {
    const hom h1 = Sum({id});
    ASSERT_EQ(id, h1);
  }
  {
    std::vector<hom> empty;
    ASSERT_THROW(Sum<conf>(empty.begin(), empty.end()), std::invalid_argument);
  }
  {
    const hom h1 = Sum({Cons(0, bitset {0,1}, id)});
    ASSERT_EQ(Cons(0, bitset {0,1}, id), h1);
  }
  {
    const hom h1 = Sum({id});
    const hom h2 = Sum({id, id});
    ASSERT_EQ(h1, h2);
  }
  {
    const hom h1 = Sum({id, Cons(0, bitset {0,1}, id)});
    const hom h2 = Sum({id, Cons(0, bitset {0,2}, id)});
    ASSERT_NE(h1, h2);
  }
  {
    const hom h1 = Sum({id, Inductive<conf>(ind(0)), id});
    const hom h2 = Sum({Inductive<conf>(ind(1)), Inductive<conf>(ind(2))});
    const hom h3 = Sum({h1, h2, id});
    const hom h4 = Sum({ id, Inductive<conf>(ind(0))
                           , Inductive<conf>(ind(1))
                           , Inductive<conf>(ind(2))});
    ASSERT_EQ(h4, h3);
  }
  {
    const hom l1 = Local(0, Inductive<conf>(ind(0)));
    const hom l2 = Local(0, Inductive<conf>(ind(1)));
    const hom s1 = Sum({l1, l2});
    const hom l3 = Local(0, Sum({ Inductive<conf>(ind(0))
                                , Inductive<conf>(ind(1))}));
    ASSERT_EQ(s1, l3);
  }
  {
    const hom l1 = Local(0, Inductive<conf>(ind(0)));
    const hom l2 = Local(0, Inductive<conf>(ind(1)));
    const hom l3 = Local(1, Inductive<conf>(ind(2)));
    const hom s1 = Sum({l1, l2, l3});
    const hom s2 = Sum({ Local( 0
                              , Sum({ Inductive<conf>(ind(0))
                                    , Inductive<conf>(ind(1))
                                    })
                              )
                        , Local(1, Inductive<conf>(ind(2)))
                        });
    ASSERT_EQ(s1, s2);
  }

}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_sum_test, evaluation)
{
  {
    const hom h = Sum({id});
    ASSERT_EQ(one, h(one));
  }
  {
    const hom h = Sum({id});
    ASSERT_EQ(zero, h(zero));
  }
  {
    hom h = Sum({Cons('a', bitset {0}, id), Cons('a', bitset {1}, id)});
    ASSERT_EQ(SDD('a', {0,1}, one), h(one));
    ASSERT_EQ(SDD('a', {0,1}, SDD('b', {0}, one)), h(SDD('b', {0}, one)));
  }
}

/*------------------------------------------------------------------------------------------------*/

TEST_F(hom_sum_test, error)
{
  hom h = Sum({Cons('a', bitset {0}, id), Cons('b', bitset {0}, id)});

  ASSERT_THROW(h(one), sdd::evaluation_error<conf>);
  try
  {
    h(one);
  }
  catch(sdd::evaluation_error<conf>& e)
  {
    ASSERT_EQ(one, e.operand());
    ASSERT_NE(nullptr, e.what());
  }

  ASSERT_THROW(h(SDD('c', {0}, one)), sdd::evaluation_error<conf>);
  try
  {
    h(SDD('c', {0}, one));
  }
  catch(sdd::evaluation_error<conf>& e)
  {
    ASSERT_EQ(SDD('c', {0}, one), e.operand());
    ASSERT_NE(nullptr, e.what());
  }

}

/*------------------------------------------------------------------------------------------------*/
