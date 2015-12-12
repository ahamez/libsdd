/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <cassert>
#include <memory>

#include "sdd/dd/definition.hh"
#include "sdd/dd/path_generator_fwd.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct paths_visitor
{
  path_push_type<C>& yield;

  void
  operator()( const hierarchical_node<C>& n
            , std::shared_ptr<path<C>> path, std::shared_ptr<sdd_stack<C>> stack)
  const
  {
    for (const auto& arc : n)
    {
      const auto local_stack = std::make_shared<sdd_stack<C>>(arc.successor(), stack);
      visit(*this, arc.valuation(), path, local_stack);
    }
  }

  void
  operator()( const flat_node<C>& n
            , std::shared_ptr<path<C>> path, std::shared_ptr<sdd_stack<C>> stack)
  const
  {
    for (const auto& arc : n)
    {
      path->emplace_back(arc.valuation());
      visit(*this, arc.successor(), path, stack);
      path->pop_back();
    }
  }

  void
  operator()( const one_terminal<C>&
            , std::shared_ptr<path<C>> path, std::shared_ptr<sdd_stack<C>> stack)
  const
  {
    if (stack)
    {
      visit(*this, stack->sdd, path, stack->next);
      return;
    }
    // end of a path
    yield(*path);
  }

  void
  operator()( const zero_terminal<C>&
            , std::shared_ptr<path<C>> path, std::shared_ptr<sdd_stack<C>>)
  const
  {
    assert(path->empty() && "Non empty path leading to |0|");
    yield(*path);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
void
paths(path_push_type<C>& yield, const SDD<C>& s)
{
  path<C> tmp;
  tmp.reserve(512);
  visit(paths_visitor<C>{yield}, s, std::make_shared<path<C>>(std::move(tmp)), nullptr);
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd
