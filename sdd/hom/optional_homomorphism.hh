#ifndef _SDD_HOM_OPTIONAL_HOMOMORPHISM_HH_
#define _SDD_HOM_OPTIONAL_HOMOMORPHISM_HH_

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

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

#endif // _SDD_HOM_OPTIONAL_HOMOMORPHISM_HH_
