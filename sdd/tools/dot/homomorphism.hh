#ifndef _SDD_DD_TOOLS_DOT_HOMOMORPHISM_HH_
#define _SDD_DD_TOOLS_DOT_HOMOMORPHISM_HH_

#include <unordered_set>

#include "sdd/hom/definition.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct hom_to_dot_visitor
{
  /// @brief Required by mem::variant visitor mechanism.
  using result_type = std::string;

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

  result_type
  operator()(const hom::_com_composition<C>& h)
  const
  {
    if (not visited(h))
    {
      os_ << node("h", &h) << " [label=\"+o+\"];" << std::endl;
      for (const auto& operand : h)
      {
        const auto n = visit(*this, operand);
        os_ << node("h", &h) << " -> " << n << ";" << std::endl;
      }
    }
    return node("h", &h);
  }

  result_type
  operator()(const hom::_composition<C>& h)
  const
  {
    if (not visited(h))
    {
      os_ << node("h", &h) << "[label=\"o\"];" << std::endl;
      const auto left = visit(*this, h.left());
      os_ << node("h", &h) << " -> " << left << "[label=\"l\"];" << std::endl;
      const auto right = visit(*this, h.right());
      os_ << node("h", &h) << " -> " << right << "[label=\"r\"];" << std::endl;
    }
    return node("h", &h);
  }

  result_type
  operator()(const hom::_fixpoint<C>& h)
  const
  {
    if (not visited(h))
    {
      os_ << node("h", &h) << "[label=\"*\"];" << std::endl;
      const auto n = visit(*this, h.hom());
      os_ << node("h", &h) << " -> " << n << std::endl;
    }
    return node("h", &h);
  }

  result_type
  operator()(const hom::_function<C>& h)
  const
  {
    if (not visited(h))
    {
      os_ << node("h", &h) << "[label=\"" << h << "\"];" << std::endl;
    }
    return node("h", &h);
  }

  result_type
  operator()(const hom::_identity<C>&)
  const
  {
    static unsigned int i = 0;
    os_ << "id" << i << " [label=\"id\"];" << std::endl;
    return "id" + std::to_string(i++);
  }

  result_type
  operator()(const hom::_local<C>& h)
  const
  {
    if (not visited(h))
    {
      os_ << node("h", &h) << " [label=\"@\"];" << std::endl;
      const auto n = visit(*this, h.hom());
      os_ << node("h", &h) << " -> " << n << " [label=\"" << h.target() << "\"];" << std::endl;
    }
    return node("h", &h);
  }

  result_type
  operator()(const hom::_saturation_com_composition<C>& h)
  const
  {
    if (not visited(h))
    {
      os_ << node("h", &h) << " [label=\"$o\"];" << std::endl;
      if (h.F())
      {
        const auto f = visit(*this, *h.F());
        os_ << node("h", &h) << " -> " << f << " [label=\"F\"];" << std::endl;
      }
      unsigned int i = 0;
      for (auto g_it = h.G().begin(); g_it != h.G().end(); ++g_it, ++i)
      {
        const auto g = visit(*this, *g_it);
        os_ << node("h", &h) << " -> " << g << " [label=\"g" << i << "\"];" << std::endl;
      }
      if (h.L())
      {
        const auto l = visit(*this, *h.L());
        os_ << node("h", &h) << " -> " << l << " [label=\"L\"];"<< std::endl;
      }
    }
    return node("h", &h);
  }

  result_type
  operator()(const hom::_saturation_fixpoint<C>& h)
  const
  {
    if (not visited(h))
    {
      os_ << node("h", &h) << " [label=\"$*\"];" << std::endl;
      if (h.F() != id<C>())
      {
        const auto f = visit(*this, h.F());
        os_ << node("h", &h) << " -> " << f << " [label=\"F\"];" << std::endl;
      }
      unsigned int i = 0;
      for (auto g_it = h.G_begin(); g_it != h.G_end(); ++g_it, ++i)
      {
        const auto g = visit(*this, *g_it);
        os_ << node("h", &h) << " -> " << g << " [label=\"g" << i << "\"];" << std::endl;
      }
      if (h.L() != id<C>())
      {
        const auto l = visit(*this, h.L());
        os_ << node("h", &h) << " -> " << l << " [label=\"L\"];"<< std::endl;
      }
    }
    return node("h", &h);
  }

  result_type
  operator()(const hom::_sum<C>& h)
  const
  {
    if (not visited(h))
    {
      os_ << node("h", &h) << " [label=\"+\"];" << std::endl;
      for (const auto& operand : h)
      {
        const auto n = visit(*this, operand);
        os_ << node("h", &h) << " -> " << n << ";" << std::endl;
      }
    }
    return node("h", &h);
  }

  template <typename T>
  result_type
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
    const auto addr = reinterpret_cast<const void*>(&h);
    const auto insertion = visited_.emplace(addr);
    return not insertion.second;
  }

  static
  std::string
  node(const std::string& prefix, const void* addr)
  noexcept
  {
    return prefix + std::to_string(reinterpret_cast<std::size_t>(addr));
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct hom_to_dot
{
  const homomorphism<C> h_;

  hom_to_dot(const homomorphism<C>& h)
    : h_(h)
  {}

  friend
  std::ostream&
  operator<<(std::ostream& out, const hom_to_dot& manip)
  {
    out << "digraph homomorphism {" << std::endl;
    visit(hom_to_dot_visitor<C>(out), manip.h_);
    return out << "}" << std::endl;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Export an homomorphism to the DOT format.
template <typename C>
hom_to_dot<C>
dot(const homomorphism<C>& h)
{
  return hom_to_dot<C>(h);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools

#endif // _SDD_DD_TOOLS_DOT_HOMOMORPHISM_HH_
