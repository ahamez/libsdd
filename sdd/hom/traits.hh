/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Default traits for homomorphisms.
template <typename T>
struct homomorphism_traits
{
  static constexpr bool should_cache = true;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom
