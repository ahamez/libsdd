#pragma once

#include <algorithm> // for_each
#include <cassert>

#include <boost/optional.hpp>

namespace sdd { namespace hash {

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
  std::for_each(cit, cend, [&](const auto& v){hash_combine(seed, v);});
}

/*------------------------------------------------------------------------------------------------*/

namespace /* anonymous */ {

/// @internal
struct seed
{
  std::size_t seed_;

  seed(std::size_t s = 0)
  noexcept
    : seed_(s)
  {}

  template <typename Cont>
  auto
  operator()(const Cont& cont)
  const noexcept
  -> decltype(auto)
  {
    return cont(seed_);
  }

  operator std::size_t()
  const noexcept
  {
    return seed_;
  }
};

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename T>
auto val(const T& x)
{
  return [&](std::size_t s) { hash_combine(s, x); return seed(s); };
}

/*------------------------------------------------------------------------------------------------*/

template <typename T>
auto val(const boost::optional<T>& x)
{
  return [&](std::size_t s) { if (x) hash_combine(s, *x); return seed(s); };
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename InputIterator>
auto
range(InputIterator begin, InputIterator end)
{
  return [=](std::size_t s) { hash_combine(s, begin, end); return seed(s); };
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename Container>
auto
range(const Container& c)
{
  return range(c.begin(), c.end());
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::hash
