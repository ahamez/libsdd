#ifndef _SDD_VALUES_SIZE_HH_
#define _SDD_VALUES_SIZE_HH_

namespace sdd { namespace values {

/*------------------------------------------------------------------------------------------------*/

template <typename T>
std::size_t
size(const T& x)
noexcept(noexcept(x.size()))
{
  return x.size();
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::values

#endif // _SDD_VALUES_SIZE_HH_
