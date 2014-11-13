#pragma once

#include <memory> // shared_ptr

#include "sdd/dd/context_fwd.hh"
#include "sdd/dd/sdd_stack.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom { namespace expr {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct app_stack
{
  const SDD<C> sdd;
  const order<C> ord;
  std::shared_ptr<app_stack> next;

  app_stack(const SDD<C>& s, const order<C>& o, const std::shared_ptr<app_stack>& n)
    : sdd(s), ord(o), next(n)
  {}
};

/// @internal
/// @brief The type of stack of results to put as a successor of a hierarchical node.
template <typename C>
struct res_stack
{
  dd::sum_builder<C, SDD<C>> result;
  std::shared_ptr<res_stack> next;

  res_stack(dd::context<C>& cxt, const std::shared_ptr<res_stack>& n)
    : result(cxt), next(n)
  {}
};

/// @internal
template <typename C>
using sdd_stack = dd::sdd_stack<C>;

/*------------------------------------------------------------------------------------------------*/

}}} // namespace sdd::hom::expr
