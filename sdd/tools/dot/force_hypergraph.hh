/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <iosfwd>

#include "sdd/order/strategies/force_hypergraph.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct force_hypergraph_to_dot
{
  const force::hypergraph<C>& g_;

  friend
  std::ostream&
  operator<<(std::ostream& os, const force_hypergraph_to_dot& manip)
  {
    os << "graph hypergraph { layout=fdp; splines=true;" << std::endl;
    for (const auto& vertex : manip.g_.vertices())
    {
      os << "v" << vertex.id() << "[label=\"" << vertex.id() << "\"];" << std::endl;
    }
    for (const auto& edge : manip.g_.hyperedges())
    {
      os << "h" << &edge << " [label=\"\",shape=point]" << std::endl;
      for (const auto& vertex_ptr : edge.vertices())
      {
        os << "h" << &edge << " -- v" << vertex_ptr->id() << ";" << std::endl;
      }
    }
    return os << "}" << std::endl;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Export a FORCE hypergraph to the DOT format.
template <typename C>
force_hypergraph_to_dot<C>
dot(const force::hypergraph<C>& g)
{
  return {g};
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools
