#pragma once

#include <list>

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The type of the container that sorts cache entries by last access date.
template <typename CacheEntry>
using lru_list = std::list<CacheEntry*>;

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem
