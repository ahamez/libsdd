#pragma once

#include <unordered_set>

#include "sdd/dd/definition.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct to_dot_visitor
{
  /// @brief A cache is necessary to to know if a node has already been encountered.
  ///
  /// We use the addresses of nodes as key. It's legit because nodes are unified and immutable.
  std::unordered_set<std::size_t> visited_;

  /// @brief The stream to export to.
  std::ostream& os_;

  /// @brief Constructor.
  to_dot_visitor(std::ostream& os)
    : visited_(), os_(os)
  {}

  /// @brief |0|.
  auto
  operator()(const zero_terminal<C>& n, const order<C>&, unsigned int)
  {
    const auto addr = reinterpret_cast<std::size_t>(&n);
    const auto search = visited_.find(addr);
    const auto node = node_string(addr);
    if (search == visited_.end())
    {
      os_ << node << " [shape=square,label=\"0\"];\n";
      visited_.emplace_hint(search, addr);
    }
    return node;
  }

  /// @brief |1|.
  auto
  operator()(const one_terminal<C>& n, const order<C>&, unsigned int)
  {
    const auto addr = reinterpret_cast<std::size_t>(&n);
    const auto search = visited_.find(addr);
    const auto node = node_string(addr);
    if (search == visited_.end())
    {
      os_ << node << " [shape=square,label=\"1\"];\n";
      visited_.emplace_hint(search, addr);
    }
    return node;
  }

  /// @brief Flat SDD.
  auto
  operator()(const flat_node<C>& n, const order<C>& o, unsigned int depth)
  {
    const auto addr = reinterpret_cast<std::size_t>(&n);
    const auto search = visited_.find(addr);
    const auto node = node_string(addr);
    if (search == visited_.end())
    {
      os_ << node << " [label=\"" << o.identifier() << "\"];\n";
      for (const auto& arc : n)
      {
        const auto succ = visit(*this, arc.successor(), o.next(), depth);
        os_ << node << " -> " << succ << " [label=\"" << arc.valuation() << "\"];\n";
      }
      visited_.emplace_hint(search, addr);
    }
    return node;
  }

  /// @brief Hierarchical SDD.
  auto
  operator()(const hierarchical_node<C>& n, const order<C>& o, unsigned int depth)
  {
    const auto addr = reinterpret_cast<std::size_t>(&n);
    const auto search = visited_.find(addr);
    const auto node = node_string(addr);
    if (search == visited_.end())
    {
      os_ << node << " [label=\"" << o.identifier() << "\"];\n";
      for (const auto& arc : n)
      {
        const auto succ = visit(*this, arc.successor(), o.next(), depth);
        const auto hier = visit(*this, arc.valuation(), o.nested(), depth + 1);
        const auto ghost = "g" + std::to_string(reinterpret_cast<std::size_t>(addr)) + succ;

        os_ << ghost << " [shape=point,label=\"\",height=0,width=0];\n";

        os_ << node << " -> " << ghost << " [arrowhead=none];\n"
            << ghost << " -> " << succ << ";" << std::endl
            << ghost << " -> " << hier << " [style=dotted];\n";

      }
      visited_.emplace_hint(search, addr);
    }

    return node;
  }

private:

  static
  std::string
  node_string(std::size_t x)
  {
    return "n" + std::to_string(x);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct to_dot
{
  const SDD<C> x_;
  const order<C>& o_;

  to_dot(const SDD<C>& x, const order<C>& o)
    : x_(x), o_(o)
  {}

  friend
  std::ostream&
  operator<<(std::ostream& out, const to_dot& manip)
  {
    out << "digraph sdd {\n";
    visit(to_dot_visitor<C>(out), manip.x_, manip.o_, 0);
    return out << "}\n";
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Export an SDD to the DOT format.
///
/// Hierarchical SDD are not supported yet.
template <typename C>
to_dot<C>
dot(const SDD<C>& x, const order<C>& o)
{
  return to_dot<C>(x, o);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools
