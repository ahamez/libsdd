#ifndef _SDD_INTERNAL_MEM_VARIANT_IMPL_HH_
#define _SDD_INTERNAL_MEM_VARIANT_IMPL_HH_

/// @cond INTERNAL_DOC

#include <cassert>
#include <cstdint>     // uint8_t
#include <functional>  // hash
#include <iosfwd>
#include <type_traits> // enable_if, forward, is_same, aligned_storage, alignment_of

#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/cat.hpp>

#include "sdd/internal/util/typelist.hh"

namespace sdd { namespace internal { namespace mem {

/*-------------------------------------------------------------------------------------------*/

using std::enable_if;
using std::is_same;
using util::nil;
using util::nb_types;

/*-------------------------------------------------------------------------------------------*/

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

/*-------------------------------------------------------------------------------------------*/

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

/*-------------------------------------------------------------------------------------------*/

/// @brief Provide our own version of std::aligned_union while it's not done by gcc and clang.
template <std::size_t Len, typename... Types>
struct aligned_union
{
  static constexpr std::size_t max_size = largest_size<Types...>::value;
  static constexpr std::size_t alignment_value = largest_alignment<Types...>::value;
  typedef typename std::aligned_storage< (Len > max_size ? Len : max_size)
                                       , alignment_value
                                       >::type
                   type;
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Dispatch the desctructor to the contained type in the visited variant.
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

/*-------------------------------------------------------------------------------------------*/

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

/*-------------------------------------------------------------------------------------------*/

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

/*-------------------------------------------------------------------------------------------*/

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

/*-------------------------------------------------------------------------------------------*/

/// Used by dispatch* to narrow down the possible invocations of visitor only to the types
/// held by the visited variant. It thus avoid to apply the visitor on the nil type which is
/// returned by util::nth when the provided index does not match any held type. It is
/// necessary as the preprocessor will generate instantations of such invocations.
template <typename Visitor, typename T>
inline
typename enable_if<is_same<T, nil>::value, typename Visitor::result_type>::type
invoke(const Visitor&, const T&)
noexcept
{
  assert(false);
  __builtin_unreachable();
}

template <typename Visitor, typename T>
inline
typename enable_if<not is_same<T, nil>::value, typename Visitor::result_type>::type
invoke(const Visitor& v, const T& x)
noexcept(noexcept(v(x)))
{
  return v(x);
}

/*-------------------------------------------------------------------------------------------*/

/// Instanciated for any index generated by the preprocessor that is greater than the number
/// of contained types in the variant, thus avoiding compilation error.
template <typename Visitor, typename Storage, uint8_t Index, typename... Types>
inline
typename enable_if< (Index >= util::nb_types<Types...>::value)
                  , typename Visitor::result_type>::type
dispatch(const Visitor&, const Storage&, const uint8_t)
noexcept
{
  assert(false);
  __builtin_unreachable();
}

/// Dispatch of the visitor on the held type.
template <typename Visitor, typename Storage, uint8_t Index, typename... Types>
typename enable_if< (Index < util::nb_types<Types...>::value)
                  , typename Visitor::result_type>::type
dispatch(const Visitor& v, const Storage& storage, const uint8_t index)
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
#define BOOST_PP_LOCAL_MACRO(n)                                           \
    case n: {                                                             \
              /* first, find the held type */                             \
              typedef typename util::nth<Index + n, Types...>::type T;    \
              /* then, see the stored data as this type */                \
              const T& x = *reinterpret_cast<const T*>(&storage);         \
              /* finally, we can apply the visitor on the correct type */ \
              return invoke(v, x);                                        \
            }

#define BOOST_PP_LOCAL_LIMITS (0, UNROLL_SIZE)

#include BOOST_PP_LOCAL_ITERATE()
  }

  // fallback: loop on the next unroll_size types
  return dispatch<Visitor, Storage, Index + UNROLL_SIZE + 1, Types...>
                 (v, storage, index - UNROLL_SIZE - 1);

#undef UNROLL_SIZE
}

/*-------------------------------------------------------------------------------------------*/

/// Used by dispatch* to narrow down the possible invocations of visitor only to the types
/// held by the visited variant. It thus avoid to apply the visitor on the nil type which is
/// returned by util::nth when the provided index does not match any held type. It is
/// necessary as the preprocessor will generate instantations of such invocations.
template <typename Visitor, typename T, typename U>
inline
typename enable_if< is_same<T, nil>::value or is_same<U, nil>::value
                  , typename Visitor::result_type>::type
binary_invoke(const Visitor&, const T&, const U&)
noexcept
{
  assert(false);
  __builtin_unreachable();
}

template <typename Visitor, typename T, typename U>
inline
typename enable_if< not (is_same<T, nil>::value or is_same<U, nil>::value)
                  , typename Visitor::result_type>::type
binary_invoke(const Visitor& v, const T& x, const U& y)
noexcept(noexcept(v(x, y)))
{
  return v(x, y);
}

/*-------------------------------------------------------------------------------------------*/

/// Instanciated for any index generated by the preprocessor that is greater than the number
/// of contained types in the variant, thus avoiding compilation error.
template <typename Visitor, typename T1, typename Storage, uint8_t Index2, typename... Types>
typename enable_if< (Index2 >= util::nb_types<Types...>::value)
                  , typename Visitor::result_type>::type
inner_dispatch_binary(const Visitor&, const T1&, const Storage&, const uint8_t)
noexcept
{
  assert(false);
  __builtin_unreachable();
}

template <typename Visitor, typename T1, typename Storage, uint8_t Index2, typename... Types>
typename enable_if< (Index2 < util::nb_types<Types...>::value)
                  , typename Visitor::result_type>::type
inner_dispatch_binary( const Visitor& v
                     , const T1& x
                     , const Storage& storage_2, const uint8_t index_2)
{
#ifndef SDD_VARIANT_UNROLL_BINARY_SIZE
#  define UNROLL_SIZE 31
#else
#  define UNROLL_SIZE SDD_VARIANT_UNROLL_BINARY_SIZE
#endif

  switch (index_2)
  {
#define BOOST_PP_LOCAL_MACRO(n)                                         \
    case n: {                                                           \
              typedef typename util::nth<Index2 + n, Types...>::type U; \
              const U& y = *reinterpret_cast<const U*>(&storage_2);     \
              return binary_invoke(v, x, y);                            \
            }

#define BOOST_PP_LOCAL_LIMITS (0, UNROLL_SIZE)

#include BOOST_PP_LOCAL_ITERATE()
    }

    // Fallback: loop on the next unroll_size types
    return inner_dispatch_binary<Visitor, T1, Storage, Index2 + UNROLL_SIZE + 1, Types...>
                                (v, x, storage_2, index_2 - UNROLL_SIZE - 1);
#undef UNROLL_SIZE
}

/*-------------------------------------------------------------------------------------------*/

/// Instanciated for any index generated by the preprocessor that is greater than the number
/// of contained types in the variant, thus avoiding compilation error.
template <typename Visitor, typename Storage, uint8_t Index1, typename... Types>
typename enable_if< (Index1 >= util::nb_types<Types...>::value)
                  , typename Visitor::result_type>::type
dispatch_binary(const Visitor&, const Storage&, const uint8_t, const Storage&, const uint8_t)
noexcept
{
  assert(false);
  __builtin_unreachable();
}

template <typename Visitor, typename Storage, uint8_t Index1, typename... Types>
typename enable_if< (Index1 < util::nb_types<Types...>::value)
                  , typename Visitor::result_type>::type
dispatch_binary( const Visitor& v
               , const Storage& storage_1, const uint8_t index_1
               , const Storage& storage_2, const uint8_t index_2)
{
#ifndef SDD_VARIANT_UNROLL_BINARY_SIZE
#  define UNROLL_SIZE 31
#else
#  define UNROLL_SIZE SDD_VARIANT_UNROLL_BINARY_SIZE
#endif

  switch (index_1)
  {
#define BOOST_PP_LOCAL_MACRO(n)                                                    \
    case n: {                                                                      \
              typedef typename util::nth<Index1 + n, Types...>::type T1;           \
              const T1& x = *reinterpret_cast<const T1*>(&storage_1);              \
              /* type of the first visitable found, now look for the second one */ \
              return inner_dispatch_binary<Visitor, T1, Storage, 0, Types...>      \
                                          (v, x, storage_2, index_2);              \
            }

#define BOOST_PP_LOCAL_LIMITS (0, UNROLL_SIZE)

#include BOOST_PP_LOCAL_ITERATE()
  }

  // Fallback: loop on the next unroll_size types
  return dispatch_binary<Visitor, Storage, Index1 + UNROLL_SIZE + 1, Types...>
                        (v, storage_1, index_1 - UNROLL_SIZE - 1, storage_2, index_2);
#undef UNROLL_SIZE
}

/*-------------------------------------------------------------------------------------------*/

}}} // namespace sdd::internal::mem

/// @endcond

#endif // _SDD_INTERNAL_MEM_VARIANT_IMPL_HH_
