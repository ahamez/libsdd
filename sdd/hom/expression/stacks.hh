#ifndef _SDD_HOM_EXPRESSION_STACKS_HH_
#define _SDD_HOM_EXPRESSION_STACKS_HH_

#include <memory> // shared_ptr

#include "sdd/dd/context_fwd.hh"
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
/// @brief The type of stack of successors of hierarchical nodes.
template <typename C>
struct sdd_stack
{
  SDD<C> sdd;
  std::shared_ptr<sdd_stack> next;

  sdd_stack(const SDD<C>& x, const std::shared_ptr<sdd_stack>& n)
    : sdd(x), next(n)
  {}
};


/*------------------------------------------------------------------------------------------------*/

}}} // namespace sdd::hom::expr

#endif // _SDD_HOM_EXPRESSION_STACKS_HH_
