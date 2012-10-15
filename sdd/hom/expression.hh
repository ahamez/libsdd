#ifndef _SDD_HOM_EXPRESSION_HH_
#define _SDD_HOM_EXPRESSION_HH_

#include <algorithm> // copy
#include <iosfwd>
#include <memory>    // unique_ptr
#include <stdexcept> // invalid_argument

#include <boost/container/flat_set.hpp>

#include "sdd/dd/definition.hh"
#include "sdd/hom/closure.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation_error.hh"

namespace sdd { namespace hom {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief
template <typename C>
class evaluator_base
{
public:

  typedef typename C::Variable  variable_type;
  typedef typename C::Values    values_type;

  virtual
  ~evaluator_base()
  {
  }

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

/*-------------------------------------------------------------------------------------------*/

/// @brief Used to wrap user's evaluator.
template <typename C, typename User>
class evaluator_derived
  : public evaluator_base<C>
{
private:

  /// @brief The user's evaluator.
  const User eval_;

public:

  /// @brief The type of a variable.
  typedef typename C::Variable  variable_type;

  /// @brief The type of a set of values.
  typedef typename C::Values    values_type;

  /// @brief Constructor.
  evaluator_derived(const User& eval)
    : eval_(eval)
  {
  }

  /// @brief Compare evaluator_derived.
  bool
  operator==(const evaluator_base<C>& other)
  const noexcept
  {
    try
    {
      return eval_ == dynamic_cast<const evaluator_derived&>(other).eval_;
    }
    catch(std::bad_cast)
    {
      return false;
    }
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

/*-------------------------------------------------------------------------------------------*/

/// @brief Expression homomorphism.
template <typename C>
class expression
{
public:

  /// @brief A variable type.
  typedef typename C::Variable variable_type;

  /// @brief The type of a set of variables.
  typedef boost::container::flat_set<variable_type> variables_type;

private:

  /// @brief Ownership of the user's evaluator.
  const std::unique_ptr<const evaluator_base<C>> eval_ptr_;

  /// @brief Variables used in this expression.
  const variables_type variables_;

  /// @brief The target of the assignment.
  const variable_type target_;

public:

  /// @brief Constructor.
  expression( const evaluator_base<C>* e_ptr, variables_type&& variables
            , const variable_type& target)
    : eval_ptr_(e_ptr)
    , variables_(std::move(variables))
    , target_(target)
  {
  }

  /// @brief Skip variable predicate.
  bool
  skip(const variable_type& var)
  const noexcept
  {
    return var != target_ and variables_.find(var) == variables_.end();
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
  operator()(context<C>& cxt, const SDD<C>& s)
  const
  {
    // Get the closure.
    const SDD<C> closure = Closure<C>(variables_.begin(), variables_.end())(cxt, s);

    // Evaluate the expression using the closure.
  }

  /// @brief Get the user's evaluator.
  const evaluator_base<C>&
  evaluator()
  const noexcept
  {
    return *eval_ptr_;
  }

  /// @brief Get the set of variables.
  const variables_type&
  variables()
  const noexcept
  {
    return variables_;
  }

  /// @brief Get the target.
  const variable_type&
  target()
  const noexcept
  {
    return target_;
  }

};

/*-------------------------------------------------------------------------------------------*/

/// @brief Equality of two expression homomorphisms.
/// @related expression
template <typename C>
inline
bool
operator==(const expression<C>& lhs, const expression<C>& rhs)
noexcept
{
  return lhs.target() == rhs.target()
     and lhs.variables() == rhs.variables()
     and lhs.evaluator() == rhs.evaluator();
}

/// @related expression
template <typename C>
std::ostream&
operator<<(std::ostream& os, const expression<C>& e)
{
  os << "Expression(" << e.target() << " = ";
  e.evaluator().print(os);
  os << " [";
  std::copy( e.variables().begin(), std::prev(e.variables().end())
           , std::ostream_iterator<typename expression<C>::variable_type>(os, ", "));
  return os << *std::prev(e.variables().end()) << "])";
}

/// @endcond

/*-------------------------------------------------------------------------------------------*/

/// @brief Create the Expression homomorphism.
/// @related homomorphism.
template <typename C, typename User, typename InputIterator>
homomorphism<C>
Expression( const User& u, InputIterator begin, InputIterator end
          , const typename C::Variable& target)
{
  if (std::distance(begin, end) == 0)
  {
    throw std::invalid_argument("Empty set of variables at Expression construction.");
  }

  return homomorphism<C>::create( internal::mem::construct<expression<C>>()
                                , new evaluator_derived<C, User>(u)
                                , typename expression<C>::variables_type(begin, end)
                                , target);
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

/// @cond INTERNAL_DOC

namespace std {

/*-------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::hom::expression.
template <typename C>
struct hash<sdd::hom::expression<C>>
{
  std::size_t
  operator()(const sdd::hom::expression<C>& e)
  const noexcept
  {
    std::size_t seed = e.evaluator().hash();
    for (const auto& v : e.variables())
    {
      sdd::internal::util::hash_combine(seed, v);
    }
    sdd::internal::util::hash_combine(seed, e.target());
    return seed;
  }
};

/*-------------------------------------------------------------------------------------------*/

} // namespace std

/// @endcond

#endif // _SDD_HOM_EXPRESSION_HH_
