/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

template <typename Values>
struct values_manager;

/*------------------------------------------------------------------------------------------------*/

template <typename Values>
values_manager<Values>&
global_values() noexcept;

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd
