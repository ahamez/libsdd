/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <cassert>
#include <functional>  // hash
#include <iosfwd>
#include <type_traits> // aligned_storage, alignment_of, result_of
#include <utility>     // forward

#include "sdd/util/typelist.hh"

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename T, typename... Types>
struct largest_alignment
{
  static constexpr std::size_t tail  = largest_alignment<Types...>::value;
  static constexpr std::size_t head  = std::alignment_of<T>::value;
  static constexpr std::size_t value = tail > head ? tail : head;
};

template <typename T>
struct largest_alignment<T>
{
  static constexpr std::size_t value = std::alignment_of<T>::value;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename T, typename... Types>
struct largest_size
{
  static constexpr std::size_t tail  = largest_size<Types...>::value;
  static constexpr std::size_t head  = sizeof(T);
  static constexpr std::size_t value = tail > head ? tail : head;
};

template <typename T>
struct largest_size<T>
{
  static constexpr std::size_t value = sizeof(T);
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief A storage large enough for the biggest T in Types.
///
/// When the packed mode is required (at compilation time), we don't care about alignement to save
/// on memory.
template <std::size_t Len, typename... Types>
struct union_storage
{
  static constexpr std::size_t max_size = largest_size<Types...>::value;
#ifdef LIBSDD_PACKED
  static constexpr std::size_t alignment_value = 1;
#else
  static constexpr std::size_t alignment_value = largest_alignment<Types...>::value;
#endif
  using type = typename std::aligned_storage< (Len > max_size ? Len : max_size)
                                            , alignment_value
                                            >::type;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Dispatch the destructor to the contained type in the visited variant.
struct dtor_visitor
{
  template <typename T>
  void
  operator()(const T& x)
  const noexcept(noexcept(x.~T()))
  {
    x.~T();
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
struct hash_visitor
{
  template <typename T>
  std::size_t
  operator()(const T& x)
  const noexcept(noexcept(std::hash<T>()(x)))
  {
    return std::hash<T>()(x);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
struct eq_visitor
{
  template <typename T>
  bool
  operator()(const T& lhs, const T& rhs)
  const noexcept
  {
    static_assert(noexcept(lhs == rhs), "Comparison should be noexcept");
    return lhs == rhs;
  }

  template <typename T, typename U>
  bool
  operator()(const T&, const U&)
  const noexcept
  {
    assert(false);
    __builtin_unreachable();
  }
};

/*------------------------------------------------------------------------------------------------*/

template <typename... Types>
struct variant;

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename Visitor, typename X, typename... Args>
inline
auto
call(Visitor&& v, const void* x, Args&&... args)
noexcept(noexcept(v(*static_cast<const X*>(x), std::forward<Args>(args)...)))
-> decltype(auto)
{
  return v(*static_cast<const X*>(x), std::forward<Args>(args)...);
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename Visitor, typename... Xs, typename... Args>
inline
std::result_of_t<Visitor(util::nth_t<0, Xs...>, Args&&...)>
apply_visitor(Visitor&& v, const variant<Xs...>& x, Args&&... args)
{
  using fun_ptr_type
    = std::result_of_t<Visitor(util::nth_t<0, Xs...>, Args&&...)>
      (*) (Visitor&&, const void*, Args&&...);

  static constexpr fun_ptr_type table[] = {&call<Visitor, Xs, Args&&...>...};
  return table[x.index](std::forward<Visitor>(v), &x.storage, std::forward<Args>(args)...);
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename Visitor, typename X, typename Y, typename... Args>
inline
auto
binary_call(Visitor&& v, const void* x, const void* y, Args&&... args)
-> decltype(auto)
{
  return v(*static_cast<const X*>(x), *static_cast<const Y*>(y), std::forward<Args>(args)...);
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Expands a list of pairs of types into pointers to binary_call.
/// @see http://stackoverflow.com/a/25661431/21584
template<typename Visitor, typename T, std::size_t N, typename... Args>
struct binary_jump_table
{
    template<class... Xs, class... Ys>
    constexpr binary_jump_table(util::list<util::pair<Xs, Ys>...>)
      : table{&binary_call<Visitor, Xs, Ys, Args...>...}
    {}

    T table[N];
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename Visitor, typename... Xs, typename... Ys, typename... Args>
std::result_of_t<Visitor(util::nth_t<0, Xs...>, util::nth_t<0, Ys...>, Args&&...)>
apply_binary_visitor(Visitor&& v, const variant<Xs...>& x, const variant<Ys...>& y, Args&&... args)
{
  using fun_ptr_type
    = std::result_of_t<Visitor(util::nth_t<0, Xs...>, util::nth_t<0, Ys...>, Args&&...)>
      (*) (Visitor&&, const void*, const void*, Args&&...);

  static constexpr binary_jump_table<Visitor, fun_ptr_type, sizeof...(Xs) * sizeof...(Ys), Args...>
    table = {typename util::join<util::list<Xs...>, util::list<Ys...>>::type()};

  return table.table[x.index * sizeof...(Ys) + y.index]
    (std::forward<Visitor>(v), &x.storage, &y.storage, std::forward<Args>(args)...);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem
