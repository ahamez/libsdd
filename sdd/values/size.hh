/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

namespace sdd { namespace values {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Return the size of a set of values.
///
/// If the function member size() is undefined for a particular set of values, one has just to
/// implement the function size() in its own namespace.
template <typename T>
std::size_t
size(const T& x)
noexcept(noexcept(x.size()))
{
  return x.size();
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::values
