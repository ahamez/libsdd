#ifndef _SDD_SDD_HH_
#define _SDD_SDD_HH_

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/dd/paths.hh"
#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"
#include "sdd/hom/rewrite.hh"

/// @brief The main namespace for the library.
namespace sdd
{

/// @brief Internal stuff related to the library configuration.
namespace conf {}

/// @brief Internal stuff for decision diagrams.
namespace dd {}

/// @brief Internal stuff for homomorphisms.
namespace hom {}

/// @brief Internal stuff necessary for the memory management.
namespace mem {}

/// @brief Contain miscellaneous utilities.
namespace util {}

/// @brief Definition of values to be stored on arcs.
namespace values {}

/// @brief Essentially hash specialization.
namespace std {}
}

#endif // _SDD_SDD_HH_
