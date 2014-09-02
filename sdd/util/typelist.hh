#pragma once

namespace sdd { namespace util {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename T, typename... Types>
struct index_of;

template <typename T, typename... Types>
struct index_of<T, T, Types...>
{
  static constexpr std::size_t value = 0;
};

template <typename T, typename Head, typename... Types>
struct index_of<T, Head, Types...>
{
  static constexpr std::size_t value = index_of<T, Types...>::value + 1;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <std::size_t, typename...>
struct nth_impl;

template <typename T, typename... Ts>
struct nth_impl<0, T, Ts...>
{
  using type = T;
};
  
template <std::size_t Index, typename T, typename... Ts>
struct nth_impl<Index, T, Ts...>
{
  static_assert(Index < sizeof...(Ts) + 1 /* + 1 for T */, "Index too large for nth");
  using type = typename nth_impl<Index - 1, Ts...>::type;
}; 

template <std::size_t Index, typename... Ts>
using nth = typename nth_impl<Index, Ts...>::type;

/*------------------------------------------------------------------------------------------------*/  

}} // namespace sdd::util
