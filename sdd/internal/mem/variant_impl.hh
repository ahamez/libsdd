#ifndef _SDD_INTERNAL_MEM_VARIANT_IMPL_HH_
#define _SDD_INTERNAL_MEM_VARIANT_IMPL_HH_

#include <cassert>
#include <cstdint>     // uint8_t
#include <functional>  // hash
#include <iosfwd>
#include <type_traits> // enable_if, forward, is_same, aligned_storage, alignment_of
#include <utility>     // forward

#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/cat.hpp>

#include "sdd/internal/util/typelist.hh"

namespace sdd { namespace internal { namespace mem {

/*------------------------------------------------------------------------------------------------*/

using std::enable_if;
using std::is_same;
using util::nil;

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
template <std::size_t Len, typename... Types>
struct union_storage
{
  static constexpr std::size_t max_size = largest_size<Types...>::value;
#ifdef LIBSDD_PACKED
  static constexpr std::size_t alignment_value = 1;
#else
  static constexpr std::size_t alignment_value = largest_alignment<Types...>::value;
#endif
  typedef typename std::aligned_storage< (Len > max_size ? Len : max_size)
                                       , alignment_value
                                       >::type
                   type;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Dispatch the destructor to the contained type in the visited variant.
struct dtor_visitor
{
  typedef void result_type;

  template <typename T>
  result_type
  operator()(const T& x)
  const noexcept(noexcept(x.~T()))
  {
    x.~T();
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Dispatch the hash function to the contained type in the visited variant.
///
/// It uses the standard (C++11) way of hashing an object.
struct hash_visitor
{
  typedef std::size_t result_type;

  template <typename T>
  result_type
  operator()(const T& x)
  const noexcept(noexcept(std::hash<T>()(x)))
  {
    return std::hash<T>()(x);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// Dispatch the ostream export to the contained type in the visited variant.
struct ostream_visitor
{
  typedef std::ostream& result_type;

  std::ostream& os_;

  ostream_visitor(std::ostream& os)
	  : os_(os)
  {
  }

  template <typename T>
  result_type
  operator()(const T& x)
  const
  {
    return os_ << x;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Dispatch the equality operator to the contained type in the visited variant.
struct eq_visitor
{
  typedef bool result_type;

  template <typename T>
  result_type
  operator()(const T& lhs, const T& rhs)
  const noexcept(noexcept(lhs == rhs))
  {
    return lhs == rhs;
  }

  template <typename T, typename U>
  result_type
  operator()(const T&, const U&)
  const noexcept
  {
    assert(false);
    __builtin_unreachable();
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Invokation out-of-bounds case.
///
/// Used by dispatch* to narrow down the possible invocations of visitor only to the types
/// held by the visited variant. It thus avoid to apply the visitor on the nil type which is
/// returned by util::nth when the provided index does not match any held type. It is
/// necessary as the preprocessor will generate instantations of such invocations.
template <typename Visitor, typename T, typename... Args>
inline
typename enable_if<is_same<T, nil>::value, typename Visitor::result_type>::type
invoke(const Visitor&, const T&, Args&&...)
noexcept
{
  assert(false);
  __builtin_unreachable();
}

/// @internal
/// @brief Invoke user's visitor for deduced type.
template <typename Visitor, typename T, typename... Args>
inline
typename enable_if<not is_same<T, nil>::value, typename Visitor::result_type>::type
invoke(const Visitor& v, const T& x, Args&&... args)
noexcept(noexcept(v(x, std::forward<Args>(args)...)))
{
  return v(x, std::forward<Args>(args)...);
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <uint8_t Index>
struct dispatch
{

  /// Instanciated for any index generated by the preprocessor that is greater than the number
  /// of contained types in the variant, thus avoiding compilation error.
  template < typename Visitor
           , typename Storage, typename... Types, template <typename...> class Tuple
           , typename... Args
           >
  typename enable_if<Index >= sizeof...(Types), typename Visitor::result_type>::type
  operator()(const Visitor&, const Storage&, const Tuple<Types...>&, const uint8_t, Args&&...)
  noexcept
  {
    assert(false);
    __builtin_unreachable();
  }

  /// Dispatch of the visitor on the held type.
  template < typename Visitor
           , typename Storage, typename... Types, template <typename...> class Tuple
           , typename... Args
           >
  typename enable_if<Index < sizeof...(Types), typename Visitor::result_type>::type
  operator()( const Visitor& v
            , const Storage& storage, const Tuple<Types...>& tuple, const uint8_t index
            , Args&&... args)
  {
    // To avoid huge generated functions, you can reduce the number of cases in the switch
    // statement. Though, it can implies recursive calls for the last types of the variant.
#ifndef SDD_VARIANT_UNROLL_UNARY_SIZE
#  define UNROLL_SIZE 15
#else
#  define UNROLL_SIZE SDD_VARIANT_UNROLL_UNARY_SIZE
#endif

    // the held type is represented by the index stored in the variant
    switch (index)
    {
#define BOOST_PP_LOCAL_MACRO(n)                                             \
      case n: {                                                             \
                /* first, find the held type */                             \
                typedef typename util::nth<Index + n, Types...>::type T;    \
                /* then, see the stored data as this type */                \
                const T& x = *reinterpret_cast<const T*>(&storage);         \
                /* finally, we can apply the visitor on the correct type */ \
                return invoke(v, x, std::forward<Args>(args)...);           \
              }

#define BOOST_PP_LOCAL_LIMITS (0, UNROLL_SIZE)

#include BOOST_PP_LOCAL_ITERATE()
    }

    // fallback: loop on the next unroll_size types
    return dispatch<Index + UNROLL_SIZE + 1>()
                   (v, storage, tuple, index - UNROLL_SIZE - 1, std::forward<Args>(args)...);

#undef UNROLL_SIZE
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Invokation out-of-bounds case.
///
/// Used by dispatch* to narrow down the possible invocations of visitor only to the types
/// held by the visited variant. It thus avoid to apply the visitor on the nil type which is
/// returned by util::nth when the provided index does not match any held type. It is
/// necessary as the preprocessor will generate instantations of such invocations.
template <typename Visitor, typename T, typename U, typename... Args>
inline
typename enable_if< is_same<T, nil>::value or is_same<U, nil>::value
                  , typename Visitor::result_type>::type
binary_invoke(const Visitor&, const T&, const U&, Args&&...)
noexcept
{
  assert(false);
  __builtin_unreachable();
}

/// @internal
/// @brief Invoke user's visitor for deduced type.
template <typename Visitor, typename T, typename U, typename... Args>
inline
typename enable_if< not (is_same<T, nil>::value or is_same<U, nil>::value)
                  , typename Visitor::result_type>::type
binary_invoke(const Visitor& v, const T& x, const U& y, Args&&... args)
noexcept(noexcept(v(x, y, std::forward<Args>(args)...)))
{
  return v(x, y, std::forward<Args>(args)...);
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <uint8_t Index>
struct inner_dispatch
{

  /// Instanciated for any index generated by the preprocessor that is greater than the number
  /// of contained types in the variant, thus avoiding compilation error.
  template < typename Visitor, typename T, typename Storage
           , typename... Types, template <typename...> class Tuple
           , typename... Args>
  typename enable_if<Index >= sizeof...(Types), typename Visitor::result_type>::type
  operator()( const Visitor&, const T&
            , const Storage&, const Tuple<Types...>&, const uint8_t
            , Args&&...)
  noexcept
  {
    assert(false);
    __builtin_unreachable();
  }

  template < typename Visitor, typename T, typename Storage
           , typename... Types, template <typename...> class Tuple
           , typename... Args>
  typename enable_if<Index < sizeof...(Types), typename Visitor::result_type>::type
  operator()( const Visitor& v
            , const T& x
            , const Storage& storage, const Tuple<Types...>& tuple, const uint8_t index
            , Args&&... args)
  {
#ifndef SDD_VARIANT_UNROLL_BINARY_SIZE
#  define UNROLL_SIZE 31
#else
#  define UNROLL_SIZE SDD_VARIANT_UNROLL_BINARY_SIZE
#endif

    switch (index)
    {
#define BOOST_PP_LOCAL_MACRO(n)                                             \
      case n: {                                                             \
                typedef typename util::nth<Index + n, Types...>::type U;    \
                const U& y = *reinterpret_cast<const U*>(&storage);         \
                return binary_invoke(v, x, y, std::forward<Args>(args)...); \
              }

#define BOOST_PP_LOCAL_LIMITS (0, UNROLL_SIZE)

#include BOOST_PP_LOCAL_ITERATE()
    }

    // Fallback: loop on the next unroll_size types
    return inner_dispatch<Index + UNROLL_SIZE + 1>()
                         ( v, x, storage, tuple, index - UNROLL_SIZE - 1
                         , std::forward<Args>(args)...);
#undef UNROLL_SIZE
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <uint8_t Index1>
struct dispatch_binary
{
  /// Instanciated for any index generated by the preprocessor that is greater than the number
  /// of contained types in the variant, thus avoiding compilation error.
  template < typename Visitor, typename Storage1, typename Storage2
           , typename... Types1, template <typename...> class Tuple1
           , typename... Types2, template <typename...> class Tuple2
           , typename... Args>
  typename enable_if<Index1 >= sizeof...(Types1), typename Visitor::result_type>::type
  operator()( const Visitor&
            , const Storage1&, const Tuple1<Types1...>&, const uint8_t
            , const Storage2&, const Tuple2<Types2...>&, const uint8_t
            , Args&&...)
  noexcept
  {
    assert(false);
    __builtin_unreachable();
  }

  template < typename Visitor, typename Storage1, typename Storage2
           , typename... Types1, template <typename...> class Tuple1
           , typename... Types2, template <typename...> class Tuple2
           , typename... Args>
  typename enable_if<Index1 < sizeof...(Types1), typename Visitor::result_type>::type
  operator()( const Visitor& v
            , const Storage1& storage1, const Tuple1<Types1...>& tuple1, const uint8_t index1
            , const Storage2& storage2, const Tuple2<Types2...>& tuple2, const uint8_t index2
            , Args&&... args)
  {
#ifndef SDD_VARIANT_UNROLL_BINARY_SIZE
#  define UNROLL_SIZE 31
#else
#  define UNROLL_SIZE SDD_VARIANT_UNROLL_BINARY_SIZE
#endif

    switch (index1)
    {
#define BOOST_PP_LOCAL_MACRO(n)                                                      \
      case n: {                                                                      \
                typedef typename util::nth<Index1 + n, Types1...>::type T1;          \
                const T1& x = *reinterpret_cast<const T1*>(&storage1);               \
                /* type of the first visitable found, now look for the second one */ \
                return inner_dispatch<0>()                                           \
                                        ( v, x                                       \
                                        , storage2, tuple2, index2                   \
                                        , std::forward<Args>(args)...);              \
              }

#define BOOST_PP_LOCAL_LIMITS (0, UNROLL_SIZE)

#include BOOST_PP_LOCAL_ITERATE()
    }

  // Fallback: loop on the next unroll_size types
  return dispatch_binary<Index1 + UNROLL_SIZE + 1>()
                        ( v
                        , storage1, tuple1, index1 - UNROLL_SIZE - 1
                        , storage2, tuple2, index2
                        , std::forward<Args>(args)...);
#undef UNROLL_SIZE
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace sdd::internal::mem

#endif // _SDD_INTERNAL_MEM_VARIANT_IMPL_HH_
