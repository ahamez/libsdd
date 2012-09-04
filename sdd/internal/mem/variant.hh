#ifndef _SDD_INTERNAL_MEM_VARIANT_HH_
#define _SDD_INTERNAL_MEM_VARIANT_HH_

/// @cond INTERNAL_DOC

#include <climits>     // USHRT_MAX
#include <cstdint>     // uint8_t
#include <functional>  // hash
#include <iosfwd>
#include <type_traits> // is_nothrow_constructible
#include <utility>     // forward

#include "sdd/internal/mem/variant_impl.hh"
#include "sdd/internal/util/hash.hh"
#include "sdd/internal/util/packed.hh"
#include "sdd/internal/util/typelist.hh"

namespace sdd { namespace internal { namespace mem {

/*-------------------------------------------------------------------------------------------*/

/// @brief Helper struct to determine the type to be constructed in place.
template <typename T>
struct construct {};

/*-------------------------------------------------------------------------------------------*/

/// @brief  A union-like structure.
/// @tparam Types The list of possible types.
///
/// This type is meant to be stored in a unique_table, wrapped in a ref_counted.
/// Once constructed, it can never be assigned an other data.
template <typename... Types>
class _LIBSDD_ATTRIBUTE_PACKED variant
{
  // Can't copy a variant.
  const variant& operator=(const variant&) = delete;
  variant(const variant&) = delete;  

  // Can't move a variant.
  variant& operator=(variant&&) = delete;
  variant(variant&&) = delete;

private:

  static_assert( util::nb_types<Types...>::value >= 1
               , "A variant should contain at least one type.");

  static_assert( util::nb_types<Types...>::value <= UCHAR_MAX
               , "A variant can't hold more than UCHAR_MAX types.");

  /// @brief Index of the held type in the list of all possible types.
  const uint8_t index_;

  /// @brief A type large enough to contain all variant's types, with the correct alignement.
  typedef typename aligned_union<0, Types...>::type storage_type;

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

  ~variant()
  {
    accept(dtor_visitor());
  }

  /// @brief Get as type T.
  template <typename T>
  const T&
  get()
  const noexcept
  {
    return *reinterpret_cast<const T*>(&storage_);
  }

  /// @brief Accept one visitor which will be dispatched on the currently held type.
  template <typename Visitor>
  typename Visitor::result_type
  accept(const Visitor& v)
  const
  {
    return dispatch<Visitor, storage_type, 0, Types...>
                   (v, storage_, index_);
  }


  /// @brief Accept one visitor which will be dispatched on the currently held type by this
  /// variant and the other visited variant.
  template <typename Visitor>
  typename Visitor::result_type
  accept(const variant& other, const Visitor& v)
  const
  {
    return dispatch_binary<Visitor, storage_type, 0, Types...>
                          (v, storage_, index_, other.storage_, other.index_);
  }

  /// @brief Return the position of the currently held type in the list of all possible types.
  uint8_t
  index()
  const noexcept
  {
    return index_;
  }
};

/*-------------------------------------------------------------------------------------------*/

/// @related variant
template <typename... Types>
inline
bool
operator==(const variant<Types...>& lhs, const variant<Types...>& rhs)
noexcept
{
  return lhs.index() == rhs.index() and lhs.accept(rhs, eq_visitor());
}

/// @related variant
template <typename T, typename... Types>
inline
const T&
variant_cast(const variant<Types...>& v)
noexcept
{
  return v.template get<T>();
}

/// @related variant
template <typename Visitor, typename Variant>
inline
typename Visitor::result_type
apply_visitor(const Visitor& v, const Variant& x)
{
  return x.accept(v);
}

/// @related variant
template <typename Visitor, typename Variant>
inline
typename Visitor::result_type
apply_visitor(const Visitor& v, const Variant& x, const Variant& y)
{
  return x.accept(y, v);
}

/*-------------------------------------------------------------------------------------------*/

/// @related variant
template <typename... Types>
std::ostream&
operator<<(std::ostream& os, const variant<Types...>& v)
{
  return apply_visitor(ostream_visitor(os), v);
}

/*-------------------------------------------------------------------------------------------*/

}}} // namespace sdd::internal::mem

namespace std {

/*-------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::internal::mem::variant
template <typename... Types>
struct hash<const sdd::internal::mem::variant<Types...>>
{
  std::size_t
  operator()(const sdd::internal::mem::variant<Types...>& x)
  const noexcept
  {
    std::size_t seed =
      sdd::internal::mem::apply_visitor(sdd::internal::mem::hash_visitor(), x);
    sdd::internal::util::hash_combine(seed, x.index());
    return seed;
  }
};

/*-------------------------------------------------------------------------------------------*/

} // namespace std

/// @endcond

#endif // _SDD_INTERNAL_MEM_VARIANT_HH_
