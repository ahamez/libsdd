/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <iosfwd>

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief The |0| terminal.
///
/// It represents the complement of paths leading to |1|.
template <typename C>
struct zero_terminal final
{
  friend
  constexpr bool
  operator==(const zero_terminal&, const zero_terminal&)
  noexcept
  {
    return true;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const zero_terminal&)
  {
    return os << "|0|";
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief The |1| terminal.
///
/// It indicates the end of a path in an SDD.
template <typename C>
struct one_terminal final
{
  friend
  constexpr bool
  operator==(const one_terminal&, const one_terminal&)
  noexcept
  {
    return true;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const one_terminal&)
  {
    return os << "|1|";
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::dd::zero_terminal.
template <typename C>
struct hash<sdd::zero_terminal<C>>
{
  std::size_t
  operator()(const sdd::zero_terminal<C>&)
  const noexcept
  {
    return 0;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::dd:one_terminal.
template <typename C>
struct hash<sdd::one_terminal<C>>
{
  std::size_t
  operator()(const sdd::one_terminal<C>&)
  const noexcept
  {
    return 1;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
