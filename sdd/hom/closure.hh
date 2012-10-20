#ifndef _SDD_HOM_CLOSURE_HH_
#define _SDD_HOM_CLOSURE_HH_

#include <algorithm> // any_of, copy, sort
#include <cassert>
#include <initializer_list>
#include <iosfwd>
#include <vector>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/constant.hh"
#include "sdd/hom/definition_fwd.hh"

namespace sdd { namespace hom {

/// @cond INTERNAL_DOC

/*-------------------------------------------------------------------------------------------*/

template <typename C>
using identifiers_type = std::vector<typename C::Identifier>;

template <typename C>
using identifiers_iterator_type = typename identifiers_type<C>::const_iterator;

template <typename C>
using identifiers_ptr_type = std::shared_ptr<identifiers_type<C>>;

template <typename C>
homomorphism<C>
Closure( const order::order<C>&, const identifiers_ptr_type<C>&
       , identifiers_iterator_type<C>, const SDD<C>&);

/*-------------------------------------------------------------------------------------------*/

/// @brief Closure homomorphism.
template <typename C>
class closure
{
public:

  /// @brief The variable type.
  typedef typename C::Variable variable_type;

  /// @brief The identifier type.
  typedef typename C::Identifier identifier_type;

  /// @brief The type of a set of values.
  typedef typename C::Values values_type;

  /// @brief The type of a set of identifiers.
//  typedef std::vector<identifier_type> identifiers_type;

  /// @brief The type of a pointer to a set of identifiers.
//  typedef std::shared_ptr<identifiers_type> identifiers_ptr_type;

  /// @brief The type of an iterator to a set of identifiers.
//  typedef typename identifiers_type::const_iterator identifiers_iterator_type;

  /// @brief We re-use this homomorphism as a visitor.
  typedef SDD<C> result_type;

private:

  /// @brief The set of variables to capture.
  ///
  /// It's shared amongst all closures capturing this same set of variables.
  const identifiers_ptr_type<C> identifiers_ptr_;

  /// @brief Mark the beginning of the set of variables this closure is really interested in.
  const identifiers_iterator_type<C> begin_;

  /// @brief What to concatenate when this closure arrives at the end of a path.
  const SDD<C> successor_;

public:

  /// @brief Constructor.
  closure( const identifiers_ptr_type<C>& ptr, identifiers_iterator_type<C> begin
         , const SDD<C>& successor)
    : identifiers_ptr_(ptr)
    , begin_(begin)
    , successor_(successor)
  {
    assert(begin_ != identifiers_ptr_->end() && "Empty set of identifiers to capture.");
  }

  /// @brief Skip predicate.
  constexpr bool
  skip(const order::order<C>& o)
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
  operator()( const hierarchical_node<C>& node
            , context<C>& cxt, const order::order<C>& o, const SDD<C>& s)
  const
  {
    assert(not o.nested().empty() && "Empty hierarchical order in a hierarchical_node.");

//    std::any_of( begin_, identifiers_ptr_->end()
//                , [&o](const identifier_type& id)
//                {
//                  return o.contains(id);
//                });

//    square_union<C, values_type> su;
//    su.reserve(node.size());
//
//    for (const auto& arc : node)
//    {
//      // First, get the new successor
//      const auto next_closure = Closure( o.next(), identifiers_ptr_, std::next(begin_)
//                                       , successor_);
//      SDD<C> new_successor = next_closure(cxt, o.next(), arc.successor());
//
//      // Then, transmit it to the nested closure.
//      const auto nested_closure = Closure( o.nested(), identifiers_ptr_, begin_
//                                         , new_successor);
//      SDD<C> new_valuation = nested_closure(cxt, o.nested(), arc.valuation());
//
//      // Finally, add the new arc to square union operands.
//      su.add(new_successor, new_valuation);
//    }
//
//    return SDD<C>(o.identifier_variable(*begin_), su(cxt.sdd_context()));
  }

  /// @brief Evaluation on flat nodes.
  SDD<C>
  operator()( const flat_node<C>& node
            , context<C>& cxt, const order::order<C>& o, const SDD<C>& s)
  const
  {
    assert(o.nested().empty() && "Hierarchical order in a flat_node.");

    if (*begin_ != o.identifier()) // remove current level
    {
      if (o.next().empty())
      {
        return successor_;
      }
      else
      {
        square_union<C, values_type> su;
        su.reserve(node.size());

        const auto next_closure = Closure(o.next(), identifiers_ptr_, begin_, successor_);
        for (const auto& arc : node)
        {
          const SDD<C> succ = next_closure(cxt, o.next(), arc.successor());
          const flat_node<C>& n
            = internal::mem::variant_cast<const flat_node<C>>(succ->data());
          for (const auto& succ_arc : n)
          {
            su.add(succ_arc.successor(), succ_arc.valuation());
          }
        }

        return SDD<C>(o.identifier_variable(*begin_), su(cxt.sdd_context()));
      }
    }
    else // keep current level
    {
      const auto next_begin = std::next(begin_);

      square_union<C, values_type> su;
      su.reserve(node.size());

      if (next_begin == identifiers_ptr_->end()) // avoid useless homomorphism application
      {
        for (const auto& arc : node)
        {
          su.add(successor_, arc.valuation());
        }
      }
      else
      {
        const auto next_closure = Closure(o.next(), identifiers_ptr_, next_begin, successor_);
        for (const auto& arc : node)
        {
          SDD<C> new_successor = next_closure(cxt, o.next(), arc.successor());
          su.add(new_successor, arc.valuation());
        }
      }

      return SDD<C>(o.identifier_variable(*begin_), su(cxt.sdd_context()));
    }
  }

  SDD<C>
  operator()( const one_terminal<C>&
            , context<C>&, const order::order<C>&, const SDD<C>&)
  const noexcept
  {
    return successor_;
  }
  
  SDD<C>
  operator()( const zero_terminal<C>&
            , context<C>&, const order::order<C>&, const SDD<C>&)
  const noexcept
  {
    assert(false);
    __builtin_unreachable();
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order::order<C>& o, const SDD<C>& x)
  const
  {
    return apply_visitor(*this, x->data(), cxt, o, x);
  }

  const identifiers_type<C>&
  identifiers()
  const noexcept
  {
    return *identifiers_ptr_;
  }

  SDD<C>
  successor()
  const noexcept
  {
    return successor_;
  }

  const identifier_type&
  head()
  const noexcept
  {
    return *begin_;
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
  return lhs.identifiers() == rhs.identifiers()
     and lhs.head() == rhs.head()
     and lhs.successor() == rhs.successor();
}

/// @related closure
template <typename C>
std::ostream&
operator<<(std::ostream& os, const closure<C>& c)
{
  return os << "Closure(";
  std::copy( c.identifiers().begin(), std::prev(c.identifiers().end())
           , std::ostream_iterator<typename C::Identifier>(os, ", "));
  return os << *std::prev(c.identifiers().end()) << ")";
}

/// @endcond

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Create the Closure homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
Closure( const order::order<C>& o, const identifiers_ptr_type<C>& ptr
       , identifiers_iterator_type<C> begin, const SDD<C>& succ)
{
  return homomorphism<C>::create(internal::mem::construct<closure<C>>(), ptr, begin, succ);
}

/// @endcond

/// @brief Create the Closure homomorphism.
/// @related homomorphism
template <typename C, typename InputIterator>
homomorphism<C>
Closure(const order::order<C>& o, InputIterator begin, InputIterator end)
{
  if (std::distance(begin, end) == 0)
  {
    throw std::invalid_argument("Empty set of variables at Closure construction.");
  }

  auto ptr = std::make_shared<identifiers_type<C>>(begin, end);

  typedef typename C::Identifier identifier_type;
  std::sort( ptr->begin(), ptr->end()
           , [&o](const identifier_type& lhs, const identifier_type& rhs)
                 {
                   return o.compare(lhs, rhs);
                 }
           );

  return Closure(o, ptr, ptr->begin(), one<C>());
}

/// @brief Create the Closure homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
Closure(const order::order<C>& o, std::initializer_list<typename C::Identifier> list)
{
  return Closure<C>(o, list.begin(), list.end());
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
    for (const auto& i : c.identifiers())
    {
      sdd::internal::util::hash_combine(seed, i);
    }
    sdd::internal::util::hash_combine(seed, c.successor());
    sdd::internal::util::hash_combine(seed, c.head());
    return seed;
  }
};

/// @endcond

/*-------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_CLOSURE_HH_
