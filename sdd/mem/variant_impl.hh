#pragma once

#include <cassert>
#include <cstdint>     // uint8_t
#include <functional>  // hash
#include <iosfwd>
#include <type_traits> // aligned_storage, alignment_of
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
  const
  {
    x.~T();
  }
};

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
/// @brief Dispatch the equality operator to the contained type in the visited variant.
struct eq_visitor
{
  template <typename T>
  bool
  operator()(const T& lhs, const T& rhs)
  const noexcept(noexcept(lhs == rhs))
  {
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

template <typename Visitor, typename... Xs, typename... Args>
inline
std::result_of_t<Visitor(util::nth<0, Xs...>, Args&&...)>
apply_visitor(Visitor&& v, const variant<Xs...>& x, Args&&... args)
{
  using first_type = util::nth<0, Xs...>;
  using result_type = std::result_of_t<Visitor(first_type, Args&&...)>;

  using fun_ptr_type = result_type (*) (Visitor&&, const void*, Args&&...);

  static constexpr fun_ptr_type table[] = {&call<Visitor, Xs, Args&&...>...};

  return table[x.index](std::forward<Visitor>(v), x.storage(), std::forward<Args>(args)...);
}

/*------------------------------------------------------------------------------------------------*/

template <typename Visitor, typename X, typename Y, typename... Args>
inline
auto
binary_call(Visitor&& v, const X& x, const void* y, Args&&... args)
noexcept(noexcept(v(x, *static_cast<const Y*>(y), std::forward<Args>(args)...)))
-> decltype(auto)
{
  return v(x, *static_cast<const Y*>(y), std::forward<Args>(args)...);
}

/*------------------------------------------------------------------------------------------------*/

template <typename Visitor, typename X, typename... Ys, typename... Args>
inline
auto
inner_visit_impl(Visitor&& v, const X& x, const variant<Ys...>& y, Args&&... args)
-> decltype(auto)
{
  using first_y_type = util::nth<0, Ys...>;
  using result_type = std::result_of_t<Visitor(X, first_y_type, Args&&...)>;

  using fun_ptr_type = result_type (*) (Visitor&&, const X&, const void*, Args&&...);

  static constexpr fun_ptr_type table[] = {&binary_call<Visitor, X, Ys, Args&&...>...};

  return table[y.index](std::forward<Visitor>(v), x, y.storage(), std::forward<Args>(args)...);
}

template <typename Visitor, typename X, typename YVariant, typename... Args>
inline
auto
inner_visit(Visitor&& v, const void* x, const YVariant& y, Args&&... args)
-> decltype(auto)
{
  return inner_visit_impl<>( std::forward<Visitor>(v), *static_cast<const X*>(x)
                           , y, std::forward<Args>(args)...);
}

/*------------------------------------------------------------------------------------------------*/

template <typename Visitor, typename... Xs, typename... Ys, typename... Args>
inline
std::result_of_t<Visitor(util::nth<0, Xs...>, util::nth<0, Ys...>, Args&&...)>
apply_binary_visitor(Visitor&& v, const variant<Xs...>& x, const variant<Ys...>& y, Args&&... args)
{
  using first_x_type = util::nth<0, Xs...>;
  using first_y_type = util::nth<0, Ys...>;
  using result_type = std::result_of_t<Visitor(first_x_type, first_y_type, Args&&...)>;

  using fun_ptr_type = result_type (*) (Visitor&&, const void*, const variant<Ys...>&, Args&&...);
 
  static constexpr fun_ptr_type table[]
    = {&inner_visit<Visitor&&, Xs, variant<Ys...>, Args&&...>...};
 
  return table[x.index](std::forward<Visitor>(v), x.storage(), y, std::forward<Args>(args)...);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem
