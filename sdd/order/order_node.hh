#ifndef _SDD_ORDER_ORDER_NODE_HH_
#define _SDD_ORDER_ORDER_NODE_HH_

#include <memory>    // shared_ptr
#include <vector>

#include "sdd/order/order_builder.hh"
#include "sdd/util/hash.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The position of an order's node, unique to it.
using order_position_type = unsigned int;

/*------------------------------------------------------------------------------------------------*/

/// @brief A node in an order: an identifier associated to an SDD variable.
template <typename C>
class order_node
{
public:

  /// @brief A user's identifier type.
  using identifier_type = typename C::Identifier;

  /// @brief A library's variable type.
  using variable_type = typename C::Variable;

  /// @brief A path, following hierarchies, to a node.
  using path_type = std::vector<order_position_type>;

private:

  /// @brief The (user's) identifier of this node.
  identifier_type identifier_;

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

  /// @brief Constructor.
  order_node( const identifier_type& id, variable_type var, unsigned int pos
            , order_node* nxt, order_node* nst
            , const std::shared_ptr<path_type>& path)
    : identifier_(id), variable_(var), position_(pos), next_(nxt), nested_(nst), path_ptr_(path)
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
  const identifier_type&
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
  const order_node* const
  next()
  const noexcept
  {
    return next_;
  }

  /// @brief Get the nested nodes of this node.
  const order_node* const
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
}; // class order_node

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_ORDER_ORDER_NODE_HH_
