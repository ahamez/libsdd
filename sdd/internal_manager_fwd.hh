/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

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
