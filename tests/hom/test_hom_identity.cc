#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"

/*-------------------------------------------------------------------------------------------*/

struct hom_id_test
  : public testing::Test
{
  typedef sdd::conf::conf0 conf;
  typedef sdd::SDD<conf> SDD;
  typedef sdd::hom::homomorphism<conf> hom;
  const SDD zero = sdd::zero<conf>();
  const SDD one = sdd::one<conf>();

  hom_id_test()
  {
  }
};

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_id_test, construction)
{
  hom h1 = sdd::hom::Id<conf>();
  hom h2 = sdd::hom::Id<conf>();
  ASSERT_EQ(h1, h2);
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_id_test, evaluation)
{
  hom h = sdd::hom::Id<conf>();
  ASSERT_EQ(one, h(one));
}

/*-------------------------------------------------------------------------------------------*/
