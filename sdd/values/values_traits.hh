#ifndef _SDD_VALUES_VALUES_TRAITS_HH_
#define _SDD_VALUES_VALUES_TRAITS_HH_

namespace sdd { namespace values {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Describe some properties of values.
template <typename Values>
struct values_traits
{
  static constexpr bool stateful = false;
  static constexpr bool fast_iterable = false;
  static constexpr bool has_value_type = true;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::values

#endif // _SDD_VALUES_VALUES_TRAITS_HH_
