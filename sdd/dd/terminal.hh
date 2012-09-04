#ifndef _SDD_DD_TERMINAL_HH_
#define _SDD_DD_TERMINAL_HH_

#include <iosfwd>

namespace sdd { namespace dd {

/*-------------------------------------------------------------------------------------------*/

/// @brief The |0| terminal.
///
/// It represents the complement of paths leading to |1|.
template <typename C>
struct zero_terminal
{
};

/// @related zero_terminal
template <typename C>
inline
bool
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

/*-------------------------------------------------------------------------------------------*/

/// @brief The |1| terminal.
///
/// It indicates the end of a path in an SDD.
template <typename C>
struct one_terminal
{
};

/// @related one_terminal
template <typename C>
inline
bool
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

}} // namespace sdd::dd

/*-------------------------------------------------------------------------------------------*/

namespace std {

/*-------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::dd::zero_terminal.
template <typename C>
struct hash<sdd::dd::zero_terminal<C>>
{
  std::size_t
  operator()(const sdd::dd::zero_terminal<C>&)
  const noexcept
  {
    return std::hash<unsigned int>()(0);
  }
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::dd::one_terminal.
template <typename C>
struct hash<sdd::dd::one_terminal<C>>
{
  std::size_t
  operator()(const sdd::dd::one_terminal<C>&)
  const noexcept
  {
    return std::hash<unsigned int>()(1);
  }
};

/*-------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_DD_TERMINAL_HH_
