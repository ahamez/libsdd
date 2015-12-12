/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

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
