#ifndef _SDD_HOM_CONSOLIDATE_HH_
#define _SDD_HOM_CONSOLIDATE_HH_

#include <iterator> // iterator_traits

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename InputIterator>
void
consolidate(char* addr, InputIterator it, InputIterator end)
noexcept
{
  typedef typename std::iterator_traits<InputIterator>::value_type value_type;
  value_type* base = reinterpret_cast<value_type*>(addr);
  std::size_t i = 0;
  for (; it != end; ++it)
  {
    new (base + i++) value_type(std::move(*it));
  }
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

#endif // _SDD_HOM_CONSOLIDATE_HH_
