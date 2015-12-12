/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <limits>
#include <type_traits> // enable_if

namespace sdd { namespace util {

/*------------------------------------------------------------------------------------------------*/

/// @brief Get the next highest power of 2.
///
/// 64 bits specialization.
/// http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
template <typename T>
inline
std::enable_if_t<std::numeric_limits<T>::digits == 64, T>
next_power_of_2(T x)
{
  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x |= x >> 32;
  x++;
  return x;
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Get the next highest power of 2.
///
/// 32 bits specialization.
/// http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
template <typename T>
inline
std::enable_if_t<std::numeric_limits<T>::digits == 32, T>
next_power_of_2(T x)
{
  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x++;
  return x;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::util
