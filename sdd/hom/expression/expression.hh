#pragma once

#include <algorithm> // any_of, find
#include <cassert>
#include <vector>

#include <boost/version.hpp>
#if BOOST_VERSION >= 105600
#include <boost/coroutine/asymmetric_coroutine.hpp>
#else
#include <boost/coroutine/all.hpp>
#endif

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/expression/evaluator.hh"
#include "sdd/hom/expression/stacks.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom { namespace expr {

/*------------------------------------------------------------------------------------------------*/

namespace bcoro = boost::coroutines;

/// @internal
/// @brief The signature of the coroutine used to implement the expression evaluation.
template <typename C>
#if BOOST_VERSION >= 105600
using coro = typename boost::coroutines::asymmetric_coroutine<SDD<C>>::pull_type;
#else
using coro = typename boost::coroutines::coroutine<SDD<C>>::pull_type;
#endif

/// @internal
template <typename C>
#if BOOST_VERSION >= 105600
using yield_type = typename boost::coroutines::asymmetric_coroutine<SDD<C>>::push_type;
#else
using yield_type = typename boost::coroutines::coroutine<SDD<C>>::push_type;
#endif

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <bool>
struct coro_fpu;

/// @internal
template <>
struct coro_fpu<true>
{
  static constexpr auto value = bcoro::fpu_preserved;
};

/// @internal
template <>
struct coro_fpu<false>
{
  static constexpr auto value = bcoro::fpu_not_preserved;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct expression_post_visitor;

/// @internal
template <typename C>
void
expression_post( yield_type<C>& yield, expression_post_visitor<C>& v, const SDD<C>& s
               , const order<C>& o, const std::shared_ptr<app_stack<C>>& app
               , const std::shared_ptr<sdd_stack<C>>& res
               , order_positions_iterator cit, order_positions_iterator end);

/// @internal
/// @brief Handle the expression evaluation below the target.
template <typename C>
struct expression_post_visitor
{
  evaluator_base<C>& eval;
  typename C::Values& valuation;
  const order_position_type target;

  void
  operator()( const hierarchical_node<C>& n
            , yield_type<C>& yield
            , const order<C>& o
            , const std::shared_ptr<app_stack<C>>& app, const std::shared_ptr<sdd_stack<C>>& res
            , order_positions_iterator cit, order_positions_iterator end)
  const
  {
    namespace ph = std::placeholders;
    for (const auto& arc : n)
    {
      const auto local_res = std::make_shared<sdd_stack<C>>(arc.successor(), res);
      const auto local_app = std::make_shared<app_stack<C>>(arc.successor(), o.next(), app);
      coro<C> gen( std::bind( expression_post<C>, ph::_1, *this, arc.valuation(), o.nested()
                            , local_app, local_res, cit, end)
                 , bcoro::attributes(coro_fpu<C::expression_preserve_fpu_registers>::value));
      while (gen)
      {
        yield(SDD<C>(o.variable(), gen.get(), local_res->sdd));
        gen();
      }
    }
  }

  void
  operator()( const flat_node<C>& n
            , yield_type<C>& yield
            , const order<C>& o
            , const std::shared_ptr<app_stack<C>>& app, const std::shared_ptr<sdd_stack<C>>& res
            , order_positions_iterator cit, order_positions_iterator end)
  const
  {
    namespace ph = std::placeholders;
    const bool target_level = o.position() == target;
    const bool update_values = std::find(cit, end, o.position()) != end;

    if (update_values)
    {
      std::advance(cit, 1); // Narrow the range for future searches of identifiers.
    }

    if (cit == end) // Last level, avoid to propagate a new coroutine.
    {
      for (const auto& arc : n)
      {
        if (update_values)
        {
          eval.update(o.identifier(), arc.valuation());
        }
        valuation = eval.evaluate();
        yield(SDD<C>(o.variable(), arc.valuation(), arc.successor()));
      }
    }
    else // cit != end
    {
      for (const auto& arc : n)
      {
        if (update_values)
        {
          eval.update(o.identifier(), arc.valuation());
        }

        coro<C> gen( std::bind( expression_post<C>, ph::_1, *this, arc.successor(), o.next()
                              , app, res, cit, end)
                   , bcoro::attributes(coro_fpu<C::expression_preserve_fpu_registers>::value));
        while (gen)
        {
          if (target_level)
          {
            valuation = eval.evaluate();
            yield(SDD<C>(o.variable(), valuation, gen.get()));
          }
          else
          {
            yield(SDD<C>(o.variable(), arc.valuation(), gen.get()));
          }
          gen();
        }
      }
    }
  }

  void
  operator()( const one_terminal<C>&
            , yield_type<C>& yield
            , const order<C>&
            , const std::shared_ptr<app_stack<C>>& app, const std::shared_ptr<sdd_stack<C>>& res
            , order_positions_iterator cit, order_positions_iterator end)
  const
  {
    namespace ph = std::placeholders;
    // We are in a nested hierarchy, we now propagate to the successor of the upper level.
    // We can't arrive here when app is not set, as the flat case ensure that we don't propagate
    // on the final |1| (it avoids propagation whenever all operands have been encoutered).
    assert(app);
    coro<C> gen( std::bind( expression_post<C>, ph::_1, *this, app->sdd, app->ord
                          , app->next, res->next, cit, end)
               , bcoro::attributes(coro_fpu<C::expression_preserve_fpu_registers>::value));
    while (gen)
    {
      res->sdd = gen.get();
      yield(one<C>());
      gen();
    }
  }

  void
  operator()( const zero_terminal<C>&
            , yield_type<C>&
            , const order<C>&
            , const std::shared_ptr<app_stack<C>>&, const std::shared_ptr<sdd_stack<C>>&
            , order_positions_iterator, order_positions_iterator)
  const
  {
    assert(false);
    __builtin_unreachable();
  }
};

/// @internal
template <typename C>
inline
void
expression_post( yield_type<C>& yield, expression_post_visitor<C>& v, const SDD<C>& s
               , const order<C>& o, const std::shared_ptr<app_stack<C>>& app
               , const std::shared_ptr<sdd_stack<C>>& res
               , order_positions_iterator cit, order_positions_iterator end)
{
  visit(v, /* visited */s, yield, o, app, res, cit, end);
}


/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief
template <typename C>
struct expression_pre
{
  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief The evaluation's context.
  context<C>& cxt_;

  /// @brief The target of the evaluated expression.
  const order_position_type target_;

  /// @brief User evaluator of the expression.
  evaluator_base<C>& eval_;

  /// @brief Placeholder for computed valuations on each distinct path.
  mutable values_type valuation_;

  /// @brief Constructor.
  expression_pre(context<C>& cxt, order_position_type target, evaluator_base<C>& eval)
    : cxt_(cxt), target_(target), eval_(eval), valuation_()
  {}

  /// @brief Evaluation on hierarchical nodes.
  SDD<C>
  operator()( const hierarchical_node<C>& node, const order<C>& o
            , const std::shared_ptr<app_stack<C>>& app, const std::shared_ptr<res_stack<C>>& res
            , order_positions_iterator cit, order_positions_iterator end)
  const
  {
    // Shortcut to the SDD evaluation context.
    auto& sdd_cxt = cxt_.sdd_context();

    if (not o.contains(o.position(), target_)) // target is not in the nested hierarchy
    {
      // Check if the nested levels contain any of the variables needed to update the evaluator.
      const bool nested_variables = std::any_of( cit, end
                                               , [&](order_position_type pos)
                                                    {return o.contains(o.position(), pos);});

      if (not nested_variables)
      {
        // We have no interest in this level, thus the visitor is propagated to the next level.
        dd::square_union<C, SDD<C>> su(sdd_cxt);
        su.reserve(node.size());
        for (const auto& arc : node)
        {
          const SDD<C> successor = visit(*this, arc.successor(), o.next(), app, res, cit, end);
          su.add(successor, arc.valuation());
        }
        return SDD<C>(o.variable(), su());
      }
      else
      {
        // We are interested in this level, but the target is not nested into it. Thus, we won't
        // modiffy the current level: a square union is sufficient.
        dd::square_union<C, SDD<C>> su(sdd_cxt);
        su.reserve(node.size());
        for (const auto& arc : node)
        {
          const auto local_app = std::make_shared<app_stack<C>>(arc.successor(), o.next(), app);
          const auto local_res = std::make_shared<res_stack<C>>(sdd_cxt, res);
          visit(*this, arc.valuation(), o.nested(), local_app, local_res, cit, end);
          assert(not local_res->result.empty() && "Invalid empty successor result");
          su.add(dd::sum<C>(sdd_cxt, std::move(local_res->result)), arc.valuation());
        }
        return SDD<C>(o.variable(), su());
      }
    }
    else // target is contained in this hierarchy
    {
      namespace ph = std::placeholders;
      dd::sum_builder<C, SDD<C>> operands(sdd_cxt);
      operands.reserve(node.size());

      for (const auto& arc : node)
      {
        const auto local_res = std::make_shared<sdd_stack<C>>(arc.successor(), nullptr);
        const auto local_app = std::make_shared<app_stack<C>>(arc.successor(), o.next(), nullptr);
        coro<C> gen( std::bind( expression_post<C>, ph::_1
                              , expression_post_visitor<C>{eval_, valuation_, target_}
                              , arc.valuation(), o.nested(), local_app, local_res, cit, end)
                   , bcoro::attributes(coro_fpu<C::expression_preserve_fpu_registers>::value));
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
  operator()( const flat_node<C>& node, const order<C>& o
            , const std::shared_ptr<app_stack<C>>& app, const std::shared_ptr<res_stack<C>>& res
            , order_positions_iterator cit, order_positions_iterator end)
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
      dd::sum_builder<C, SDD<C>> operands(sdd_cxt);
      operands.reserve(node.size());

      for (const auto& arc : node)
      {
        if (update_values)
        {
          eval_.update(o.identifier(), arc.valuation());
        }

        coro<C> gen( std::bind( expression_post<C>, ph::_1
                              , expression_post_visitor<C>{eval_, valuation_, target_}
                              , arc.successor(), o.next(), nullptr, nullptr, cit, end)
                   , bcoro::attributes(coro_fpu<C::expression_preserve_fpu_registers>::value));

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
      dd::square_union<C, values_type> su(sdd_cxt);
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
      return SDD<C>(o.variable(), su());
    }
  }

  /// @brief Evaluation on |1|.
  SDD<C>
  operator()( const one_terminal<C>&, const order<C>&
            , const std::shared_ptr<app_stack<C>>& app, const std::shared_ptr<res_stack<C>>& res
            , order_positions_iterator cit, order_positions_iterator end)
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
  operator()( const zero_terminal<C>&, const order<C>&
            , const std::shared_ptr<app_stack<C>>&, const std::shared_ptr<res_stack<C>>&
            , order_positions_iterator, order_positions_iterator)
  const noexcept
  {
    assert(false);
    __builtin_unreachable();
  }
}; // struct expression_pre

/*------------------------------------------------------------------------------------------------*/

}}} // namespace sdd::hom::expr
