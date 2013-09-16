#ifndef _SDD_VALUES_EMPTY_HH_
#define _SDD_VALUES_EMPTY_HH_

namespace sdd { namespace values {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename T>
auto
empty_values_impl(const T& x, int)
noexcept(noexcept(x.empty()))
-> decltype(x.empty())
{
  return x.empty();
}

/// @internal
template <typename T>
auto
empty_values_impl(const T& x, long)
noexcept(noexcept(size(x)))
-> decltype(bool())
{
  return empty(x);
}

/// @internal
/// @brief Tell if a set of values is empty.
///
/// Whether the set of valuations contains or not the empty() member function, it will dispatch the
/// call to the member function or the free function found by ADL.
template <typename T>
bool
empty_values(const T& x)
noexcept(noexcept(empty_values_impl(x, 0)))
{
  return empty_values_impl(x, 0);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::values

#endif // _SDD_VALUES_EMPTY_HH_
