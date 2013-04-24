#ifndef _SDD_MANAGER_FWD_HH_
#define _SDD_MANAGER_FWD_HH_

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct internal_manager;

/*------------------------------------------------------------------------------------------------*/

template <typename C>
internal_manager<C>&
global() noexcept;

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_MANAGER_FWD_HH_
