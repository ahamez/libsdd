/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

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

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Helper struct to determine the type to be constructed in place.
template <typename T>
struct construct {};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief A type-safe discriminated union.
/// @tparam Types The list of possible types.
///
/// This type is meant to be stored in a unique_table, wrapped in a ref_counted.
/// Once constructed, it can never be assigned an other data.
/// It is at the heart of the library: sdd::SDD and sdd::homomorphism definitions rely on it.
template <typename... Types>
struct LIBSDD_ATTRIBUTE_PACKED variant
{
  // Can't copy a variant.
  const variant& operator=(const variant&) = delete;
  variant(const variant&) = delete;  

  // Can't move a variant.
  variant& operator=(variant&&) = delete;
  variant(variant&&) = delete;

  static_assert( sizeof...(Types) >= 1
               , "A variant should contain at least one type.");

  static_assert( sizeof...(Types) <= std::numeric_limits<uint8_t>::max()
               , "A variant can't hold more than UCHAR_MAX types.");

  /// @brief Index of the held type in the list of all possible types.
  const uint8_t index;

  /// @brief A type large enough to contain all variant's types, with the correct alignement.
  using storage_type = typename union_storage<0, Types...>::type;

  /// @brief Memory storage suitable for all Types.
  const storage_type storage;

  /// @brief In place construction of an held type.
  ///
  /// Unlike boost::variant, we don't need the never empty guaranty, so we don't need to check
  /// if held types can throw exceptions upon construction.
  template <typename T, typename... Args>
  variant(construct<T>, Args&&... args)
  noexcept(std::is_nothrow_constructible<T, Args...>::value)
    : index(util::index_of<T, Types...>::value)
  	, storage()
  {
    new (const_cast<storage_type*>(&storage)) T{std::forward<Args>(args)...};
  }

  /// @brief Destructor.
  ~variant()
  {
    apply_visitor(dtor_visitor(), *this);
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const variant& v)
  {
    return apply_visitor([&](const auto& x) -> std::ostream& {return os << x;}, v);
  }

  friend
  bool
  operator==(const variant& lhs, const variant& rhs)
  noexcept
  {
    return lhs.index == rhs.index and apply_binary_visitor(eq_visitor(), lhs, rhs);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related variant
template <typename T, typename... Types>
inline
bool
is(const variant<Types...>& v)
noexcept
{
  return v.index == util::index_of<typename std::decay<T>::type, Types...>::value;
}

/// @internal
/// @related variant
template <typename T, typename VariantProxy>
inline
bool
is(const VariantProxy& v)
noexcept
{
  return is<T>(*v);
}

/// @internal
/// @related variant
template <typename T, typename... Types>
inline
const T&
variant_cast(const variant<Types...>& v)
noexcept
{
  assert(is<T>(v));
  return *reinterpret_cast<const T*>(&v.storage);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace mem

/*------------------------------------------------------------------------------------------------*/

/// @brief
/// @related mem::variant
template <typename Visitor, typename X, typename... Args>
inline
auto
visit(Visitor&& v, const X& x, Args&&... args)
-> decltype(apply_visitor(std::forward<Visitor>(v), *x, std::forward<Args>(args)...))
{
  return apply_visitor(std::forward<Visitor>(v), *x, std::forward<Args>(args)...);
}

/*------------------------------------------------------------------------------------------------*/

/// @brief
/// @related mem::variant
template <typename Visitor, typename X, typename Y, typename... Args>
inline
auto
binary_visit(Visitor&& v, const X& x, const Y& y, Args&&... args)
-> decltype(apply_binary_visitor(std::forward<Visitor>(v), *x, *y, std::forward<Args>(args)...))
{
  return apply_binary_visitor(std::forward<Visitor>(v), *x, *y, std::forward<Args>(args)...);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::mem::variant
template <typename... Types>
struct hash<sdd::mem::variant<Types...>>
{
  std::size_t
  operator()(const sdd::mem::variant<Types...>& x)
  const
  {
    using namespace sdd::hash;
    return seed(apply_visitor(sdd::mem::hash_visitor{}, x)) (val(x.index));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
