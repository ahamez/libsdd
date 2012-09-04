#ifndef _SDD_SDD_HH_
#define _SDD_SDD_HH_

#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"

/// @brief The main namespace for the library.
namespace sdd
{

/// @brief Contain all stuff related to SDD: definition and operations.
namespace dd {}

/// @brief Contain definition of values to be stored on arcs.
namespace values {}

/// @cond INTERNAL_DOC

/// @brief Contain stuff necessary for the library but that should not be exposed.
namespace internal {}

/// @brief Contain stuff necessary for the memory management.
namespace mem {}

/// @endcond

}

#endif // _SDD_SDD_HH_
