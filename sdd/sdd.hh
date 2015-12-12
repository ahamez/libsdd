/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"
#include "sdd/hom/rewrite.hh"
#include "sdd/order/carrier.hh"

#include "sdd/manager.hh"

/// @brief The main namespace for the library.
namespace sdd
{

/// @internal
/// @brief Internal stuff related to the library configuration.
namespace conf {}

/// @internal
/// @brief Internal stuff for decision diagrams.
namespace dd {}

/// @internal
/// @brief Internal stuff for homomorphisms.
namespace hom {}

/// @internal
/// @brief Internal stuff necessary for the memory management.
namespace mem {}

/// @internal
/// @brief Contain miscellaneous utilities.
namespace util {}

/// @internal
/// @brief Definition of values to be stored on arcs.
namespace values {}

} // namespace sdd

/// @brief Essentially hash specialization.
namespace std {}
