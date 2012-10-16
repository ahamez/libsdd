#include <vector>

#include "gtest/gtest.h"

#include "tests/hom/common.hh"

/*-------------------------------------------------------------------------------------------*/

const SDD zero = sdd::zero<conf>();
const SDD one = sdd::one<conf>();
const hom id = sdd::hom::Id<conf>();

struct hom_sum_test
  : public testing::Test
{
};

/*-------------------------------------------------------------------------------------------*/

struct ind
{
  const std::string var_;

  ind(const std::string& v)
    : var_(v)
  {
  }

  bool
  skip(const std::string& var)
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
  operator()(const order<conf>&, const SDD&)
  const
  {
    return sdd::hom::Id<conf>();
  }

  hom
  operator()(const order<conf>&, const bitset& val)
  const
  {
    return sdd::hom::Id<conf>();
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
  return os << "ind(" << i.var_ << ")";
}

namespace std {

template <>
struct hash<ind>
{
  std::size_t
  operator()(const ind& i)
  const noexcept
  {
    return std::hash<std::string>()(i.var_);
  }
};

}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_sum_test, construction)
{
  {
    order<conf> o;
    const hom h1 = sdd::hom::Sum(o, {id});
    const hom h2 = sdd::hom::Sum(o, {id});
    ASSERT_EQ(h1, h2);
  }
  {
    order<conf> o;
    const hom h1 = sdd::hom::Sum(o, {id});
    ASSERT_EQ(id, h1);
  }
  {
    order<conf> o;
    std::vector<hom> empty;
    ASSERT_THROW(sdd::hom::Sum<conf>(o, empty.begin(), empty.end()), std::invalid_argument);
  }
  {
    order<conf> o {"0"};
    const hom h1 = sdd::hom::Sum(o, {sdd::hom::Cons("0", o, bitset {0,1}, id)});
    ASSERT_EQ(sdd::hom::Cons("0", o, bitset {0,1}, id), h1);
  }
  {
    order<conf> o;
    const hom h1 = sdd::hom::Sum(o, {id});
    const hom h2 = sdd::hom::Sum(o, {id, id});
    ASSERT_EQ(h1, h2);
  }
  {
    order<conf> o {"0"};
    const hom h1 = sdd::hom::Sum(o, {id, sdd::hom::Cons("0", o, bitset {0,1}, id)});
    const hom h2 = sdd::hom::Sum(o, {id, sdd::hom::Cons("0", o, bitset {0,2}, id)});
    ASSERT_NE(h1, h2);
  }
  {
    order<conf> o {"0", "1", "2"};
    const hom h1 = sdd::hom::Sum(o, {id, Inductive<conf>(ind("0")), id});
    const hom h2 = sdd::hom::Sum(o, {Inductive<conf>(ind("1")), Inductive<conf>(ind("2"))});
    const hom h3 = sdd::hom::Sum(o, {h1, h2, id});
    const hom h4 = sdd::hom::Sum(o, { id, Inductive<conf>(ind("0")), Inductive<conf>(ind("1"))
                                    , Inductive<conf>(ind("2"))});
    ASSERT_EQ(h4, h3);
  }
  {
    order<conf> o {"0", "1", "2"};
    const hom l1 = sdd::hom::Local("0", o, Inductive<conf>(ind("0")));
    const hom l2 = sdd::hom::Local("0", o, Inductive<conf>(ind("1")));
    const hom s1 = sdd::hom::Sum(o, {l1, l2});
    const hom l3 = sdd::hom::Local("0", o, sdd::hom::Sum(o, { Inductive<conf>(ind("0"))
                                                            , Inductive<conf>(ind("1"))}));
    ASSERT_EQ(s1, l3);
  }
  {
    order<conf> o {"0", "1", "2"};
    const hom l1 = sdd::hom::Local("0", o, Inductive<conf>(ind("0")));
    const hom l2 = sdd::hom::Local("0", o, Inductive<conf>(ind("1")));
    const hom l3 = sdd::hom::Local("1", o, Inductive<conf>(ind("2")));
    const hom s1 = sdd::hom::Sum(o, {l1, l2, l3});
    const hom s2 = sdd::hom::Sum(o, { sdd::hom::Local( "0", o
                                                     , sdd::hom::Sum(o, { Inductive<conf>(ind("0"))
                                                                        , Inductive<conf>(ind("1"))
                                                                        })
                                                     )
                                 , sdd::hom::Local("1", o, Inductive<conf>(ind("2")))
                                 });
    ASSERT_EQ(s1, s2);
  }

}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_sum_test, evaluation)
{
  {
    const hom h = Sum(order<conf>(), {id});
    ASSERT_EQ(one, h(order<conf>(), one));
  }
  {
    const hom h = Sum(order<conf>(), {id});
    ASSERT_EQ(zero, h(order<conf>(), zero));
  }
  {
    order<conf> o {"a", "b"};
    hom h = Sum(o, {Cons("a", o, bitset {0}, id), Cons("a", o, bitset {1}, id)});
    ASSERT_EQ(SDD(1, {0,1}, one), h(o, one));
    ASSERT_EQ(SDD(1, {0,1}, SDD(0, {0}, one)), h(o, SDD(0, {0}, one)));
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_sum_test, error)
{
  order<conf> o {"a", "b", "c"};
  hom h = Sum(o, {Cons("a", o, bitset {0}, id), Cons("b", o, bitset {0}, id)});

  ASSERT_THROW(h(o, one), sdd::hom::evaluation_error<conf>);
  try
  {
    h(o, one);
  }
  catch(sdd::hom::evaluation_error<conf>& e)
  {
    ASSERT_EQ(one, e.operand());
    ASSERT_NE(nullptr, e.what());
  }

  ASSERT_THROW(h(o, SDD(0, {0}, one)), sdd::hom::evaluation_error<conf>);
  try
  {
    h(o, SDD(0, {0}, one));
  }
  catch(sdd::hom::evaluation_error<conf>& e)
  {
    ASSERT_EQ(SDD(0, {0}, one), e.operand());
    ASSERT_NE(nullptr, e.what());
  }
}

/*-------------------------------------------------------------------------------------------*/
