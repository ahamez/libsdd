#ifndef _SDD_ORDER_ORDER_IDENTIFIER_HH_
#define _SDD_ORDER_ORDER_IDENTIFIER_HH_

#include "sdd/util/hash.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief Store a user's identifier or an artificial one.
/// @todo Use a discriminated union.
///
/// An artificial identifier is created by order strategies (automatic variable ordering).
/// Furthermore, only identifiers that represent hierarchical levels can be artificial.
template <typename C>
struct order_identifier
{
public:

  /// @brief The of user's identifier.
  using identifier_type = typename C::Identifier;

private:

  /// @brief The value of this identifier, if artificial.
  ///
  /// If it's equals to 0, then this node identifier is not an artificial one and
  /// it's safe to call user().
  unsigned int artificial_;

  /// @brief The value of this identifier, if it's a user one.
  identifier_type user_;

public:

  /// @internal
  /// @brief Construct an artificial node with a given index.
  /// @param Must be unique for a given artificial identifier.
  order_identifier()
    : artificial_(next_artificial()), user_()
  {}

  /// @internal
  /// @brief Construct with a user's identifier.
  order_identifier(const identifier_type& id)
    : artificial_(0), user_(id)
  {}

  /// @brief Get the identifier as an artificial one.
  unsigned int
  artificial()
  const noexcept
  {
    return artificial_;
  }

  /// @brief Tell if it's an artificial identifier.
  bool
  is_artificial()
  const noexcept
  {
    return artificial_ != 0;
  }

  /// @brief Get the identifier as a user one.
  const identifier_type&
  user()
  const noexcept
  {
    return user_;
  }

private:

  /// @brief Get the next valid unique counter for artificial identifiers.
  static
  unsigned int
  next_artificial()
  noexcept
  {
    static unsigned int counter = 0;
    return ++counter;
  }
};

/// @internal
/// @related order_identifier
template <typename C>
bool
operator==(const order_identifier<C>& lhs, const order_identifier<C>& rhs)
{
  return lhs.artificial() == rhs.artificial() and lhs.user() == rhs.user();
}

/// @internal
/// @brief Output an order_identifier to an ostream.
/// @related order_identifier.
template <typename C>
std::ostream&
operator<<(std::ostream& os, const order_identifier<C>& id)
{
  if (id.is_artificial())
  {
    return os << "@" << id.artificial() << "@";
  }
  else
  {
    return os << id.user();
  }
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::order_identifier.
template <typename C>
struct hash<sdd::order_identifier<C>>
{
  std::size_t
  operator()(const sdd::order_identifier<C>& o)
  const noexcept
  {
    std::size_t seed = sdd::util::hash(o.artificial());
    sdd::util::hash_combine(seed, o.user());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_ORDER_ORDER_IDENTIFIER_HH_


