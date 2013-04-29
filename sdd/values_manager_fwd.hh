#ifndef _SDD_VALUES_MANAGER_FWD_HH_
#define _SDD_VALUES_MANAGER_FWD_HH_

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

template <typename Values>
struct values_manager;

/*------------------------------------------------------------------------------------------------*/

template <typename Values>
values_manager<Values>&
global_values() noexcept;

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_VALUES_MANAGER_FWD_HH_
