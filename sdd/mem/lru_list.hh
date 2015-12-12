/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

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
