#ifndef _SDD_UTIL_HASH_HH_
#define _SDD_UTIL_HASH_HH_

#include <algorithm> // for_each
#include <iterator>

namespace sdd { namespace util {

/*------------------------------------------------------------------------------------------------*/

/// @brief Combine the hash value of x with seed.
///
/// Taken from <boost/functional/hash.hpp>. Sligthy modified to use std::hash<T> instead of
/// boost::hash_value().
template <typename T>
inline
void
hash_combine(std::size_t& seed, const T& x)
noexcept(noexcept(std::hash<T>()(x)))
{
  seed ^= std::hash<T>()(x) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Hash a range.
template <typename InputIterator>
inline
void
hash_combine(std::size_t& seed, InputIterator cit, InputIterator cend)
{
  using value_type = typename std::iterator_traits<InputIterator>::value_type;
  std::for_each(cit, cend, [&](const value_type& v){hash_combine(seed, v);});
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Call std::hash<>
template <typename T>
inline
std::size_t
hash(const T& x)
noexcept(noexcept(std::hash<T>()(x)))
{
  return std::hash<T>()(x);
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Call std::hash<>
template <typename InputIterator>
inline
std::size_t
hash(InputIterator cit, InputIterator cend)
noexcept(noexcept(std::hash<typename std::decay<decltype(*cit)>::type>()(*cit)))
{
  assert(cit != cend && "Empty range to hash.");
  std::size_t seed = hash(*cit);
  hash_combine(seed, cit + 1, cend);
  return seed;
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::util

#endif // _SDD_UTIL_HASH_HH_
