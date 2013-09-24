#ifndef _SDD_HOM_EXPRESSION_SIMPLE_HH_
#define _SDD_HOM_EXPRESSION_SIMPLE_HH_

#include <algorithm> // any_of, copy, find
#include <cassert>
#include <utility>   // pair
#include <vector>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/expression/evaluator.hh"
#include "sdd/hom/expression/stacks.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom { namespace expr {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief
template <typename C>
struct simple
{
  /// @brief Needed by mem::variant.
  using result_type = SDD<C>;

  /// @brief A variable type.
  using variable_type = typename C::Variable;

  /// @brief An identifier type.
  using identifier_type = typename C::Identifier;

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief The type of a set of variables.
  using identifiers_type = std::vector<identifier_type>;

  /// @brief An iterator on a set of identifiers.
  using identifiers_iterator = typename identifiers_type::const_iterator;

  /// @brief The evaluation's context.
  context<C>& cxt_;

  /// @brief The target of the evaluated expression.
  const identifier_type& target_;

  /// @brief User evaluator of the expression.
  evaluator_base<C>& eval_;

  /// @brief Constructor.
  simple(context<C>& cxt, const identifier_type& target, evaluator_base<C>& eval)
    : cxt_(cxt), target_(target), eval_(eval)
  {}

  /// @brief Evaluation on hierarchical nodes.
  SDD<C>
  operator()( const hierarchical_node<C>& node
            , const order<C>& o
            , const std::shared_ptr<app_stack<C>>& app, const std::shared_ptr<res_stack<C>>& res
            , identifiers_iterator cit, identifiers_iterator end)
  const
  {
    auto& sdd_cxt = cxt_.sdd_context();

    if (not o.contains(o.identifier(), target_)) // not the last level?
    {
      const bool nested_variables = std::any_of( cit, end
                                               , [&](const identifier_type& id)
                                                    {
                                                      return o.contains(o.identifier(), id);
                                                    });

      if (not nested_variables)
      {
        // We are not interested in this level, thus the visitor is propagated to the next level.
        dd::square_union<C, SDD<C>> su;
        su.reserve(node.size());
        for (const auto& arc : node)
        {
          const SDD<C> successor = visit(*this, arc.successor(), o.next(), app, res, cit, end);
          su.add(successor, arc.valuation());
        }
        return SDD<C>(o.variable(), su(cxt_.sdd_context()));
      }
      else
      {
        // We are interested in this level, but the target is not nested into it. Thus, we must
        // propagate on both nesteds SDD and successors.
        dd::sum_builder<C, SDD<C>> operands(node.size());
        for (const auto& arc : node)
        {
          // push on stacks
          const auto local_app = std::make_shared<app_stack<C>>(arc.successor(), o.next(), app);
          const auto local_res = std::make_shared<res_stack<C>>(res);

          const auto nested = visit( *this, arc.valuation(), o.nested(), local_app, local_res
                                   , cit, end);

          assert(not local_res->result.empty() && "Invalid empty successor result");
          operands.add( SDD<C>( o.variable(), nested
                              , dd::sum<C>(sdd_cxt, std::move(local_res->result))));
        }
        return dd::sum<C>(sdd_cxt, std::move(operands));
      }
    }
    else
    {
      // Final level, we just need to propagate in the nested SDD.
      dd::sum_builder<C, SDD<C>> operands(node.size());
      for (const auto& arc : node)
      {
        const auto nested = visit(*this, arc.valuation(), o.nested(), app, res, cit, end);
        operands.add(SDD<C>(o.variable(), nested, arc.successor()));
      }
      return dd::sum<C>(sdd_cxt, std::move(operands));
    }
  }

  /// @brief Evaluation on flat nodes.
  SDD<C>
  operator()( const flat_node<C>& node
            , const order<C>& o
            , const std::shared_ptr<app_stack<C>>& app, const std::shared_ptr<res_stack<C>>& res
            , identifiers_iterator cit, identifiers_iterator end)
  const
  {
    auto& sdd_cxt = cxt_.sdd_context();
    const bool last_level = o.identifier() == target_;
    const bool update_values = std::find(cit, end, o.identifier()) != end;

    if (update_values)
    {
      // Narrow the range for future searches of identifiers.
      cit = std::next(cit);
    }

    if (last_level)
    {
      dd::sum_builder<C, SDD<C>> operands(node.size());
      for (const auto& arc : node)
      {
        if (update_values)
        {
          eval_.update(o.identifier(), arc.valuation());
        }
        operands.add(SDD<C>(o.variable(), eval_.evaluate(), arc.successor()));
      }
      return dd::sum<C>(sdd_cxt, std::move(operands));
    }
    else
    {
      // Not the last level, we just need to update values if necessary and to propagate on
      // successors.
      dd::square_union<C, values_type> su;
      for (const auto& arc : node)
      {
        if (update_values)
        {
          eval_.update(o.identifier(), arc.valuation());
        }
        const auto successor = visit(*this, arc.successor(), o.next(), app, res, cit, end);
        su.add(successor, arc.valuation());
      }
      return SDD<C>(o.variable(), su(sdd_cxt));
    }
  }

  /// @brief Evaluation on |1|.
  SDD<C>
  operator()( const one_terminal<C>&
            , const order<C>& o
            , const std::shared_ptr<app_stack<C>>& app, const std::shared_ptr<res_stack<C>>& res
            , identifiers_iterator cit, identifiers_iterator end)
  const
  {
    assert(app && "Target never encountered while evaluating SimpleExpression");
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
            , identifiers_iterator, identifiers_iterator)
  const noexcept
  {
    assert(false);
    __builtin_unreachable();
  }
}; // struct simple

/*------------------------------------------------------------------------------------------------*/

}}} // namespace sdd::hom::expr

#endif // _SDD_HOM_EXPRESSION_SIMPLE_HH_
