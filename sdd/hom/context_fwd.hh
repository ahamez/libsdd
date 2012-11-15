#ifndef _SDD_HOM_CONTEXT_FWD_HH_
#define _SDD_HOM_CONTEXT_FWD_HH_

namespace sdd { namespace hom {

/*-------------------------------------------------------------------------------------------*/

// Forward declaration needed for homomorphism evaluation.
template <typename C>
class context;

// Forward declaration needed for homomorphism evaluation.
template <typename C>
context<C>&
initial_context() noexcept;

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

#endif // _SDD_HOM_CONTEXT_FWD_HH_
