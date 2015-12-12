/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <algorithm>  // minmax_element
#include <numeric>    // accumulate
#include <vector>

#include "sdd/order/strategies/force_hyperedge_fwd.hh"
#include "sdd/order/strategies/force_vertex.hh"

namespace sdd { namespace force {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename Identifier>
class hyperedge
{
public:

  using identifier_type = Identifier;

private:

  /// @brief
  const double weight_;

  /// @brief The center of gravity.
  double cog_;

  /// @brief Vertices connected to this hyperedge.
  std::vector<vertex<identifier_type>*> vertices_;

public:

  /// @brief Constructor with an already existing container of vertices.
  hyperedge(double weight, std::vector<vertex<identifier_type>*>&& v)
    : weight_(weight), cog_(0), vertices_(std::move(v))
  {}

  /// @brief Return the computed center of gravity.
  double
  center_of_gravity()
  const noexcept
  {
    return cog_;
  }

  double
  weight()
  const noexcept
  {
    return weight_;
  }

  std::vector<vertex<identifier_type>*>&
  vertices()
  noexcept
  {
    return vertices_;
  }

  const std::vector<vertex<identifier_type>*>&
  vertices()
  const noexcept
  {
    return vertices_;
  }

  /// @brief Compute the center of gravity.
  void
  compute_center_of_gravity()
  noexcept
  {
    assert(not vertices_.empty());
    cog_ = std::accumulate( vertices_.cbegin(), vertices_.cend(), 0
                          , [](auto&& acc, auto&&v){return acc + v->location();}
                          ) / vertices_.size();
  }

  /// @brief Compute the span of all vertices.
  double
  span()
  const noexcept
  {
    assert(not vertices_.empty());
    const auto minmax
      = std::minmax_element( vertices_.cbegin(), vertices_.cend()
                           , [](auto&& lhs, auto&& rhs){return lhs->location() < rhs->location();});
    return (*minmax.second)->location() - (*minmax.first)->location();
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::force
