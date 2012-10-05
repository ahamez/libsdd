#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"

/*-------------------------------------------------------------------------------------------*/

struct hom_fixpoint_test
: public testing::Test
{
  typedef sdd::conf::conf0 conf;
  typedef sdd::SDD<conf> SDD;
  typedef sdd::hom::homomorphism<conf> hom;
  const SDD zero = sdd::zero<conf>();
  const SDD one = sdd::one<conf>();

  hom_fixpoint_test()
  {
  }
};

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_fixpoint_test, construction)
{
}

/*-------------------------------------------------------------------------------------------*/

TEST_F(hom_fixpoint_test, evaluation)
{
}

/*-------------------------------------------------------------------------------------------*/
