/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <initializer_list>
#include <memory>    // shared_ptr
#include <vector>

#include "sdd/order/order_identifier.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief Prepare an order to build.
template <typename C>
class order_builder final
{
public:

  /// @brief The type of an identifier.
  using identifier_type = typename C::Identifier;

private:

  // Pre-declaration of a node.
  struct node;

  /// @brief The type of a pointer to a node.
  ///
  /// It's the actual reprensation of an order to build
  using node_ptr = std::shared_ptr<node>;

  /// @brief The building block of an order_builder.
  struct node
  {
    /// @brief The node's identifier.
    const order_identifier<C> identifier;

    /// @brief The nested order.
    ///
    /// If nullptr, this node is a flat node.
    const node_ptr nested;

    /// @brief The node's next variable.
    ///
    /// If nullptr, this node is the last one.
    const node_ptr next;

    /// @brief Constructor.
    node(order_identifier<C> id, node_ptr nst, node_ptr nxt)
      : identifier{std::move(id)}
      , nested{std::move(nst)}
      , next{std::move(nxt)}
    {}
  };

  /// @brief The concrete order.
  node_ptr ptr_;

public:

  /// @brief Default constructor.
  order_builder()
    : ptr_{nullptr}
  {}

  /// @brief Constructor from a list of identifiers.
  template <typename InputIterator>
  order_builder(InputIterator begin, InputIterator end)
    : order_builder()
  {
    std::vector<identifier_type> tmp(begin, end);
    for (auto rcit = tmp.crbegin(); rcit != tmp.crend(); ++rcit)
    {
      push(*rcit, nullptr);
    }
  }

  /// @brief Constructor from a list of identifiers.
  order_builder(std::initializer_list<identifier_type> list)
    : order_builder(list.begin(), list.end())
  {}

  /// @brief Constructor with a single identifier.
  order_builder(identifier_type&& id)
    : ptr_{mk_ptr(std::move(id), nullptr, nullptr)}
  {}

  /// @brief Constructor with a single identifier.
  order_builder(const identifier_type& id)
    : ptr_{mk_ptr(id, nullptr, nullptr)}
  {}

  /// @brief Constructor with an identifier and its associated nested order.
  order_builder(identifier_type&& id, const order_builder& nested)
    : ptr_{mk_ptr(std::move(id), nested.ptr_, nullptr)}
  {}

  /// @brief Constructor with an identifier and its associated nested order.
  order_builder(const identifier_type& id, const order_builder& nested)
    : ptr_{mk_ptr(id, nested.ptr_, nullptr)}
  {}

  /// @internal
  order_builder(const order_identifier<C>& id, const order_builder& nested)
    : ptr_{mk_ptr(id, nested.ptr_, nullptr)}
  {}

  /// @internal
  order_builder(order_identifier<C>&& id, const order_builder& nested)
    : ptr_{mk_ptr(std::move(id), nested.ptr_, nullptr)}
  {}

  /// @internal
  order_builder(const order_identifier<C>& id)
    : ptr_{mk_ptr(id, nullptr, nullptr)}
  {}

  /// @internal
  order_builder(order_identifier<C>&& id)
    : ptr_{mk_ptr(std::move(id), nullptr, nullptr)}
  {}

  /// @internal
  /// @brief Tell if this order is empty.
  ///
  /// It's unsafe to call any other method, except push(), if this order is empty.
  bool
  empty()
  const noexcept
  {
    return not ptr_;
  }

  /// @internal
  /// @brief Get the identifier of this order's head.
  const order_identifier<C>&
  identifier()
  const noexcept
  {
    return ptr_->identifier;
  }

  /// @internal
  /// @brief Get this order's head's next order.
  order_builder
  next()
  const noexcept
  {
    return order_builder(ptr_->next);
  }

  /// @internal
  /// @brief Get this order's head's nested order.
  order_builder
  nested()
  const noexcept
  {
    return order_builder(ptr_->nested);
  }

  /// @brief Push a flat identifier at the top of this order.
  order_builder&
  push(const identifier_type& id)
  {
    return push(order_identifier<C>(id), nullptr);
  }

  /// @brief Add a nested identifier at the top of this order.
  ///
  /// If nested is an empty order, a flat identifier is added instead.
  order_builder&
  push(const identifier_type& id, const order_builder& nested)
  {
    return push(order_identifier<C>(id), nested.ptr_);
  }

  /// @internal
  order_builder&
  push(const order_identifier<C>& id, const order_builder& nested)
  {
    return push(id, nested.ptr_);
  }

  /// @brief Appends an order to this one.
  order_builder
  operator<<(const order_builder& next)
  {
    ptr_ = concat(ptr_, next.ptr_);
    return *this;
  }

  /// @brief Get the number of identifiers.
  std::size_t
  size()
  const noexcept
  {
    std::size_t acc = 0;
    node_size(acc, ptr_);
    return acc;
  }

  /// @brief Get the number of identifiers, at the current level only.
  std::size_t
  height()
  const noexcept
  {
    std::size_t res = 1;
    node_ptr current = ptr_->next;
    while (current)
    {
      current = current->next;
      ++res;
    }
    return res;
  }

private:

  /// @brief Get the number of identifiers of a node.
  static
  void
  node_size(std::size_t& acc, node_ptr ptr)
  noexcept
  {
    if (ptr)
    {
      ++acc;
      if (ptr->nested)
      {
        node_size(acc, ptr->nested);
      }
      if (ptr->next)
      {
        node_size(acc, ptr->next);
      }
    }
  }

  /// @brief Concatenate two order builders.
  static
  node_ptr
  concat(node_ptr head, node_ptr next)
  {
    return head
         ? std::make_shared<node>(head->identifier, head->nested, concat(head->next, next))
         : next;
  }

  /// @brief Constructor from an already existing pointer.
  order_builder(node_ptr ptr)
  : ptr_{std::move(ptr)}
  {}

  /// @brief Actual implementation of push.
  order_builder&
  push(const order_identifier<C>& id, const node_ptr& nested_ptr)
  {
    ptr_ = std::make_shared<node>(id, nested_ptr, ptr_);
    return *this;
  }

  /// @brief Helper for constructors.
  static
  node_ptr
  mk_ptr(const order_identifier<C>& id, const node_ptr& nested_ptr, const node_ptr& next_ptr)
  {
    return std::make_shared<node>(id, nested_ptr, next_ptr);
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd
