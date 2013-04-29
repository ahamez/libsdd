#ifndef _SDD_VALUES_MANAGER_HH_
#define _SDD_VALUES_MANAGER_HH_

#include <cassert>

#include "sdd/internal_manager_fwd.hh"
#include "sdd/mem/unique_table.hh"
#include "sdd/values/values_traits.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

namespace detail {

/// @brief Signature of the structure that will store, if necessary, the state needed by C::Values.
template <typename Values, bool>
struct values_state;

/// @brief Specialization for a stateful set of values.
template <typename Values>
struct values_state<Values, true>
{
  /// @brief The actual state.
  typename values::values_traits<Values>::state_type state;

  /// @brief Constructor.
  template <typename C>
  values_state(const C& configuration)
    : state(configuration)
  {}
};

/// @brief Specialization for a stateless set of values.
template <typename Values>
struct values_state<Values, false>
{
  /// @brief Constructor.
  template <typename C>
  values_state(const C&)
  {}
};

/// @brief Dispatch on the correct type of values_state.
template <typename Values>
using values_state_type = values_state<Values, values::values_traits<Values>::stateful>;

} // namespace detail

/// @internal
/// @brief Hold the state that the set of values may need.
template <typename Values>
struct values_manager
  : public detail::values_state_type<Values>
{
  /// @brief Constructor.
  template <typename C>
  values_manager(const C& configuration)
    : detail::values_state_type<Values>(configuration)
  {}
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Contains the global (static) values manager.
/// @related values_manager
template <typename Values>
inline
values_manager<Values>**
global_values_ptr()
noexcept
{
  static values_manager<Values>* m = nullptr;
  return &m;
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Get the global values context as a reference.
/// @related values_manager
///
/// Make internal calls to the global values_manager easier.
template <typename Values>
inline
values_manager<Values>&
global_values()
noexcept
{
  assert(*global_values_ptr<Values>() != nullptr && "Uninitialized global values_manager");
  return **global_values_ptr<Values>();
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_VALUES_MANAGER_HH_
