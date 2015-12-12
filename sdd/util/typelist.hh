/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

namespace sdd { namespace util {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename T, typename Head, typename... Types>
struct index_of
{
  static constexpr std::size_t value = index_of<T, Types...>::value + 1;
};

/// @internal
template <typename T, typename... Types>
struct index_of<T, T, Types...>
{
  static constexpr std::size_t value = 0;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <std::size_t Index, typename T, typename... Ts>
struct nth
{
  static_assert(Index < sizeof...(Ts) + 1 /* + 1 for T */, "Index too large for nth");
  using type = typename nth<Index - 1, Ts...>::type;
};

/// @internal
template <typename T, typename... Ts>
struct nth<0, T, Ts...>
{
  using type = T;
};

/// @internal
template <std::size_t Index, typename... Ts>
using nth_t = typename nth<Index, Ts...>::type;

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename X, typename Y>
struct pair {};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename... Xs>
struct list {};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename X, typename... Ys>
using mul = list<pair<X, Ys>...>;

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename...>
struct cat;

template <typename X>
struct cat<X>
{
  using type = X;
};

template<typename X, typename Y, typename... Zs>
struct cat<X, Y, Zs...>
{
  using type = typename cat<typename cat<X, Y>::type, Zs...>::type;
};

template <typename... Xs, typename... Ys>
struct cat<list<Xs...>, list<Ys...>>
{
  using type = list<Xs..., Ys...>;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename...>
struct join {};

template <typename... Xs, typename... Ys>
struct join<list<Xs...>, list<Ys...>>
{
  using type = typename cat<mul<Xs, Ys...>...>::type;
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::util
