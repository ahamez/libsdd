#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"

/*-------------------------------------------------------------------------------------------*/

struct hom_cons_test
  : public testing::Test
{
  typedef sdd::conf::conf0 conf;
  typedef sdd::SDD<conf> SDD;
  typedef sdd::hom::homomorphism<conf> hom;
  const SDD zero = sdd::zero<conf>();
  const SDD one = sdd::one<conf>();

  hom_cons_test()
  {
  }
};

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_cons_test, construction)
{
  {
    const hom h1 = sdd::hom::Cons<conf>(0, conf::Values {0,1,2});
    const hom h2 = sdd::hom::Cons<conf>(0, conf::Values {0,1,2});
    ASSERT_EQ(h1, h2);
  }
  {
    const hom h1 = sdd::hom::Cons<conf>(0, one);
    const hom h2 = sdd::hom::Cons<conf>(0, one);
    ASSERT_EQ(h1, h2);
  }
  {
    const hom h1 = sdd::hom::Cons<conf>(0, conf::Values {0,1,3});
    const hom h2 = sdd::hom::Cons<conf>(0, conf::Values {0,1,2});
    ASSERT_NE(h1, h2);
  }
  {
    const hom h1 = sdd::hom::Cons<conf>(0, one);
    const hom h2 = sdd::hom::Cons<conf>(0, zero);
    ASSERT_NE(h1, h2);
  }
  {
    const hom h1 = sdd::hom::Cons<conf>(0, one);
    const hom h2 = sdd::hom::Cons<conf>(0, conf::Values {0,1,2});
    ASSERT_NE(h1, h2);
  }
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_cons_test, evaluation)
{
  {
    const hom h = sdd::hom::Cons<conf>(0, conf::Values {0,1,2});
    ASSERT_EQ(SDD(0, {0,1,2}, one), h(one));
  }
  {
    const hom h = sdd::hom::Cons<conf>(0, conf::Values {});
    ASSERT_EQ(zero, h(one));
  }
  {
    const hom h = sdd::hom::Cons<conf>(0, one);
    ASSERT_EQ(SDD(0, one, one), h(one));
  }
}

/*-------------------------------------------------------------------------------------------*/
