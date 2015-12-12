/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <vector>

#include "sdd/order/strategies/force_hyperedge_fwd.hh"

namespace sdd { namespace force {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename Identifier>
class vertex
{
public:

  /// @brief The user's identifier.
  using identifier_type = Identifier;

private:

  /// @brief The corresponding identifier.
  const identifier_type id_;

  /// @brief This vertex's tentative location.
  double location_;

  /// @brief The hyperedges this vertex is connected to.
  std::vector<hyperedge<identifier_type>*> hyperedges_;

public:

  /// @brief Constructor.
  vertex(const identifier_type& id, double l)
    : id_(id), location_(l), hyperedges_()
  {}

  /// @brief Get the tentative location.
  double&
  location()
  noexcept
  {
    return location_;
  }

  /// @brief Get the tentative location.
  double
  location()
  const noexcept
  {
    return location_;
  }

  /// @brief Get the identifier associated to this vertex.
  const identifier_type&
  id()
  const noexcept
  {
    return id_;
  }

  /// @brief Get the hyperedges this vertex is connected to.
  std::vector<hyperedge<identifier_type>*>&
  hyperedges()
  noexcept
  {
    return hyperedges_;
  }

  /// @brief Get the hyperedges this vertex is connected to.
  const std::vector<hyperedge<identifier_type>*>&
  hyperedges()
  const noexcept
  {
    return hyperedges_;
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::force
