/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <sstream>
#include <unordered_set>

#include "sdd/hom/definition.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct hom_to_dot_visitor
{
  /// @brief A cache is necessary to to know if a node has already been encountered.
  ///
  /// We use the addresses of nodes as key. It's legit because nodes are unified and immutable.
  mutable std::unordered_set<const void*> visited_;

  /// @brief The stream to export to.
  std::ostream& os_;

  /// @brief Constructor.
  hom_to_dot_visitor(std::ostream& os)
    : visited_(), os_(os)
  {}

  std::string
  operator()(const hom::_composition<C>& h)
  const
  {
    if (not visited(h))
    {
      os_ << node("h", &h) << "[label=\"o\"];\n";
      const auto left = visit(*this, h.left);
      os_ << node("h", &h) << " -> " << left << "[label=\"l\"];\n";
      const auto right = visit(*this, h.right);
      os_ << node("h", &h) << " -> " << right << "[label=\"r\"];\n";
    }
    return node("h", &h);
  }

  std::string
  operator()(const hom::_fixpoint<C>& h)
  const
  {
    if (not visited(h))
    {
      os_ << node("h", &h) << "[label=\"*\"];\n";
      const auto n = visit(*this, h.h);
      os_ << node("h", &h) << " -> " << n << '\n';
    }
    return node("h", &h);
  }

  std::string
  operator()(const hom::_function<C>& h)
  const
  {
    if (not visited(h))
    {
      os_ << node("h", &h) << " [label=\"" << h << "\"];\n";
    }
    return node("h", &h);
  }

  std::string
  operator()(const hom::_identity<C>&)
  const
  {
    static unsigned int i = 0;
    os_ << "id" << i << " [label=\"id\"];\n";
    return "id" + std::to_string(i++);
  }

  std::string
  operator()(const hom::_local<C>& h)
  const
  {
    if (not visited(h))
    {
      os_ << node("h", &h) << " [label=\"@\"];\n";
      const auto n = visit(*this, h.h);
      os_ << node("h", &h) << " -> " << n << " [label=\"" << h.target << "\"]\n";
    }
    return node("h", &h);
  }

  std::string
  operator()(const hom::_saturation_fixpoint<C>& h)
  const
  {
    if (not visited(h))
    {
      os_ << node("h", &h) << " [label=\"$*\"];\n";
      if (h.F != id<C>())
      {
        const auto f = visit(*this, h.F);
        os_ << node("h", &h) << " -> " << f << " [label=\"F\"];\n";
      }
      unsigned int i = 0;
      for (auto&& g : h)
      {
        const auto sub_g = visit(*this, g);
        os_ << node("h", &h) << " -> " << sub_g << " [label=\"g" << i++ << "\"];\n";
      }
      if (h.L != id<C>())
      {
        const auto l = visit(*this, h.L);
        os_ << node("h", &h) << " -> " << l << " [label=\"L\"];\n";
      }
    }
    return node("h", &h);
  }

  std::string
  operator()(const hom::_sum<C>& h)
  const
  {
    if (not visited(h))
    {
      os_ << node("h", &h) << " [label=\"+\"];\n";
      for (const auto& operand : h)
      {
        const auto n = visit(*this, operand);
        os_ << node("h", &h) << " -> " << n << ";\n";
      }
    }
    return node("h", &h);
  }

  std::string
  operator()(const hom::_if_then_else<C>& h)
  const
  {
    if (not visited(h))
    {
      os_ << node("h", &h) << " [label=\"ITE\"];\n";

      const auto pred = visit(*this, h.h_if);
      os_ << node("h", &h) << " -> " << pred << " [label=\"if\"];\n";

      const auto then_branch = visit(*this, h.h_then);
      os_ << node("h", &h) << " -> " << then_branch << " [label=\"then\"];\n";

      const auto else_branch = visit(*this, h.h_else);
      os_ << node("h", &h) << " -> " << else_branch << " [label=\"else\"];\n";
    }
    return node("h", &h);
  }

  template <typename T>
  std::string
  operator()(const T& x)
  const
  {
    return node("h", &x);
  }

private:

  template <typename H>
  bool
  visited(const H& h)
  const
  {
    const auto insertion = visited_.emplace(&h);
    return not insertion.second;
  }

  static
  std::string
  node(const std::string& prefix, const void* addr)
  noexcept
  {
    std::ostringstream oss;
    oss << prefix << addr;
    return oss.str();
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct hom_to_dot
{
  const homomorphism<C> h_;

  friend
  std::ostream&
  operator<<(std::ostream& out, const hom_to_dot& manip)
  {
    out << "digraph homomorphism {\n";
    visit(hom_to_dot_visitor<C>(out), manip.h_);
    return out << "}\n";
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Export an homomorphism to the DOT format.
template <typename C>
hom_to_dot<C>
dot(const homomorphism<C>& h)
{
  return {h};
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools
