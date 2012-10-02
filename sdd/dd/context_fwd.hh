#ifndef _SDD_DD_CONTEXT_FWD_HH_
#define _SDD_DD_CONTEXT_FWD_HH_

/// @file  context_fwd.hh
/// @brief Forward declaration of a context.
///
/// Needed by operations.

namespace sdd {

/*-------------------------------------------------------------------------------------------*/

template <typename C>
class context;

template <typename C>
context<C>&
initial_context() noexcept;

/*-------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_DD_CONTEXT_FWD_HH_
