#ifndef _SDD_TESTS_CONFIGURATION_HH_
#define _SDD_TESTS_CONFIGURATION_HH_

#include "sdd/conf/default_configurations.hh"

/*------------------------------------------------------------------------------------------------*/

typedef sdd::conf0 conf;

namespace /* anonymous */ {

conf
small_conf()
{
  conf c;
  c.sdd_unique_table_size = 1000;
  c.sdd_difference_cache_size = 1000;
  c.sdd_intersection_cache_size = 1000;
  c.sdd_sum_cache_size = 1000;
  c.hom_unique_table_size = 1000;
  c.hom_cache_size = 1000;
  return c;
}

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

#endif // _SDD_TESTS_CONFIGURATION_HH_
