#ifndef _SDD_HOM_TRAITS_HH_
#define _SDD_HOM_TRAITS_HH_

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

#endif // _SDD_HOM_TRAITS_HH_
