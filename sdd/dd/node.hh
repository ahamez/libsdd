/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <algorithm>  // equal, for_each
#include <functional> // hash
#include <iosfwd>

#include "sdd/dd/alpha.hh"
#include "sdd/dd/definition.hh"
#include "sdd/util/hash.hh"
#include "sdd/util/packed.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief  A non-terminal node in an SDD.
/// \tparam Valuation If a set of values, define a flat node; if an SDD, define a hierarchical
/// node.
///
/// For the sake of canonicity, a node shall not exist in several locations, thus we prevent
/// its copy. Also, to enforce this canonicity, we need that nodes have always the same
/// address, thus they can't be moved to an other memory location once created.
template <typename C, typename Valuation>
class LIBSDD_ATTRIBUTE_PACKED node final
{
  // Can't copy a node.
  node& operator=(const node&) = delete;
  node(const node&) = delete;  

  // Can't move a node.
  node& operator=(node&&) = delete;
  node(node&&) = delete;

public:

  /// @brief The type of the variable of this node.
  using variable_type = typename C::variable_type;

  /// @brief The type of the valuation of this node.
  using valuation_type = Valuation;

  /// @brief The type used to store the number of arcs of this node.
  using alpha_size_type = typename C::alpha_size_type;

  /// @brief A (const) iterator on the arcs of this node.
  using const_iterator = const arc<C, Valuation>*;

  /// @brief The arc type.
  using arc_type = arc<C, Valuation>;

private:

  /// @brief The variable of this node.
  const variable_type variable_;

  /// @brief The number of arcs of this node.
  const alpha_size_type size_;

public:

  /// @internal
  /// @brief Constructor.
  ///
  /// O(n) where n is the number of arcs in the builder.
  /// It can't throw as the memory for the alpha has already been allocated.
  node(variable_type var, dd::alpha_builder<C, Valuation>& builder)
  noexcept
    : variable_(var), size_(static_cast<alpha_size_type>(builder.size()))
  {
    // Instruct the alpha builder to place it right after the node.
    builder.consolidate(alpha_addr());
  }

  /// @internal
  /// @brief Destructor.
  ///
  /// O(n) where n is the number of arcs in the node.
  ~node()
  {
    for (auto& a : *this)
    {
      a.~arc<C, Valuation>();
    }
  }

  /// @brief Get the variable of this node.
  ///
  /// O(1).
  variable_type
  variable()
  const noexcept
  {
    return variable_;
  }

  /// @brief Get the beginning of arcs.
  ///
  /// O(1).
  const_iterator
  begin()
  const noexcept
  {
    return reinterpret_cast<const arc<C, Valuation>*>(alpha_addr());
  }

  /// @brief Get the end of arcs.
  ///
  /// O(1).
  const_iterator
  end()
  const noexcept
  {
    return reinterpret_cast<const arc<C, Valuation>*>(alpha_addr()) + size_;
  }

  /// @brief Get the number of arcs.
  ///
  /// O(1).
  alpha_size_type
  size()
  const noexcept
  {
    return size_;
  }

  /// @brief   Equality of two nodes.
  ///
  /// O(1) if nodes don't have the same number of arcs; otherwise O(n) where n is the number of
  /// arcs.
  friend
  bool
  operator==(const node& lhs, const node& rhs)
  noexcept
  {
    return lhs.size_ == rhs.size_ and lhs.variable_ == rhs.variable_
       and std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  /// @brief   Export a node to a stream.
  friend
  std::ostream&
  operator<<(std::ostream& os, const node& n)
  {
    // +n.variable(): widen the type, useful to print the values of char and unsigned char types.
    os << +n.variable_ << "[";
    std::for_each( n.begin(), n.end() - 1
                 , [&](const auto& a){os << a.valuation() << " --> " << a.successor() << " || ";});
    return os << (n.end() - 1)->valuation() << " --> " << (n.end() - 1)->successor() << "]";
  }

private:

  /// @internal
  /// @brief Return the address of the alpha function.
  ///
  /// O(1).
  /// The alpha function is located right after the current node, so to compute its address,
  /// we just have to add the size of a node to the address of the current node.
  char*
  alpha_addr()
  const noexcept
  {
    return reinterpret_cast<char*>(const_cast<node*>(this)) + sizeof(node);
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::node
template <typename C, typename Valuation>
struct hash<sdd::node<C, Valuation>>
{
  std::size_t
  operator()(const sdd::node<C, Valuation>& n)
  const
  {
    using namespace sdd::hash;
    return seed(n.variable()) (range(n));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
