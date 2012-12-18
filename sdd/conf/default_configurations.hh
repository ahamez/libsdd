#ifndef _SDD_CONF_DEFAULT_CONFIGURATIONS_HH_
#define _SDD_CONF_DEFAULT_CONFIGURATIONS_HH_

#include <string>

#include "sdd/values/bitset.hh"
#include "sdd/values/flat_set.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

struct default_parameters
{
  typedef unsigned short alpha_size_type;
  typedef unsigned int operands_size_type;
};

/*------------------------------------------------------------------------------------------------*/

struct conf0
  : public default_parameters
{
  typedef std::string        Identifier;
  typedef unsigned char      Variable;
  typedef values::bitset<64> Values;
};

/*------------------------------------------------------------------------------------------------*/

struct conf1
  : public default_parameters
{
  typedef std::string Identifier;
  typedef unsigned int Variable;
  typedef values::flat_set<unsigned int> Values;
};

/*------------------------------------------------------------------------------------------------*/

}

#endif // _SDD_CONF_DEFAULT_CONFIGURATIONS_HH_
