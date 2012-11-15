#ifndef _SDD_UTIL_TYPELIST_HH_
#define _SDD_UTIL_TYPELIST_HH_

namespace sdd { namespace util {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Represent the empty type.
///
/// Used by nth to indicate that a type was not found.
struct nil {};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename... Types>
struct typelist
{
};

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
template <std::size_t Index, typename... Types>
struct nth
{
  typedef nil type;
};

template <typename Head, typename... Tail>
struct nth<0, Head, Tail...>
{
  typedef Head type;
};
  
template <std::size_t Index, typename Head, typename... Tail>
struct nth<Index, Head, Tail...>
{
  typedef typename nth<Index - 1, Tail...>::type type;
};  

/*------------------------------------------------------------------------------------------------*/  

}} // namespace sdd::util

#endif // _SDD_UTIL_TYPELIST_HH_
