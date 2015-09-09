/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include "sdd/dd/definition.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related SDD
template <typename C>
struct check_compatibility_visitor
{
  // Can't have different types of SDD.
  template <typename T, typename U>
  void
  operator()(const T&, const U&, const SDD<C>& lhs_orig, const SDD<C>& rhs_orig)
  const
  {
    throw top<C>(lhs_orig, rhs_orig);
  }

  // Same terminals is OK.
  template <typename T>
  void
  operator()(const T&, const T&, const SDD<C>&, const SDD<C>&)
  const
  {}

  void
  operator()( const flat_node<C>& lhs, const flat_node<C>& rhs
            , const SDD<C>& lhs_orig, const SDD<C>& rhs_orig)
  const
  {
    if (lhs.variable() != rhs.variable())
    {
      throw top<C>(lhs_orig, rhs_orig);
    }
  }

  void
  operator()( const hierarchical_node<C>& lhs, const hierarchical_node<C>& rhs
            , const SDD<C>& lhs_orig, const SDD<C>& rhs_orig)
  const
  {
    if (lhs.variable() != rhs.variable())
    {
      throw top<C>(lhs_orig, rhs_orig);
    }
  }
};

/// @internal
/// @related SDD
/// @brief Throws top if two SDD are incompatible, does nothing otherwise
template <typename C>
void
check_compatibility(const SDD<C>& lhs, const SDD<C>& rhs)
{
  binary_visit(check_compatibility_visitor<C>{}, lhs, rhs, lhs, rhs);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd
