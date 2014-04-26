#ifndef _SDD_DD_TOOLS_DOT_SDD_HH_
#define _SDD_DD_TOOLS_DOT_SDD_HH_

#include <unordered_set>

#include "sdd/dd/definition.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct to_dot_visitor
{
  /// @brief Required by mem::variant visitor mechanism.
  using result_type = const void*;

  /// @brief A cache is necessary to to know if a node has already been encountered.
  ///
  /// We use the addresses of nodes as key. It's legit because nodes are unified and immutable.
  mutable std::unordered_set<const void*> visited_;

  /// @brief The stream to export to.
  std::ostream& os_;

  /// @brief Constructor.
  to_dot_visitor(std::ostream& os)
    : visited_(), os_(os)
  {}

  /// @brief |0|.
  result_type
  operator()(const zero_terminal<C>& n)
  const
  {
    const auto addr = reinterpret_cast<const void*>(&n);
    const auto search = visited_.find(addr);
    if (search == visited_.end())
    {
      os_ << "node_" << addr << " [shape=square,label=\"0\"];" << std::endl;
      visited_.emplace_hint(search, addr);
    }
    return addr;
  }

  /// @brief |1|.
  result_type
  operator()(const one_terminal<C>& n)
  const
  {
    const auto addr = reinterpret_cast<const void*>(&n);
    const auto search = visited_.find(addr);
    if (search == visited_.end())
    {
      os_ << "node_" << addr << " [shape=square,label=\"1\"];" << std::endl;
      visited_.emplace_hint(search, addr);
    }
    return addr;
  }

  /// @brief Flat SDD.
  result_type
  operator()(const flat_node<C>& n)
  const
  {
    const auto addr = reinterpret_cast<const void*>(&n);
    const auto search = visited_.find(addr);
    if (search == visited_.end())
    {
      os_ << "node_" << addr << std::endl;
      for (const auto& arc : n)
      {
        const auto succ = visit(*this, arc.successor());
        os_ << "node_" << addr << " -> " << "node_" << succ
            << " [label=\"" << arc.valuation() << "\"];"
            << std::endl;
      }
      visited_.emplace_hint(search, addr);
    }
    return addr;
  }

  /// @brief Hierarchical SDD.
  result_type
  operator()(const hierarchical_node<C>&)
  const
  {
    std::cerr << "DOT export of hierarchical SDD not supported" << std::endl;
    return nullptr;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct to_dot
{
  const SDD<C> x_;

  to_dot(const SDD<C>& x)
    : x_(x)
  {}

  friend
  std::ostream&
  operator<<(std::ostream& out, const to_dot& manip)
  {
    out << "digraph sdd {" << std::endl;
    visit(to_dot_visitor<C>(out), manip.x_);
    return out << "}" << std::endl;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Export an SDD to the DOT format.
///
/// Hierarchical SDD are not supported yet.
template <typename C>
to_dot<C>
dot(const SDD<C>& x)
{
  return to_dot<C>(x);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools

#endif // _SDD_DD_TOOLS_DOT_SDD_HH_
