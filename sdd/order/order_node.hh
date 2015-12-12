/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <memory>    // shared_ptr
#include <vector>

#include "sdd/order/order_builder.hh"
#include "sdd/order/order_identifier.hh"
#include "sdd/util/hash.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The position of an order's node (from top to bottom), unique to it.
using order_position_type = unsigned int;

/*------------------------------------------------------------------------------------------------*/

/// @brief A node in an order: an identifier associated to an SDD variable.
template <typename C>
class order_node
{
public:

  /// @brief A library's variable type.
  using variable_type = typename C::variable_type;

  /// @brief A path, following hierarchies, to a node.
  using path_type = std::vector<order_position_type>;

private:

  /// @brief The (user or artificial) identifier of this node.
  order_identifier<C> identifier_;

  /// @brief The (library's) variable of this node.
  variable_type variable_;

  /// @brief Absolute position, when seeing the order as flatten.
  ///
  /// Used to establish a total order on identifiers.
  order_position_type position_;

  /// @brief A pointer to following order's head.
  order_node* next_;

  /// @brief A pointer to the nested order's head.
  order_node* nested_;

  /// @brief The path to this node.
  std::shared_ptr<path_type> path_ptr_;

public:

  /// @internal
  /// @brief Constructor.
  order_node( order_identifier<C> id, variable_type var, unsigned int pos
            , order_node* nxt, order_node* nst
            , std::shared_ptr<path_type> path)
    : identifier_{std::move(id)}
    , variable_{var}
    , position_{pos}
    , next_{nxt}
    , nested_{nst}
    , path_ptr_{std::move(path)}
  {}

  /// @brief Default constructor.
  ///
  /// Needed by std::vector.
  order_node() = default;

  /// @brief Default move constructor.
  order_node(order_node&&) = default;

  /// @brief Default move operator.
  order_node& operator=(order_node&&) = default;

  /// @brief Can't copy-construct a node.
  order_node(const order_node&) = delete;

  /// @brief Can't copy a node.
  order_node& operator=(const order_node&) = delete;

  /// @brief Get the identifier of this node.
  const order_identifier<C>&
  identifier()
  const noexcept
  {
    return identifier_;
  }

  /// @brief Get the variable of this node.
  variable_type
  variable()
  const noexcept
  {
    return variable_;
  }

  /// @brief Get the position of this node.
  order_position_type
  position()
  const noexcept
  {
    return position_;
  }

  /// @brief Get the successor of this node.
  const order_node*
  next()
  const noexcept
  {
    return next_;
  }

  /// @brief Get the nested nodes of this node.
  const order_node*
  nested()
  const noexcept
  {
    return nested_;
  }

  /// @brief Get the path, in hierarchy, leading to this node.
  const path_type&
  path()
  const noexcept
  {
    return *path_ptr_;
  }

  /// @brief Tell if lhs is before rhs.
  friend
  bool
  operator<(const order_node& lhs, const order_node& rhs)
  noexcept
  {
    return lhs.position() < rhs.position();
  }
}; // class order_node

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd
