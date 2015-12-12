/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

namespace sdd { namespace values {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Describe some properties of values.
template <typename Values>
struct values_traits
{
  static constexpr bool stateful = false;
  static constexpr bool fast_iterable = false;
  static constexpr bool has_value_type = true;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::values
