#ifndef _SDD_SDD_HH_
#define _SDD_SDD_HH_

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/order/utility.hh"

/// @brief The main namespace for the library.
namespace sdd
{

/// @brief Stuff related to SDD: definition and operations.
namespace dd {}

/// @brief Stuff necessary to describe and manipulate variable orders.
namespace order {}

/// @brief Definition of values to be stored on arcs.
namespace values {}

/// @cond INTERNAL_DOC

/// @brief Contain internal stuff necessary for the library.
namespace internal {}

/// @brief Contain stuff necessary for the memory management.
namespace mem {}

/// @endcond

}

#endif // _SDD_SDD_HH_
