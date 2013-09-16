#ifndef _SDD_VALUES_EMPTY_HH_
#define _SDD_VALUES_EMPTY_HH_

namespace sdd { namespace values {

/*------------------------------------------------------------------------------------------------*/

template <typename T>
auto
empty_values_impl(const T& x, int)
noexcept(noexcept(x.empty()))
-> decltype(x.empty())
{
  return x.empty();
}

template <typename T>
auto
empty_values_impl(const T& x, long)
noexcept(noexcept(size(x)))
-> decltype(bool())
{
  return empty(x);
}

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
