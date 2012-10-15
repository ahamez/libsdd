#ifndef _SDD_HOM_CLOSURE_HH_
#define _SDD_HOM_CLOSURE_HH_

#include <algorithm> // copy
#include <cassert>
#include <initializer_list>
#include <iosfwd>

#include <boost/container/flat_set.hpp>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/constant.hh"
#include "sdd/hom/definition_fwd.hh"

namespace sdd { namespace hom {

/// @cond INTERNAL_DOC

/*-------------------------------------------------------------------------------------------*/

template <typename C, typename InputIterator>
homomorphism<C>
Closure(InputIterator, InputIterator, const SDD<C>&);

/*-------------------------------------------------------------------------------------------*/

/// @brief Closure homomorphism.
template <typename C>
class closure
{
public:

  /// @brief The variable type.
  typedef typename C::Variable variable_type;

  /// @brief The type of a set of values.
  typedef typename C::Values values_type;

  /// @brief The type of a set of variables.
  typedef boost::container::flat_set<variable_type> variables_type;

  /// @brief We re-use this homomorphism as a visitor.
  typedef SDD<C> result_type;


private:

  /// @brief The set of variables to capture.
  const variables_type variables_;

  /// brief
  const SDD<C> successor_;


public:

  /// @brief Constructor.
  closure(const variables_type& variables, const SDD<C>& successor)
    : variables_(variables)
    , successor_(successor)
  {
  }

  /// @brief Skip predicate.
  constexpr bool
  skip(const variable_type& var)
  const noexcept
  {
    return false;
  }

  /// @brief Selector predicate.
  constexpr bool
  selector()
  const noexcept
  {
    return true;
  }

  /// @brief Evaluation on hierarchical nodes.
  SDD<C>
  operator()(const hierarchical_node<C>& node, context<C>& cxt, const SDD<C>& s)
  const
  {
    const auto next_closure = Closure(variables_.begin(), variables_.end(), successor_);

    square_union<C, values_type> su;
    su.reserve(node.size());

    auto cit = node.begin();

    const SDD<C> first_succ = next_closure(cxt, cit->successor());
    const auto first_nested_closure = Closure( variables_.begin(), variables_.end()
                                             , first_succ);
    const SDD<C> first_val = first_nested_closure(cxt, cit->valuation());

    if (first_val == one<C>()) // All applications on nested will return |1|.
    {
      if (first_succ == one<C>())
      {
        // no variables of the requested closure were found
        return one<C>();
      }
      else
      {
        const flat_node<C>* f = &internal::mem::variant_cast<flat_node<C>>(first_succ->data());
        const variable_type& var = f->variable();
        for (const auto& f_arc : *f)
        {
          // add arcs of the first result
          su.add(f_arc.successor(), f_arc.valuation());
        }

        for (++cit; cit != node.end(); ++cit)
        {
          // apply the closure on successors
          const SDD<C> succ = next_closure(cxt, cit->successor());
          f = &internal::mem::variant_cast<flat_node<C>>(succ->data());
          for (const auto& f_arc : *f)
          {
            // add arcs of the result
            su.add(f_arc.successor(), f_arc.valuation());
          }
        }
        return SDD<C>(var, su(cxt.sdd_context()));
      }
    }
    else
    {
      const flat_node<C>* fv = &internal::mem::variant_cast<flat_node<C>>(first_val->data());
      const variable_type& var = fv->variable();

      for (const auto& fv_arc : *fv)
      {
        // add the arcs of the first result
        su.add(fv_arc.successor(), fv_arc.valuation());
      }

      for (++cit; cit != node.end(); ++cit)
      {
        const SDD<C> succ = next_closure(cxt, cit->successor());
        const auto nested_closure = Closure(variables_.begin(), variables_.end(), succ);
        const SDD<C> val = nested_closure(cxt, cit->valuation());

        fv = &internal::mem::variant_cast<flat_node<C>>(val->data());
        for (const auto& fv_arc : *fv)
        {
        // add the arcs of the result
          su.add(fv_arc.successor(), fv_arc.valuation());
        }
      }
      return SDD<C>(var, su(cxt.sdd_context()));
    }
  }

  /// @brief Evaluation on flat nodes.
  SDD<C>
  operator()(const flat_node<C>& node, context<C>& cxt, const SDD<C>& s)
  const
  {
    const auto next_closure = Closure(variables_.begin(), variables_.end(), successor_);

    if (variables_.find(node.variable()) != variables_.end())
    {
      square_union<C, values_type> su;
      su.reserve(node.size());
      for (const auto& arc : node)
      {
        SDD<C> new_successor = next_closure(cxt, arc.successor());
        su.add(new_successor, arc.valuation());
      }
      return SDD<C>(node.variable(), su(cxt.sdd_context()));
    }
    else
    {
      square_union<C, values_type> su;
      su.reserve(node.size());

      auto cit = node.begin();
      SDD<C> first = next_closure(cxt, cit->successor());

      if (first == one<C>())
      {
        // All applications on successors will return |1|.
        // OK, there we make a strong assumption. We suppose the order is the same on every
        // path (is it really a strong assumption?). Otherwise, it would be quite difficult
        // to make a closure...
        return one<C>();
      }

      const flat_node<C>& f = internal::mem::variant_cast<flat_node<C>>(first->data());
      for (const auto& f_arc : f)
      {
        su.add(f_arc.successor(), f_arc.valuation());
      }

      for (++cit; cit != node.end(); ++cit)
      {
        SDD<C> succ = next_closure(cxt, cit->successor());
        const flat_node<C>& ns = internal::mem::variant_cast<flat_node<C>>(succ->data());
        for (const auto& ns_arc : ns)
        {
          su.add(ns_arc.successor(), ns_arc.valuation());
        }
      }
      return SDD<C>(f.variable(), su(cxt.sdd_context()));
    }
  }

  SDD<C>
  operator()(const one_terminal<C>&, context<C>&, const SDD<C>&)
  const noexcept
  {
    return successor_;
  }
  
  SDD<C>
  operator()(const zero_terminal<C>&, context<C>&, const SDD<C>&)
  const noexcept
  {
    assert(false);
    __builtin_unreachable();
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const SDD<C>& x)
  const
  {
    return apply_visitor(*this, x->data(), cxt, x);
  }

  const variables_type&
  variables()
  const noexcept
  {
    return variables_;
  }

  SDD<C>
  successor()
  const noexcept
  {
    return successor_;
  }
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Equality of two closure.
/// @related closure
template <typename C>
inline
bool
operator==(const closure<C>& lhs, const closure<C>& rhs)
noexcept
{
  return lhs.variables() == rhs.variables() and lhs.successor() == rhs.successor();
}

/// @related closure
template <typename C>
std::ostream&
operator<<(std::ostream& os, const closure<C>& c)
{
  return os << "Closure(";
  std::copy( c.variables().begin(), std::prev(c.variables().end())
           , std::ostream_iterator<typename C::Variable>(os, ", "));
  return os << *std::prev(c.variables().end()) << ")";
}

/// @endcond

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Create the Closure homomorphism.
/// @related homomorphism
template <typename C, typename InputIterator>
homomorphism<C>
Closure(InputIterator begin, InputIterator end, const SDD<C>& succ)
{
  if (std::distance(begin, end) == 0)
  {
    throw std::invalid_argument("Empty set of variables at Closure construction.");
  }

  return homomorphism<C>::create( internal::mem::construct<closure<C>>()
                                , typename closure<C>::variables_type(begin, end), succ);
}

/// @endcond

/// @brief Create the Closure homomorphism.
/// @related homomorphism
template <typename C, typename InputIterator>
homomorphism<C>
Closure(InputIterator begin, InputIterator end)
{
  return Closure<C>(begin, end, one<C>());
}

/// @brief Create the Closure homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
Closure(std::initializer_list<typename C::Variable> list)
{
  return Closure<C>(list.begin(), list.end());
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

namespace std {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Hash specialization for sdd::hom::closure.
template <typename C>
struct hash<sdd::hom::closure<C>>
{
  std::size_t
  operator()(const sdd::hom::closure<C>& c)
  const noexcept
  {
    std::size_t seed = 0;
    for (const auto& v : c.variables())
    {
      sdd::internal::util::hash_combine(seed, v);
    }
    sdd::internal::util::hash_combine(seed, c.successor());
    return seed;
  }
};

/// @endcond

/*-------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_CLOSURE_HH_
