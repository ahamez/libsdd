#ifndef _SDD_MEM_VARIANT_HH_
#define _SDD_MEM_VARIANT_HH_

#ifndef LIBSDD_VARIANT_SIZE
#define LIBSDD_VARIANT_SIZE 14
#endif

#include <cstdint>     // uint8_t
#include <functional>  // hash
#include <iosfwd>
#include <limits>      // numeric_limits
#include <type_traits> // is_nothrow_constructible
#include <utility>     // forward

#include "sdd/mem/variant_impl.hh"
#include "sdd/util/hash.hh"
#include "sdd/util/packed.hh"
#include "sdd/util/typelist.hh"
#include "sdd/util/print_sizes_fwd.hh"

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @page Variant
///
/// The sdd::mem::variant class is at the heart of the library. Indeed, sdd::SDD and
/// sdd::homomorphism definitions rely on it.
///
/// Its purpose is to emulate a union, but with much more possibilities. It's completely inspired
/// from boost::variant.
/// http://www.boost.org/doc/libs/release/doc/html/variant.html
///
/// The visit mechanism uses 'computed goto', a GCC extension, also supported by clang.
/// Note that a 'switch' statement will also do the job.
/// http://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Labels-as-Values.html

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Helper struct to determine the type to be constructed in place.
template <typename T>
struct construct {};

/*------------------------------------------------------------------------------------------------*/

// Forward declaration for destructor.
template <typename Visitor, typename Variant, typename... Args>
typename Visitor::result_type
apply_visitor(const Visitor&, const Variant&, Args&&...);

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief  A union-like structure.
/// @tparam Types The list of possible types.
///
/// This type is meant to be stored in a unique_table, wrapped in a ref_counted.
/// Once constructed, it can never be assigned an other data.
template <typename... Types>
class LIBSDD_ATTRIBUTE_PACKED variant
{
  // Can't copy a variant.
  const variant& operator=(const variant&) = delete;
  variant(const variant&) = delete;  

  // Can't move a variant.
  variant& operator=(variant&&) = delete;
  variant(variant&&) = delete;

private:

  template <typename C>
  friend void util::print_sizes(std::ostream&);

  static_assert( sizeof...(Types) >= 1
               , "A variant should contain at least one type.");

  static_assert( sizeof...(Types) <= std::numeric_limits<unsigned char>::max()
               , "A variant can't hold more than UCHAR_MAX types.");

  static_assert( sizeof...(Types) <= LIBSDD_VARIANT_SIZE
               , "LIBSDD_VARIANT_SIZE is too small for the required number of types.");

  /// @brief Index of the held type in the list of all possible types.
  const uint8_t index_;

  /// @brief A type large enough to contain all variant's types, with the correct alignement.
  typedef typename union_storage<0, Types...>::type storage_type;

  /// @brief Memory storage suitable for all Types.
  const storage_type storage_;

public:

  /// @brief In place construction of an held type.
  ///
  /// Unlike boost::variant, we don't need the never empty guaranty, so we don't need to check
  /// if held types can throw exceptions upon construction.
  template <typename T, typename... Args>
  variant(construct<T>, Args&&... args)
  noexcept(std::is_nothrow_constructible<T, Args...>::value)
    : index_(util::index_of<const T, const Types...>::value)
  	, storage_()
  {
    new (const_cast<storage_type*>(&storage_)) T(std::forward<Args>(args)...);
  }

  /// @brief Destructor.
  ~variant()
  {
    apply_visitor(dtor_visitor(), *this);
  }

  /// @brief Get as type T.
  ///
  /// No verifications are done.
  template <typename T>
  const T&
  get()
  const noexcept
  {
    return *reinterpret_cast<const T*>(&storage_);
  }

  /// @brief Return the position of the currently held type in the list of all possible types.
  uint8_t
  index()
  const noexcept
  {
    return index_;
  }

  /// @brief Return the raw storage.
  const storage_type&
  storage()
  const noexcept
  {
    return storage_;
  }

  /// @brief Get the number of extra bytes that can be used by the actual type.
  std::size_t
  extra_bytes()
  const noexcept
  {
    return apply_visitor(extra_bytes_visitor(), *this);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related variant
template < typename Visitor
         , typename... Types, template <typename...> class Variant
         , typename... Args>
inline
typename Visitor::result_type
apply_visitor(const Visitor& v, const Variant<Types...>& x, Args&&... args)
{
  return dispatch( v
                 , x.storage(), util::typelist<Types...>(), x.index()
                 , std::forward<Args>(args)...);
}

/// @internal
/// @related variant
template < typename Visitor
         , typename... Types1, template <typename...> class Variant1
         , typename... Types2, template <typename...> class Variant2
         , typename... Args>
inline
typename Visitor::result_type
apply_binary_visitor( const Visitor& v
                    , const Variant1<Types1...>& x, const Variant2<Types2...>& y
                    , Args&&... args)
{
  return binary_dispatch( v
                        , x.storage(), util::typelist<Types1...>(), x.index()
                        , y.storage(), util::typelist<Types2...>(), y.index()
                        , std::forward<Args>(args)...);
}

/// @internal
/// @related variant
template <typename... Types>
inline
bool
operator==(const variant<Types...>& lhs, const variant<Types...>& rhs)
noexcept
{
  return lhs.index() == rhs.index() and apply_binary_visitor(eq_visitor(), lhs, rhs);
}

/// @internal
/// @related variant
template <typename T, typename... Types>
inline
const T&
variant_cast(const variant<Types...>& v)
noexcept
{
  return v.template get<T>();
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related variant
template <typename... Types>
std::ostream&
operator<<(std::ostream& os, const variant<Types...>& v)
{
  return apply_visitor(ostream_visitor(os), v);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::mem

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::mem::variant
template <typename... Types>
struct hash<sdd::mem::variant<Types...>>
{
  std::size_t
  operator()(const sdd::mem::variant<Types...>& x)
  const noexcept
  {
    std::size_t seed = sdd::mem::apply_visitor(sdd::mem::hash_visitor(), x);
    sdd::util::hash_combine(seed, x.index());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_INTERNAL_MEM_VARIANT_HH_
