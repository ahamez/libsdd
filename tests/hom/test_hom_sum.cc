#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"

/*-------------------------------------------------------------------------------------------*/

struct hom_sum_test
  : public testing::Test
{
  typedef sdd::conf::conf0 conf;
  typedef sdd::SDD<conf> SDD;
  typedef sdd::hom::homomorphism<conf> hom;
  const SDD zero = sdd::zero<conf>();
  const SDD one = sdd::one<conf>();
};

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_sum_test, construction)
{
  const hom h1 = sdd::hom::Sum({sdd::hom::Id<conf>()});
  const hom h2 = sdd::hom::Sum({sdd::hom::Id<conf>()});
  ASSERT_EQ(h1, h2);
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
