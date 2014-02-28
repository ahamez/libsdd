#ifndef _SDD_HOM_COMMON_TYPES_HH_
#define _SDD_HOM_COMMON_TYPES_HH_

#include <boost/container/flat_set.hpp>
#include <boost/optional.hpp>

#include "sdd/hom/definition_fwd.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The type of an optional homomorphism.
///
/// Useful for saturation_* operations.
template <typename C>
using optional_homomorphism = boost::optional<homomorphism<C>>;

/// @internal
/// @brief The type of a set of homomorphisms.
///
/// Useful for saturation_* operations.
template <typename C>
using homomorphism_set = boost::container::flat_set<homomorphism<C>>;

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

#endif // _SDD_HOM_COMMON_TYPES_HH_
