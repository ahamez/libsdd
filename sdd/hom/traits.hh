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
