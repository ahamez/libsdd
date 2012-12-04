#ifndef _SDD_HOM_EXPRESSION_HH_
#define _SDD_HOM_EXPRESSION_HH_

#include <algorithm> // any_of, copy, find
#include <cassert>
#include <iosfwd>
#include <memory>    // make_shared, shared_ptr
#include <stdexcept> // invalid_argument
#include <typeinfo>  // typeid
#include <vector>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation_error.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief
template <typename C>
class evaluator_base
{
public:

  typedef typename C::Identifier identifier_type;
  typedef typename C::Values     values_type;

  virtual
  ~evaluator_base()
  {
  }

  ///
  virtual
  void
  update(const identifier_type&, const values_type&) = 0;

  ///
  virtual
  values_type
  evaluate() = 0;

  /// @brief Compare evaluator_base.
  virtual
  bool
  operator==(const evaluator_base&) const noexcept = 0;

  /// @brief Get the user's evaluator hash value.
  virtual
  std::size_t
  hash() const noexcept = 0;

  /// @brief Get the user's evaluator textual representation.
  virtual
  void
  print(std::ostream&) const = 0;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Used to wrap user's evaluator.
template <typename C, typename User>
class evaluator_derived
  : public evaluator_base<C>
{
private:

  /// @brief The user's evaluator.
  User eval_;

public:

  /// @brief The type of a variable.
  typedef typename C::Identifier identifier_type;

  /// @brief The type of a set of values.
  typedef typename C::Values values_type;

  /// @brief Constructor.
  evaluator_derived(const User& eval)
    : eval_(eval)
  {
  }

  ///
  void
  update(const identifier_type& id, const values_type& values)
  {
    eval_.update(id, values);
  }

  ///
  values_type
  evaluate()
  {
    return eval_.evaluate();
  }

  /// @brief Compare evaluator_derived.
  bool
  operator==(const evaluator_base<C>& other)
  const noexcept
  {
    return typeid(*this) == typeid(other)
         ? eval_ == reinterpret_cast<const evaluator_derived&>(other).eval_
         : false
         ;
  }

  /// @brief Get the user's evaluator hash value.
  std::size_t
  hash()
  const noexcept
  {
    return std::hash<User>()(eval_);
  }

  /// @brief Get the user's evaluator textual representation.
  void
  print(std::ostream& os)
  const
  {
    os << eval_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Expression homomorphism.
template <typename C>
class expression
{
public:

  /// @brief A variable type.
  typedef typename C::Variable variable_type;

  /// @brief An identifier type.
  typedef typename C::Identifier identifier_type;

  /// @brief The type of a set of values.
  typedef typename C::Values values_type;

  /// @brief The type of a set of variables.
  typedef std::vector<identifier_type> identifiers_type;

private:

  /// @brief Ownership of the user's evaluator.
  const std::shared_ptr<evaluator_base<C>> eval_ptr_;

  /// @brief The set of the expression's variables.
  const identifiers_type identifiers_;

  /// @brief The target of the assignment.
  const identifier_type target_;

  /// @brief An iterator on a set of identifiers.
  typedef typename identifiers_type::const_iterator identifiers_iterator;

  struct application_stack_node;

  /// @brief A pointer to a stack of successors.
  typedef std::shared_ptr<application_stack_node> application_stack;

  /// @brief A stack of successors to apply visitor on.
  struct application_stack_node
  {
    SDD<C> sdd;
    const order<C> ord;
    application_stack next;

    application_stack_node(const SDD<C>& s, const order<C>& o, const application_stack& n)
      : sdd(s)
      , ord(o)
      , next(n)
    {
    }
  };

  struct results_stack_node;

  /// @brief A pointer to a stack of results.
  typedef std::shared_ptr<results_stack_node> results_stack;

  /// @brief A stack of results to create successors of hierarchical nodes.
  struct results_stack_node
  {
    dd::sum_builder<C, SDD<C>> result;
    results_stack next;

    results_stack_node(const results_stack& n)
      : result()
      , next(n)
    {
    }
  };

  /// @brief Evaluation of the expression after the target is found.
  struct post
  {
    /// @brief Needed by variant.
    typedef void result_type;

    /// @brief Shared pointer to the user evaluation function.
    const std::shared_ptr<evaluator_base<C>> eval_ptr_;

    /// @brief Constructor.
    post(std::shared_ptr<evaluator_base<C>> eval_ptr)
      : eval_ptr_(eval_ptr)
    {
    }

    /// @brief Evaluation on hierarchical nodes.
    void
    operator()( const hierarchical_node<C>& node, const order<C>& o
              , application_stack app, values_type& valuation
              , identifiers_iterator begin, identifiers_iterator end)
    const noexcept
    {
      if (begin == end)
      {
        valuation = values::sum(valuation, eval_ptr_->evaluate());
        return;
      }

      // Are there some values of interest in the current hierarchy?
      const bool interested = std::any_of( begin, end
                                         , [&](const identifier_type& id)
                                              {
                                                return o.contains(o.identifier(), id);
                                              });

      if (interested)
      {
        for (const auto& arc : node)
        {
          // push on top of stack
          app = std::make_shared<application_stack_node>(arc.successor(), o.next(), app);
          apply_visitor(*this, arc.valuation()->data(), o.nested(), app, valuation, begin, end);
        }
      }
      else
      {
        for (const auto& arc : node)
        {
          apply_visitor(*this, arc.successor()->data(), o.next(), app, valuation, begin, end);
        }
      }
    }

    /// @brief Evaluation on flat nodes.
    void
    operator()( const flat_node<C>& node, const order<C>& o
              , application_stack app, values_type& valuation
              , identifiers_iterator begin, identifiers_iterator end)
    const noexcept
    {
      if (begin == end)
      {
        valuation = values::sum(valuation, eval_ptr_->evaluate());
        return;
      }

      const bool update = std::find(begin, end, o.identifier()) != end;

      if (update)
      {
        begin = std::next(begin);
      }

      for (const auto& arc : node)
      {
        if (update)
        {
          eval_ptr_->update(o.identifier(), arc.valuation());
        }
        apply_visitor(*this, arc.successor()->data(), o.next(), app, valuation, begin, end);
      }
    }

    /// @brief Evaluation on |1|.
    void
    operator()( const one_terminal<C>&, const order<C>&
              , application_stack app, values_type& valuation
              , identifiers_iterator begin, identifiers_iterator end)
    const noexcept
    {
      if (begin == end)
      {
        valuation = values::sum(valuation, eval_ptr_->evaluate());
      }
      else if (app)
      {
        // Still in a nested hierarchy.
        apply_visitor(*this, app->sdd->data(), app->ord, app->next, valuation, begin, end);
      }
    }

    /// @brief Evaluation on |0|.
    void
    operator()( const zero_terminal<C>&, const order<C>&, application_stack
              , values_type&, identifiers_iterator, identifiers_iterator)
    const noexcept
    {
      assert(false);
      __builtin_unreachable();
    }
  }; // struct post

  /// @brief Evaluation of the expression until the target is found.
  struct pre
  {
    /// @brief Needed by variant.
    typedef SDD<C> result_type;

    ///
    const identifier_type& target_;

    /// @brief Shared pointer to the user evaluation function.
    const std::shared_ptr<evaluator_base<C>> eval_ptr_;

    /// @brief Avoid to construct several time the same visitor.
    const post post_;

    /// @brief Constructor.
    pre(const identifier_type& target, std::shared_ptr<evaluator_base<C>> eval_ptr)
      : target_(target)
      , eval_ptr_(eval_ptr)
      , post_(eval_ptr_)
    {
    }

    /// @brief Evaluation on hierarchical nodes.
    SDD<C>
    operator()( const hierarchical_node<C>& node
              , context<C>& cxt, const order<C>& o, const SDD<C>& s
              , application_stack app, results_stack res
              , identifiers_iterator begin, identifiers_iterator end)
    const
    {
      // Is the target nested in the current hierarchy?
      if (o.contains(o.identifier(), target_))
      {
        dd::sum_builder<C, SDD<C>> operands(node.size());

        for (const auto& arc : node)
        {
          // push on top of stacks
          app = std::make_shared<application_stack_node>(arc.successor(), o.next(), app);
          res = std::make_shared<results_stack_node>(res);

          const SDD<C> nested = apply_visitor( *this, arc.valuation()->data()
                                             , cxt, o.nested(), arc.valuation(), app, res
                                             , begin, end);

          assert(not res->result.empty() && "Invalid result");
          operands.add( SDD<C>(o.variable(), nested
                      , dd::sum<C>(cxt.sdd_context(), std::move(res->result))));
        }

        return dd::sum<C>(cxt.sdd_context(), std::move(operands));
      }

      // Are there some values of interest in the current hierarchy?
      const bool interested = std::any_of( begin, end
                                         , [&](const identifier_type& id)
                                              {
                                                return o.contains(o.identifier(), id);
                                              });
      if (interested)
      {
        dd::square_union<C, SDD<C>> su;
        su.reserve(node.size());
        
        for (const auto& arc : node)
        {
          // push on top of stacks
          app = std::make_shared<application_stack_node>(arc.successor(), o.next(), app);
          res = std::make_shared<results_stack_node>(res);

          const SDD<C> nested = apply_visitor( *this, arc.valuation()->data()
                                             , cxt, o.nested(), arc.valuation(), app, res
                                             , begin, end);

          assert(not res->result.empty() && "Invalid result");
          su.add(dd::sum<C>(cxt.sdd_context(), std::move(res->result)), nested);
        }

        return SDD<C>(o.variable(), su(cxt.sdd_context()));
      }

      // We are not interested in this level, thus the visitor is propagated to the next level.
      dd::square_union<C, SDD<C>> su;
      su.reserve(node.size());
      for (const auto& arc : node)
      {
        const SDD<C> successor = apply_visitor( *this, arc.successor()->data()
                                              , cxt, o.next(), arc.successor(), app, res
                                              , begin, end);
        su.add(successor, arc.valuation());
      }
      return SDD<C>(o.variable(), su(cxt.sdd_context()));
    }

    /// @brief Evaluation on flat nodes.
    SDD<C>
    operator()( const flat_node<C>& node
              , context<C>& cxt, const order<C>& o, const SDD<C>& s
              , application_stack app, results_stack res
              , identifiers_iterator begin, identifiers_iterator end)
    const
    {
      const bool update_values = std::find(begin, end, o.identifier()) != end;

      // Factorize call to user's evaluation function.
      const auto update = [&](const values_type& values)
                             {
                               if (update_values)
                               {
                                 eval_ptr_->update(o.identifier(), values);
                               }
                             };

      if (update_values)
      {
        // If we have to update the values, it means that the current identifier is part of the
        // expression to evaluate.
        begin = std::next(begin);
      }

      if (o.identifier() == target_)
      {
        // The target of the expression is reached, a different visitor is needed.

        dd::sum_builder<C, SDD<C>> operands(node.size());

        for (const auto& arc : node)
        {
          update(arc.valuation());
          values_type valuation;
          apply_visitor(post_, arc.successor()->data(), o.next(), app, valuation, begin, end);
          operands.add(SDD<C>(o.variable(), std::move(valuation), arc.successor()));
        }

        return dd::sum(cxt.sdd_context(), std::move(operands));
      }

      if (o.same_hierarchy(o.identifier(), target_))
      {
        // The current identifier is above the target in the same hierarchy.

        dd::square_union<C, values_type> su;
        su.reserve(node.size());

        for (const auto& arc : node)
        {
          update(arc.valuation());
          const SDD<C> next = apply_visitor( *this, arc.successor()->data()
                                           , cxt, o.next(), arc.successor(), app, res, begin, end);
          su.add(next, arc.valuation());
        }

        return SDD<C>(o.variable(), su(cxt.sdd_context()));
      }

      // Read-only, the identifier variable is above the target, but in a different hierarchy.

      for (const auto& arc : node)
      {
        update(arc.valuation());
        apply_visitor( *this, arc.successor()->data()
                     , cxt, o.next(), arc.successor(), app, res, begin, end);
      }
      return s;
    }

    /// @brief Evaluation on |1|.
    SDD<C>
    operator()( const one_terminal<C>&
              , context<C>& cxt, const order<C>&, const SDD<C>& one
              , application_stack app, results_stack res
              , identifiers_iterator begin, identifiers_iterator end)
    const
    {
      if (app)
      {
        assert(res && "Empty res for a non-empty app.");

        // Still in a nested hierarchy.
        res->result.add(apply_visitor( *this, app->sdd->data()
                                     , cxt, app->ord, app->sdd, app->next, res->next
                                     , begin, end));
      }
      return one;
    }
    
    /// @brief Evaluation on |0|.
    SDD<C>
    operator()( const zero_terminal<C>&
              , context<C>&, const order<C>&, const SDD<C>&
              , application_stack, results_stack
              , identifiers_iterator, identifiers_iterator)
    const noexcept
    {
      assert(false);
      __builtin_unreachable();
    }

  }; // struct pre

public:

  /// @brief Constructor.
  expression( std::shared_ptr<evaluator_base<C>> e_ptr, identifiers_type&& identifiers
            , const identifier_type& target)
    : eval_ptr_(e_ptr)
    , identifiers_(std::move(identifiers))
    , target_(target)
  {
  }

  /// @brief Skip variable predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return o.identifier() != target_
       and o.identifier() != *identifiers_.cbegin()
       and not o.contains(o.identifier(), *identifiers_.cbegin());
  }

  /// @brief Selector predicate.
  constexpr bool
  selector()
  const noexcept
  {
    return false;
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& s)
  const
  {
    return apply_visitor( pre(target_, eval_ptr_), s->data()
                        , cxt, o, s, nullptr, nullptr
                        , identifiers_.cbegin(), identifiers_.cend());
  }

  /// @brief Get the user's evaluator.
  const evaluator_base<C>&
  evaluator()
  const noexcept
  {
    return *eval_ptr_;
  }

  /// @brief Get the set of variables.
  const identifiers_type&
  identifiers()
  const noexcept
  {
    return identifiers_;
  }

  /// @brief Get the target.
  const identifier_type&
  target()
  const noexcept
  {
    return target_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief Equality of two expression homomorphisms.
/// @related expression
template <typename C>
inline
bool
operator==(const expression<C>& lhs, const expression<C>& rhs)
noexcept
{
  return lhs.target() == rhs.target()
     and lhs.identifiers() == rhs.identifiers()
     and lhs.evaluator() == rhs.evaluator();
}

/// @related expression
template <typename C>
std::ostream&
operator<<(std::ostream& os, const expression<C>& e)
{
  os << "Expression(" << e.target() << " = ";
  e.evaluator().print(os);
  return os << ")";
}

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the Expression homomorphism.
/// @related homomorphism.
template <typename C, typename User, typename InputIterator>
homomorphism<C>
Expression( const order<C>& o, const User& u, InputIterator begin, InputIterator end
          , const typename C::Identifier& target)
{
  if (std::distance(begin, end) == 0)
  {
    throw std::invalid_argument("Empty set of variables at Expression construction.");
  }

  typedef typename C::Identifier identifier_type;
  typename hom::expression<C>::identifiers_type identifiers(begin, end);
  std::sort( identifiers.begin(), identifiers.end()
           , [&](const identifier_type& lhs, const identifier_type& rhs)
                {
                  return o.compare(lhs, rhs);
                });

  return homomorphism<C>::create( mem::construct<hom::expression<C>>()
                                , std::make_shared<hom::evaluator_derived<C, User>>(u)
                                , std::move(identifiers)
                                , target);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::expression.
template <typename C>
struct hash<sdd::hom::expression<C>>
{
  std::size_t
  operator()(const sdd::hom::expression<C>& e)
  const noexcept
  {
    std::size_t seed = e.evaluator().hash();
    for (const auto& v : e.identifiers())
    {
      sdd::util::hash_combine(seed, v);
    }
    sdd::util::hash_combine(seed, e.target());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_EXPRESSION_HH_
