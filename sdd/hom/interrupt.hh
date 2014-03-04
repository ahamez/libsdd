#ifndef _SDD_HOM_INTERRUPT_HH_
#define _SDD_HOM_INTERRUPT_HH_

#include <exception>

#include "sdd/dd/definition.hh"
#include "sdd/mem/interrupt.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @exception interrupt
/// @brief Throw this exception to interrupt the evaluation of an homomorphism.
template <typename C>
using interrupt = mem::interrupt<SDD<C>>;

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_HOM_INTERRUPT_HH_
