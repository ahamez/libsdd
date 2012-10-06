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

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_sum_test, construction)
{
  {
    const hom h1 = sdd::hom::Sum({id});
    const hom h2 = sdd::hom::Sum({id});
    ASSERT_EQ(h1, h2);
  }
  {
    const hom h1 = sdd::hom::Sum({id});
    ASSERT_EQ(id, h1);
  }
  {
    std::vector<hom> empty;
    const hom h1 = sdd::hom::Sum<conf>(empty.begin(), empty.end());
    ASSERT_EQ(id, h1);
  }
  {
    const hom h1 = sdd::hom::Sum({sdd::hom::Cons(0, bitset {0,1}, id)});
    ASSERT_EQ(sdd::hom::Cons(0, bitset {0,1}, id), h1);
  }
  {
    const hom h1 = sdd::hom::Sum({id});
    const hom h2 = sdd::hom::Sum({id, id});
    ASSERT_EQ(h1, h2);
  }
  {
    const hom h1 = sdd::hom::Sum({id, sdd::hom::Cons(0, bitset {0,1}, id)});
    const hom h2 = sdd::hom::Sum({id, sdd::hom::Cons(0, bitset {0,2}, id)});
    ASSERT_NE(h1, h2);
  }
  {
    const hom h1 = sdd::hom::Sum({id, Inductive<conf>(ind(0)), id});
    const hom h2 = sdd::hom::Sum({Inductive<conf>(ind(1)), Inductive<conf>(ind(2))});
    const hom h3 = sdd::hom::Sum({h1, h2, id});
    const hom h4 = sdd::hom::Sum({ id, Inductive<conf>(ind(0)), Inductive<conf>(ind(1))
                                 , Inductive<conf>(ind(2))});
    ASSERT_EQ(h4, h3);
  }
  {
    const hom l1 = sdd::hom::Local(0, Inductive<conf>(ind(0)));
    const hom l2 = sdd::hom::Local(0, Inductive<conf>(ind(1)));
    const hom s1 = sdd::hom::Sum({l1, l2});
    const hom l3 = sdd::hom::Local(0, sdd::hom::Sum({ Inductive<conf>(ind(0))
                                                    , Inductive<conf>(ind(1))}));
    ASSERT_EQ(s1, l3);
  }
  {
    const hom l1 = sdd::hom::Local(0, Inductive<conf>(ind(0)));
    const hom l2 = sdd::hom::Local(0, Inductive<conf>(ind(1)));
    const hom l3 = sdd::hom::Local(1, Inductive<conf>(ind(2)));
    const hom s1 = sdd::hom::Sum({l1, l2, l3});
    const hom s2 = sdd::hom::Sum({ sdd::hom::Local( 0
                                                  , sdd::hom::Sum({ Inductive<conf>(ind(0))
                                                                  , Inductive<conf>(ind(1))
                                                                  })
                                                  )
                                 , sdd::hom::Local(1, Inductive<conf>(ind(2)))
                                 });
    ASSERT_EQ(s1, s2);
  }

}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_sum_test, evaluation)
{
  {
    const hom h = sdd::hom::Sum({sdd::hom::Id<conf>()});
    ASSERT_EQ(one, h(one));
  }
  {
    const hom h = sdd::hom::Sum({sdd::hom::Id<conf>()});
    ASSERT_EQ(zero, h(zero));
  }
}

/*-------------------------------------------------------------------------------------------*/
