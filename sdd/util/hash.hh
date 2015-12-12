/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <algorithm> // for_each
#include <iterator>  // iterator_traits
#include <utility>   // declval

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
noexcept(noexcept(
  hash_combine(seed, std::declval<typename std::iterator_traits<InputIterator>::value_type>())
))
{
  std::for_each(cit, cend, [&](const auto& v){hash_combine(seed, v);});
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
struct seed
{
  const std::size_t seed_;

  seed(std::size_t s = 0)
  noexcept
    : seed_(s)
  {}

  template <typename T>
  seed(const T& x)
  noexcept(noexcept(std::hash<T>()(x)))
    : seed_(std::hash<T>()(x))
  {}

  template <typename Cont>
  auto
  operator()(const Cont& cont)
  const noexcept
  {
    return cont(seed_);
  }

  operator std::size_t()
  const noexcept
  {
    return seed_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename T>
auto val(const T& x)
noexcept
{
  return [&](std::size_t s) { hash_combine(s, x); return seed(s); };
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename T>
auto val(const boost::optional<T>& x)
noexcept
{
  return [&](std::size_t s) { if (x) hash_combine(s, *x); return seed(s); };
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename InputIterator>
auto
range(InputIterator begin, InputIterator end)
noexcept
{
  return [=](std::size_t s) { hash_combine(s, begin, end); return seed(s); };
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename Container>
auto
range(const Container& c)
noexcept
{
  return range(c.begin(), c.end());
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::hash
