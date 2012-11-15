#ifndef _SDD_INTERNAL_UTIL_HASH_HH_
#define _SDD_INTERNAL_UTIL_HASH_HH_

namespace sdd { namespace internal { namespace util {

/*-------------------------------------------------------------------------------------------*/

/// @brief Combine the hash value of x with seed.
///
/// Taken from <boost/functional/hash.hpp>. Sligthy modified to use std::hash<T> instead of
/// boost::hash_value().
template <typename T>
inline
void
hash_combine(std::size_t& seed, const T& x)
{
  seed ^= std::hash<T>()(x) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}
/*-------------------------------------------------------------------------------------------*/

}}} // namespace sdd::internal::util

#endif // _SDD_INTERNAL_UTIL_NIL_HH_
