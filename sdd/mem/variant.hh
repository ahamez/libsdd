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
///
/// Its purpose is to emulate a union, but with much more possibilities. It's completely inspired
/// from boost::variant: http://www.boost.org/doc/libs/release/doc/html/variant.html
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

  static_assert( sizeof...(Types) <= std::numeric_limits<unsigned char>::max()
               , "A variant can't hold more than UCHAR_MAX types.");

  /// @brief Index of the held type in the list of all possible types.
  const uint8_t index;

  /// @brief A type large enough to contain all variant's types, with the correct alignement.
  using storage_type = typename union_storage<0, Types...>::type;

private:

  /// @brief Memory storage suitable for all Types.
  storage_type storage_;

public:

  /// @brief In place construction of an held type.
  ///
  /// Unlike boost::variant, we don't need the never empty guaranty, so we don't need to check
  /// if held types can throw exceptions upon construction.
  template <typename T, typename... Args>
  variant(construct<T>, Args&&... args)
  noexcept(std::is_nothrow_constructible<T, Args...>::value)
    : index(util::index_of<const T, const Types...>::value)
  	, storage_()
  {
    new (&storage_) T(std::forward<Args>(args)...);
  }

  /// @brief Destructor.
  ~variant()
  {
    apply_visitor(dtor_visitor(), *this);
  }

  /// @brief Return the index for a type contained in Types
  template <typename T>
  static constexpr
  std::size_t
  index_for_type()
  noexcept
  {
    return util::index_of<T, Types...>::value;
  }

  const storage_type*
  storage()
  const noexcept
  {
    return &storage_;
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
const T&
variant_cast(const variant<Types...>& v)
noexcept
{
  return *reinterpret_cast<const T*>(v.storage());
}

/// @internal
/// @related variant
template <typename T, typename... Types>
inline
auto
is(const variant<Types...>& v)
noexcept
{
  return v.index == util::index_of<T, Types...>::value;
}

/// @internal
/// @related variant
template <typename T, typename VariantProxy>
inline
auto
is(const VariantProxy& v)
noexcept
{
  return is<T>(*v);
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
-> decltype(auto)
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
-> decltype(auto)
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
