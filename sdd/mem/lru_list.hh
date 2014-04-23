#ifndef _SDD_MEM_LRU_LIST_HH_
#define _SDD_MEM_LRU_LIST_HH_

#include <list>

#include "sdd/mem/cache_entry_fwd.hh"

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @brief The type of the container that sorts cache entries by last access date.
template <typename Operation, typename Result>
using lru_list_type = std::list<cache_entry<Operation, Result>*>;

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem


#endif // _SDD_MEM_LRU_LIST_HH_
