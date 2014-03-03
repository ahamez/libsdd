#ifndef _SDD_ORDER_STRATEGIES_FORCE_HH_
#define _SDD_ORDER_STRATEGIES_FORCE_HH_

#include <cassert>
#include <functional> // reference_wrapper
#include <limits>
#include <numeric>    // accumulate
#include <stdexcept>  // runtime_error
#include <vector>

#include <boost/container/flat_set.hpp>

#include <fstream>

#include "sdd/hom/definition.hh"
#include "sdd/order/order.hh"
#include "sdd/order/strategies/force_hyperedge.hh"
#include "sdd/order/strategies/force_vertex.hh"

namespace sdd { namespace force {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
class mk_hyperedges_visitor
{
private:

  /// @brief Where to get vertices to connect to hyperedges.
  std::vector<vertex>& vertices_;

  /// @brief Where to put hyperedges created by this visitor.
  std::vector<hyperedge>& hyperedges_;

public:

  /// @brief Required by the mem::variant visitor mechanism.
  using result_type = boost::container::flat_set<vertex*>;

  /// @brief Constructor.
  mk_hyperedges_visitor(std::vector<vertex>& v, std::vector<hyperedge>& h)
    : vertices_(v), hyperedges_(h)
  {}

  /// @brief composition is a node of the AST.
  result_type
  operator()(const hom::_composition<C>& c)
  const
  {
    result_type&& res = visit(*this, c.left());
    auto&& r = visit(*this, c.right());
    res.insert(r.begin(), r.end());
    if (not res.empty())
    {
      hyperedges_.emplace_back(res);
    }
    return res;
  }

#if !defined(HAS_NO_BOOST_COROUTINE)
  /// @brief expression is a leaf of the AST.
  result_type
  operator()(const hom::_expression<C>& e)
  const
  {
    // There are several variables touched by an expression, we keep this information.
    result_type res;
    res.reserve(e.operands().size());
    for (const auto& position : e.operands())
    {
      res.insert(&vertices_[position]);
    }
    if (not res.empty())
    {
      hyperedges_.emplace_back(res);
    }
    return res;
  }
#endif // !defined(HAS_NO_BOOST_COROUTINE)

  /// @brief fixpoint is a node of the AST.
  result_type
  operator()(const hom::_fixpoint<C>& f)
  const
  {
    // We don't need  to create an hyperedge as it's the same as the nested operand.
    return visit(*this, f.hom());
  }

  /// @brief function is a leaf of the AST.
  result_type
  operator()(const hom::_function<C>& f)
  const
  {
    result_type res;
    res.insert(&vertices_[f.target()]);
    return res;
  }

  /// @brief inductive is a leaf of the AST.
  ///
  /// There's a problem: we don't know ahead of time on what variable an inductive applies.
  /// @todo Ask the inductive if it skips for every variable of the order (it will just be an
  /// approximation though).
  result_type
  operator()(const hom::_inductive<C>&)
  const
  {
    return result_type();
  }

  /// @brief intersection is a node of the AST.
  result_type
  operator()(const hom::_intersection<C>& i)
  const
  {
    result_type res;
    for (const auto& h : i)
    {
      auto&& tmp = visit(*this, h);
      res.insert(tmp.begin(), tmp.end());
    }
    if (not res.empty())
    {
      hyperedges_.emplace_back(res);
    }
    return res;
  }

  /// @brief local is a node of the AST.
  ///
  /// Hierarchy is unsupported for the moment.
  result_type
  operator()(const hom::_local<C>& l)
  const
  {
    throw std::runtime_error("FORCE:  local homomorphism is unsupported.");
  }

  /// @brief saturation_fixpoint is a node of the AST.
  result_type
  operator()(const hom::_saturation_fixpoint<C>& sf)
  const
  {
    // We don't want to create an hyperedge for saturation operations.
    result_type&& res = visit(*this, sf.F());
    auto&& l = visit(*this, sf.L());
    res.insert(l.begin(), l.end());
    for (auto cit = sf.G_begin(); cit != sf.G_end(); ++cit)
    {
      auto&& g = visit(*this, *cit);
      res.insert(g.begin(), g.end());
    }
    return res;
  }

  /// @brief saturation_intersection is a node of the AST.
  result_type
  operator()(const hom::_saturation_intersection<C>& si)
  const
  {
    // We don't want to create an hyperedge for saturation operations.
    result_type res;
    if (si.F())
    {
      auto&& f = visit(*this, *si.F());
      res.insert(f.begin(), f.end());
    }
    if (si.L())
    {
      auto&& l = visit(*this, *si.L());
      res.insert(l.begin(), l.end());
    }
    for (const auto& h : si.G())
    {
      auto&& g = visit(*this, h);
      res.insert(g.begin(), g.end());
    }
    return res;
  }

  /// @brief saturation_sum is a node of the AST.
  result_type
  operator()(const hom::_saturation_sum<C>& si)
  const
  {
    // We don't want to create an hyperedge for saturation operations.
    result_type res;
    if (si.F())
    {
      auto&& f = visit(*this, *si.F());
      res.insert(f.begin(), f.end());
    }
    if (si.L())
    {
      auto&& l = visit(*this, *si.L());
      res.insert(l.begin(), l.end());
    }
    for (const auto& h : si.G())
    {
      auto&& g = visit(*this, h);
      res.insert(g.begin(), g.end());
    }
    return res;
  }

  /// @brief simple_expression is a leaf of the AST.
  result_type
  operator()(const hom::_simple_expression<C>& e)
  const
  {
    // There are several variables touched by a simple expression, we keep this information.
    result_type res;
    res.reserve(e.operands().size());
    for (const auto& position : e.operands())
    {
      res.insert(&vertices_[position]);
    }
    if (not res.empty())
    {
      hyperedges_.emplace_back(res);
    }
    return res;
  }

  /// @brief sum is a node of the AST.
  result_type
  operator()(const hom::_sum<C>& s)
  const
  {
    result_type res;
    for (const auto& h : s)
    {
      auto&& tmp = visit(*this, h);
      res.insert(tmp.begin(), tmp.end());
    }
    if (not res.empty())
    {
      hyperedges_.emplace_back(res);
    }
    return res;
  }

  /// @brief All other homomorphisms that are leaves of the AST, but that do not target a specific
  /// variable.
  template <typename T>
  result_type
  operator()(const T&)
  const
  {
    return result_type();
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
class worker
{
private:

  /// @brief Keep the order.
  const order<C> o_;

  /// @brief Keep the homomorphism.
  const homomorphism<C> h_;

  /// @brief Map an order_node position to its corresponding vertex;
  ///
  /// The order_node position is the index in this vector.
  std::vector<vertex> vertices_;

  /// @brief The hyperedges that link together vertices.
  std::vector<hyperedge> hyperedges_;

public:

  /// @brief Constructor.
  worker(const order<C>& o, const homomorphism<C>& h)
    : o_(o), h_(h), vertices_(), hyperedges_()
  {
    // Assign a initial location for every vertex.
    vertices_.reserve(o_.nodes().size());
    for (std::size_t i = 0; i < o_.nodes().size(); ++i)
    {
      vertices_.emplace_back(i /* position in order.nodes */, i /* initial location */);
    }

    // Build hyperedges
    visit(mk_hyperedges_visitor<C>(vertices_, hyperedges_), h);

    std::ofstream dot_file("/tmp/force.dot");
    if (dot_file.is_open())
    {
      to_dot(dot_file);
    }

    // Update all vertices with their connected hyperedges
    for (auto& edge : hyperedges_)
    {
      for (auto& vertex_ptr : edge.vertices)
      {
        vertex_ptr->hyperedges.push_back(&edge);
      }
    }
  }

  /// @brief Dump the hypergraph to the DOT format.
  void
  to_dot(std::ostream& os)
  const
  {
    os << "graph hypergraph { layout=fdp;" << std::endl;
    for (const auto& vertex : vertices_)
    {
      os << "vertex_" << vertex.pos << " [label=\"" << o_.nodes()[vertex.pos].identifier().user()
         << "\"];" << std::endl;
    }
    for (const auto& edge : hyperedges_)
    {
      os << "hyperedge_" << &edge << " [label=\"\",shape=point]" << std::endl;
      for (const auto& vertex_ptr : edge.vertices)
      {
        os << "hyperedge_" << &edge << " -- vertex_" << vertex_ptr->pos << ";"
           << std::endl;
      }
    }
    os << "}" << std::endl;
  }

  /// @brief Effectively apply the FORCE ordering strategy.
  order<C>
  operator()()
  {
    std::vector<std::reference_wrapper<vertex>> sorted_vertices(vertices_.begin(), vertices_.end());

    double span = std::numeric_limits<double>::max();
    double old_span = 0;

    do
    {
      old_span = span;

      // Compute the new center of gravity for every hyperedge.
      for (auto& edge : hyperedges_)
      {
        edge.center_of_gravity();
      }

      // Compute the tentative new location of every vertex.
      for (auto& vertex : vertices_)
      {
        assert(not vertex.hyperedges.empty());
        vertex.location = std::accumulate( vertex.hyperedges.cbegin(), vertex.hyperedges.cend(), 0
                                         , [](double acc, const hyperedge* e){return acc + e->cog;}
                                         ) / vertex.hyperedges.size();
      }

      // Sort tentative vertex locations.
      std::sort( sorted_vertices.begin(), sorted_vertices.end()
               , [](vertex& lhs, vertex& rhs){return lhs.location < rhs.location;});

      // Assign integer indices to the vertices.
      unsigned int pos = 0;;
      std::for_each( sorted_vertices.begin(), sorted_vertices.end()
                   , [&pos](vertex& v){v.location = pos++;});

      span = get_total_span();
    } while (old_span > span);

    order_builder<C> ob;
//    for (auto rcit = sorted_vertices.rbegin(); rcit != sorted_vertices.rend(); ++rcit)
//    {
//      ob.push(o_.nodes()[rcit->get().pos].identifier().user());
//    }
    for (const auto& vertex : sorted_vertices)
    {
      ob.push(o_.nodes()[vertex.get().pos].identifier().user());
    }

    return order<C>(ob);
  }

private:

  double
  get_total_span()
  const noexcept
  {
    return std::accumulate( hyperedges_.cbegin(), hyperedges_.cend(), 0
                          , [](double acc, const hyperedge& h){return acc + h.span();});
  }
};

} // namespace force

/*------------------------------------------------------------------------------------------------*/

/// @brief The FORCE ordering strategy.
///
/// See http://dx.doi.org/10.1145/764808.764839 for the details.
template <typename C>
order<C>
force_ordering(const order<C>& o, const homomorphism<C>& h)
{
  if (o.empty() or h == id<C>())
  {
    return o;
  }

  return force::worker<C>(o, h)();
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_ORDER_STRATEGIES_FORCE_HH_
