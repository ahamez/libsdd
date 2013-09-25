#ifndef _SDD_HOM_EXPRESSION_EXPRESSION_HH_
#define _SDD_HOM_EXPRESSION_EXPRESSION_HH_

#include <algorithm> // any_of, find
#include <cassert>
#include <vector>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/expression/evaluator.hh"
#include "sdd/hom/expression/stacks.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom { namespace expr {

template <typename C>
using coro = boost::coroutines::coroutine<SDD<C>()>;

/*------------------------------------------------------------------------------------------------*/

template <typename C>
using positions_iterator = typename std::vector<typename order<C>::position_type>::const_iterator;

template <typename C>
void
expression_post( typename coro<C>::caller_type& yield
               , SDD<C> x, const order<C>& o
               , typename order<C>::position_type target
               , typename C::Values* valuation
               , const std::shared_ptr<app_stack<C>>& app, const std::shared_ptr<sdd_stack<C>>& res
               , positions_iterator<C> cit, const positions_iterator<C>& end
               , evaluator_base<C>* eval)
{
  namespace ph = std::placeholders;

  static void* table[4] = {&&zero, &&one, &&flat, &&hierarchical};
  goto *table[x.index()];

  /*-----------------------*/
  hierarchical:
  {
    const auto& node = sdd::mem::variant_cast<sdd::hierarchical_node<C>>(*x);

    for (const auto& arc : node)
    {
      const auto local_res = std::make_shared<sdd_stack<C>>(arc.successor(), res);
      const auto local_app = std::make_shared<app_stack<C>>(arc.successor(), o.next(), app);
      coro<C> gen(std::bind( expression_post<C>, ph::_1, arc.valuation(), o.nested(), target
                           , valuation, local_app, local_res, cit, std::cref(end), eval));
      while (gen)
      {
        yield(SDD<C>(o.variable(), gen.get(), local_res->sdd));
        gen();
      }
    }
    return;
  }

  /*-----------------------*/
  flat:
  {
    const bool target_level = o.position() == target;
    const bool update_values = std::find(cit, end, o.position()) != end;

    if (update_values)
    {
      // Narrow the range for future searches of identifiers.
      std::advance(cit, 1);
    }

    const auto& node = sdd::mem::variant_cast<sdd::flat_node<C>>(*x);

    if (cit == end) // Last level, avoid to propagate a new coroutine.
    {
      for (const auto& arc : node)
      {
        if (update_values)
        {
          eval->update(o.identifier(), arc.valuation());
        }
        *valuation = eval->evaluate();
        yield(SDD<C>(o.variable(), arc.valuation(), arc.successor()));
      }
      return;
    }

    for (const auto& arc : node)
    {
      if (update_values)
      {
        eval->update(o.identifier(), arc.valuation());
      }

      coro<C> gen(std::bind( expression_post<C>, ph::_1, arc.successor(), o.next(), target
                           , valuation, app, res, cit, std::cref(end), eval));

      while (gen)
      {
        if (target_level)
        {
          *valuation = eval->evaluate();
          yield(SDD<C>(o.variable(), *valuation, gen.get()));
        }
        else
        {
          yield(SDD<C>(o.variable(), arc.valuation(), gen.get()));
        }
        gen();
      }
    }
    return;
  }

  /*-----------------------*/
  one:
  {
    // We are in a nested hierarchy, we now propagate to the successor of the upper level.
    // We can't arrive here when app is not set, as the flat case ensure that we don't propagate
    // on the final |1| (it avoids propagation whenever all operands have been encoutered).
    assert(app);
    coro<C> gen(std::bind( expression_post<C>, ph::_1, app->sdd, app->ord, target, valuation
                         , app->next, res->next, cit, std::cref(end), eval));
    while (gen)
    {
      res->sdd = gen.get();
      yield(one<C>());
      gen();
    }
    return;
  }

  /*-----------------------*/
  zero:
  {
    assert(false);
    __builtin_unreachable();
  }
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief
template <typename C>
struct expression_pre
{
  /// @brief Needed by mem::variant.
  using result_type = SDD<C>;

  /// @brief A variable type.
  using variable_type = typename C::Variable;

  /// @brief An identifier type.
  using position_type = typename order<C>::position_type;

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief The type of a set of variables.
  using positions_type = std::vector<position_type>;

  /// @brief An iterator on a set of identifiers.
  using positions_iterator = typename positions_type::const_iterator;

  /// @brief The evaluation's context.
  context<C>& cxt_;

  /// @brief The target of the evaluated expression.
  const position_type target_;

  /// @brief User evaluator of the expression.
  evaluator_base<C>& eval_;

  /// @brief Placeholder for computed valuations on each distinct path.
  mutable values_type valuation_;

  /// @brief Constructor.
  expression_pre(context<C>& cxt, position_type target, evaluator_base<C>& eval)
    : cxt_(cxt), target_(target), eval_(eval), valuation_()
  {}

  /// @brief Evaluation on hierarchical nodes.
  SDD<C>
  operator()( const hierarchical_node<C>& node
            , const order<C>& o
            , const std::shared_ptr<app_stack<C>>& app, const std::shared_ptr<res_stack<C>>& res
            , positions_iterator cit, positions_iterator end)
  const
  {
    // Shortcut to the SDD evaluation context.
    auto& sdd_cxt = cxt_.sdd_context();

    if (not o.contains(o.position(), target_)) // target is not in the nested hierarchy
    {
      // Check if the nested levels contain any of the variables needed to update the evaluator.
      const bool nested_variables = std::any_of( cit, end
                                               , [&](position_type pos)
                                                    {return o.contains(o.position(), pos);});

      if (not nested_variables)
      {
        // We have no interest in this level, thus the visitor is propagated to the next level.
        dd::square_union<C, SDD<C>> su;
        su.reserve(node.size());
        for (const auto& arc : node)
        {
          const SDD<C> successor = visit(*this, arc.successor(), o.next(), app, res, cit, end);
          su.add(successor, arc.valuation());
        }
        return SDD<C>(o.variable(), su(sdd_cxt));
      }
      else
      {
        // We are interested in this level, but the target is not nested into it. Thus, we won't
        // modiffy the current level: a square union is sufficient.
        dd::square_union<C, SDD<C>> su;
        su.reserve(node.size());
        for (const auto& arc : node)
        {
          const auto local_app = std::make_shared<app_stack<C>>(arc.successor(), o.next(), app);
          const auto local_res = std::make_shared<res_stack<C>>(res);
          visit(*this, arc.valuation(), o.nested(), local_app, local_res, cit, end);
          assert(not local_res->result.empty() && "Invalid empty successor result");
          su.add(dd::sum<C>(sdd_cxt, std::move(local_res->result)), arc.valuation());
        }
        return SDD<C>(o.variable(), su(sdd_cxt));
      }
    }
    else // target is contained in this hierarchy
    {
      namespace ph = std::placeholders;
      dd::sum_builder<C, SDD<C>> operands(node.size());

      for (const auto& arc : node)
      {
        const auto local_res = std::make_shared<sdd_stack<C>>(arc.successor(), nullptr);
        const auto local_app = std::make_shared<app_stack<C>>(arc.successor(), o.next(), nullptr);
        coro<C> gen(std::bind( expression_post<C>, ph::_1, arc.valuation(), o.nested(), target_
                             , &valuation_, local_app, local_res, cit, std::cref(end), &eval_));
        while(gen)
        {
          assert(not local_res->sdd.empty() && "Invalid |0| successor result");
          operands.add(SDD<C>(o.variable(), /*nested*/ gen.get(), /*successor*/ local_res->sdd));
          gen();
        }
      }
      return dd::sum<C>(sdd_cxt, std::move(operands));
    }
  }

  /// @brief Evaluation on flat nodes.
  SDD<C>
  operator()( const flat_node<C>& node
            , const order<C>& o
            , const std::shared_ptr<app_stack<C>>& app, const std::shared_ptr<res_stack<C>>& res
            , positions_iterator cit, positions_iterator end)
  const
  {
    auto& sdd_cxt = cxt_.sdd_context();
    const bool update_values = std::find(cit, end, o.position()) != end;

    if (update_values)
    {
      // Narrow the range for future searches of identifiers.
      std::advance(cit, 1);
    }

    if (o.position() == target_)
    {
      namespace ph = std::placeholders;
      dd::sum_builder<C, SDD<C>> operands(node.size());

      for (const auto& arc : node)
      {
        if (update_values)
        {
          eval_.update(o.identifier(), arc.valuation());
        }

        coro<C> gen(std::bind( expression_post<C>, ph::_1, arc.successor(), o.next(), target_
                             , &valuation_, nullptr, nullptr, cit, std::cref(end), &eval_));

        while (gen)
        {
          // valuation_ has been modified in the meantime
          operands.add(SDD<C>(o.variable(), valuation_, gen.get()));
          gen();
        }
      }
      return dd::sum<C>(sdd_cxt, std::move(operands));
    }
    else // target is still below
    {
      // We don't modify the current level, a square union is sufficient
      dd::square_union<C, values_type> su;
      su.reserve(node.size());
      for (const auto& arc : node)
      {
        if (update_values)
        {
          eval_.update(o.identifier(), arc.valuation());
        }
        const SDD<C> successor = visit(*this, arc.successor(), o.next(), app, res, cit, end);
        su.add(successor, arc.valuation());
      }
      return SDD<C>(o.variable(), su(cxt_.sdd_context()));
    }
  }

  /// @brief Evaluation on |1|.
  SDD<C>
  operator()( const one_terminal<C>&
            , const order<C>& o
            , const std::shared_ptr<app_stack<C>>& app, const std::shared_ptr<res_stack<C>>& res
            , positions_iterator cit, positions_iterator end)
  const
  {
    // Continue to the stacked successor of a previously visited hierachical node.
    const auto stacked_succ = visit(*this, app->sdd, app->ord, app->next, res->next, cit, end);
    res->result.add(stacked_succ);
    return sdd::one<C>();
  }
  
  /// @brief Evaluation on |0|.
  ///
  /// Should never happen.
  SDD<C>
  operator()( const zero_terminal<C>&
            , const order<C>&
            , const std::shared_ptr<app_stack<C>>&, const std::shared_ptr<res_stack<C>>&
            , positions_iterator, positions_iterator)
  const noexcept
  {
    assert(false);
    __builtin_unreachable();
  }
}; // struct expression_pre

/*------------------------------------------------------------------------------------------------*/

}}} // namespace sdd::hom::expr

#endif // _SDD_HOM_EXPRESSION_EXPRESSION_HH_
