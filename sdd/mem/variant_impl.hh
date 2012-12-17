#ifndef _SDD_MEM_VARIANT_IMPL_HH_
#define _SDD_MEM_VARIANT_IMPL_HH_

#include <cassert>
#include <cstdint>     // uint8_t
#include <functional>  // hash
#include <iosfwd>
#include <type_traits> // enable_if, forward, is_same, aligned_storage, alignment_of
#include <utility>     // forward

#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/cat.hpp>

#include "sdd/util/typelist.hh"

namespace sdd { namespace mem {

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
/// @brief Returns the number of extra bytes that the actual type contained by the variant
/// may have.
///
/// It is used by the unicity table to help it manage memory.
struct extra_bytes_visitor
{
  typedef std::size_t result_type;

  template <typename T>
  std::size_t
  operator()(const T& x)
  const noexcept
  {
    // Static dispatch.
    return impl(x, 0);
  }

  /// brief Called when the actual type defines extra_bytes.
  template <typename U>
  static auto
  impl(const U& x, int)
  noexcept
  -> decltype(x.extra_bytes())
  {
    return x.extra_bytes();
  }

  /// brief Called when the actual type doesn't define extra_bytes.
  template <typename U>
  static constexpr auto
  impl(const U&, long)
  noexcept
  -> decltype(0)
  {
    return 0;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Out of bounds case.
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
/// @brief Dispatch of the visitor on the held type.
template < typename Visitor
         , typename Storage, typename... Types, template <typename...> class Tuple
         , typename... Args
         >
typename Visitor::result_type
dispatch( const Visitor& v
        , const Storage& storage, const Tuple<Types...>&, const uint8_t index
        , Args&&... args)
{
  void* table[LIBSDD_VARIANT_SIZE] = {BOOST_PP_ENUM_PARAMS(LIBSDD_VARIANT_SIZE, &&lab)};
  goto *table[index];

# define BOOST_PP_LOCAL_MACRO(n)                        \
  BOOST_PP_CAT(lab,n):                                  \
  {                                                     \
    typedef typename util::nth<n, Types...>::type T;    \
    const T& x = *reinterpret_cast<const T*>(&storage); \
    return invoke(v, x, std::forward<Args>(args)...);   \
  }                                                     \

# define BOOST_PP_LOCAL_LIMITS (0, LIBSDD_VARIANT_SIZE - 1)
# include BOOST_PP_LOCAL_ITERATE()

  assert(false);
  __builtin_unreachable();
}

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
/// @brief Dispatch a binary visitor on the second visitable.
template < typename Visitor, typename T, typename Storage
         , typename... Types, template <typename...> class Tuple
         , typename... Args>
typename Visitor::result_type
inner_dispatch( const Visitor& v
              , const T& x
              , const Storage& storage, const Tuple<Types...>&, const uint8_t index
              , Args&&... args)
{
  void* table[LIBSDD_VARIANT_SIZE] = {BOOST_PP_ENUM_PARAMS(LIBSDD_VARIANT_SIZE, &&lab)};
  goto *table[index];

# define BOOST_PP_LOCAL_MACRO(n)                                \
  BOOST_PP_CAT(lab,n):                                          \
  {                                                             \
    typedef typename util::nth<n, Types...>::type U;            \
    const U& y = *reinterpret_cast<const U*>(&storage);         \
    return binary_invoke(v, x, y, std::forward<Args>(args)...); \
  }                                                             \

# define BOOST_PP_LOCAL_LIMITS (0, LIBSDD_VARIANT_SIZE - 1)
# include BOOST_PP_LOCAL_ITERATE()

  assert(false);
  __builtin_unreachable();
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Dispatch a binary visitor on the first visitable.
template < typename Visitor, typename Storage1, typename Storage2
         , typename... Types1, template <typename...> class Tuple1
         , typename... Types2, template <typename...> class Tuple2
         , typename... Args>
typename Visitor::result_type
binary_dispatch( const Visitor& v
               , const Storage1& storage1, const Tuple1<Types1...>&, const uint8_t index1
               , const Storage2& storage2, const Tuple2<Types2...>& tuple2, const uint8_t index2
               , Args&&... args)
{
  void* table[LIBSDD_VARIANT_SIZE] = {BOOST_PP_ENUM_PARAMS(LIBSDD_VARIANT_SIZE, &&lab)};
  goto *table[index1];

# define BOOST_PP_LOCAL_MACRO(n)                                                        \
  BOOST_PP_CAT(lab,n):                                                                  \
  {                                                                                     \
    typedef typename util::nth<n, Types1...>::type T;                                   \
    const T& x = *reinterpret_cast<const T*>(&storage1);                                \
    return inner_dispatch(v, x, storage2, tuple2, index2, std::forward<Args>(args)...); \
  }                                                                                     \

# define BOOST_PP_LOCAL_LIMITS (0, LIBSDD_VARIANT_SIZE - 1)
# include BOOST_PP_LOCAL_ITERATE()

  assert(false);
  __builtin_unreachable();
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem

#endif // _SDD_MEM_VARIANT_IMPL_HH_
