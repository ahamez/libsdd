#ifndef _SDD_DD_TERMINAL_HH_
#define _SDD_DD_TERMINAL_HH_

#include <iosfwd>

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief The |0| terminal.
///
/// It represents the complement of paths leading to |1|.
template <typename C>
struct zero_terminal final
{};

/// @related zero_terminal
template <typename C>
inline
constexpr bool
operator==(const zero_terminal<C>&, const zero_terminal<C>&)
noexcept
{
  return true;
}

/// @related zero_terminal
template <typename C>
std::ostream&
operator<<(std::ostream& os, const zero_terminal<C>&)
{
  return os << "|0|";
}

/*------------------------------------------------------------------------------------------------*/

/// @brief The |1| terminal.
///
/// It indicates the end of a path in an SDD.
template <typename C>
struct one_terminal final
{};

/// @related one_terminal
template <typename C>
inline
constexpr bool
operator==(const one_terminal<C>&, const one_terminal<C>&)
noexcept
{
  return true;
}

/// @related one_terminal
template <typename C>
std::ostream&
operator<<(std::ostream& os, const one_terminal<C>&)
{
  return os << "|1|";
}

} // namespace sdd

/*------------------------------------------------------------------------------------------------*/

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
    return sdd::util::hash(0);
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
    return sdd::util::hash(1);
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_DD_TERMINAL_HH_
