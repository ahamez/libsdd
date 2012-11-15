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

/// @brief Definition of values to be stored on arcs.
namespace values {}

/// @internal
/// @brief Definition and evaluation of homomorphisms.
namespace hom {}

/// @internal
/// @brief Contain stuff necessary for the memory management.
namespace mem {}

/// @internal
/// @brief Contain miscellaneous utilities.
namespace util {}

}

#endif // _SDD_SDD_HH_
