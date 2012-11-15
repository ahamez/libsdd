#ifndef _SDD_DD_CONTEXT_FWD_HH_
#define _SDD_DD_CONTEXT_FWD_HH_

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

// Forward declaration needed by operations.
template <typename C>
class context;

// Forward declaration needed by operations.
template <typename C>
context<C>&
initial_context() noexcept;

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_DD_CONTEXT_FWD_HH_
