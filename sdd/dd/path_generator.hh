#ifndef _SDD_DD_PATH_GENERATOR_HH_
#define _SDD_DD_PATH_GENERATOR_HH_

#if !defined(HAS_NO_BOOST_COROUTINE)

#include <cassert>
#include <memory>

#include <boost/coroutine/coroutine.hpp>

#include "sdd/dd/definition.hh"
#include "sdd/dd/path_generator_fwd.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Generate all paths of an SDD.
///
/// It uses coroutines in order to generate them on the fly. For now, it doesn't use the visitor
/// mechanism of mem::variant, but a manual dispatch using variant's indexes.
template <typename C>
void
xpaths_impl( typename path_generator<C>::caller_type& yield
           , SDD<C> sdd, std::shared_ptr<path<C>> p, std::shared_ptr<sdd_stack<C>> stack)
{
  switch(sdd.index())
  {
    case (SDD<C>::hierarchical_node_index) :
    {
      const auto& node = sdd::mem::variant_cast<sdd::hierarchical_node<C>>(*sdd);
      for (const auto& arc : node)
      {
        const auto local_stack = std::make_shared<sdd_stack<C>>(arc.successor(), stack);
        xpaths_impl(yield, arc.valuation(), p, local_stack);
      }
      break;
    }

    case (SDD<C>::flat_node_index) :
    {
      const auto& node = sdd::mem::variant_cast<sdd::flat_node<C>>(*sdd);
      for (const auto& arc : node)
      {
        p->emplace_back(arc.valuation());
        xpaths_impl(yield, arc.successor(), p, stack);
        p->pop_back();
      }
      break;
    }

    case (SDD<C>::one_terminal_index) :
    {
      if (stack)
      {
        xpaths_impl(yield, stack->sdd, p, stack->next);
        return;
      }
      // end of a path
      yield(*p);
      break;
    }

    default /* zero_terminal */ :
    {
      assert(p->empty() && "Non empty path leading to |0|");
      yield(*p);
    }
  }
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd

#endif // !defined(HAS_NO_BOOST_COROUTINE)

#endif // _SDD_DD_PATH_GENERATOR_HH_
