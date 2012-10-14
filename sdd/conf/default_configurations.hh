#ifndef _SDD_CONF_DEFAULT_CONFIGURATIONS_HH_
#define _SDD_CONF_DEFAULT_CONFIGURATIONS_HH_

#include <string>

#include "sdd/values/bitset.hh"

namespace sdd { namespace conf {

/*-------------------------------------------------------------------------------------------*/

struct default_parameters
{
  typedef unsigned short alpha_size_type;
  typedef unsigned short operands_size_type;
};

/*-------------------------------------------------------------------------------------------*/

struct conf0
  : public default_parameters
{
  typedef unsigned char      Variable;
  typedef values::bitset<64> Values;
};

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::conf

#endif // _SDD_CONF_DEFAULT_CONFIGURATIONS_HH_
