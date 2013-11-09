#ifndef _SDD_TESTS_CONFIGURATION_HH_
#define _SDD_TESTS_CONFIGURATION_HH_

#include "gtest/gtest.h"

#include "sdd/conf/default_configurations.hh"

/*------------------------------------------------------------------------------------------------*/

using conf0 = sdd::conf0;
using conf1 = sdd::conf1;

template <typename C>
C
small_conf()
noexcept
{
  C c;
  c.sdd_unique_table_size = 1000;
  c.sdd_difference_cache_size = 1000;
  c.sdd_intersection_cache_size = 1000;
  c.sdd_sum_cache_size = 1000;
  c.hom_unique_table_size = 1000;
  c.hom_cache_size = 1000;
  return c;
}

/*------------------------------------------------------------------------------------------------*/

using configurations = ::testing::Types<conf0, conf1>;

/*------------------------------------------------------------------------------------------------*/

#endif // _SDD_TESTS_CONFIGURATION_HH_
