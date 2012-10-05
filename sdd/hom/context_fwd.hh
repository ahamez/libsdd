#ifndef _SDD_HOM_CONTEXT_FWD_HH_
#define _SDD_HOM_CONTEXT_FWD_HH_

/// @file  context_fwd.hh
/// @brief Forward declaration of a context.
///
/// Needed by homomorphisms evaluation.

namespace sdd { namespace hom {

/*-------------------------------------------------------------------------------------------*/

template <typename C>
class context;

template <typename C>
context<C>&
initial_context() noexcept;

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

#endif // _SDD_HOM_CONTEXT_FWD_HH_
