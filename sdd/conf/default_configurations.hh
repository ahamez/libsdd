#ifndef _SDD_CONF_DEFAULT_CONFIGURATIONS_HH_
#define _SDD_CONF_DEFAULT_CONFIGURATIONS_HH_

#include <string>

#include "sdd/values/bitset.hh"
#include "sdd/values/flat_set.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

struct default_configuration
{
  typedef unsigned short alpha_size_type;
  typedef unsigned int operands_size_type;

  std::size_t sdd_unique_table_size;
  std::size_t sdd_difference_cache_size;
  std::size_t sdd_intersection_cache_size;
  std::size_t sdd_sum_cache_size;
  std::size_t hom_unique_table_size;
  std::size_t hom_cache_size;

  default_configuration()
    : sdd_unique_table_size(1000000)
    , sdd_difference_cache_size(500000)
    , sdd_intersection_cache_size(500000)
    , sdd_sum_cache_size(1000000)
    , hom_unique_table_size(1000000)
    , hom_cache_size(1000000)
  {
  }
};

/*------------------------------------------------------------------------------------------------*/

struct conf0
  : public default_configuration
{
  typedef std::string        Identifier;
  typedef unsigned char      Variable;
  typedef values::bitset<64> Values;
};

/*------------------------------------------------------------------------------------------------*/

struct conf1
  : public default_configuration
{
  typedef std::string                    Identifier;
  typedef unsigned int                   Variable;
  typedef values::flat_set<unsigned int> Values;

  std::size_t flat_set_unique_table_size;

  conf1()
    : default_configuration()
    , flat_set_unique_table_size(1000)
  {
  }
};

/*------------------------------------------------------------------------------------------------*/

}

#endif // _SDD_CONF_DEFAULT_CONFIGURATIONS_HH_
