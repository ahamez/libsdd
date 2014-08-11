#pragma once

#include <list>

#include "sdd/mem/cache_entry_fwd.hh"

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The type of the container that sorts cache entries by last access date.
template <typename Operation, typename Result>
using lru_list = std::list<cache_entry<Operation, Result>*>;

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem
