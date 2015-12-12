/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <iterator> // iterator_traits

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Move elements of a range to a given position.
template <typename InputIterator>
void
consolidate(char* addr, InputIterator it, InputIterator end)
noexcept
{
  using value_type = typename std::iterator_traits<InputIterator>::value_type;
  auto* base = reinterpret_cast<value_type*>(addr);
  while (it != end)
  {
    // A placement new is required to initialize memory.
    new (base++) value_type(std::move(*it++));
  }
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom
