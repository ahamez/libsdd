#ifndef _SDD_HOM_VALUES_FUNCTION_HH_
#define _SDD_HOM_VALUES_FUNCTION_HH_

#include <iosfwd>
#include <memory>   // unique_ptr
#include <typeinfo> // typeid

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation_error.hh"
#include "sdd/internal/util/packed.hh"

namespace sdd { namespace hom {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Used to wrap user's values function.
template <typename C>
class values_function_base
{
public:

  typedef typename C::Values    values_type;

  virtual
  ~values_function_base()
  {
  }

  /// @brief Tell if the user's function is a selector.
  virtual
  bool
  selector() const noexcept = 0;

  /// @brief Apply the user function.
  virtual
  values_type
  operator()(const values_type&) const = 0;

  /// @brief Compare values_base.
  virtual
  bool
  operator==(const values_function_base&) const noexcept = 0;

  /// @brief Get the user's function hash value.
  virtual
  std::size_t
  hash() const noexcept = 0;

  /// @brief Get the user's function textual representation.
  virtual
  void
  print(std::ostream&) const = 0;
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Used to wrap user's values function.
template <typename C, typename User>
class values_function_derived
  : public values_function_base<C>
{
private:

  /// @brief The user's values function.
  const User fun_;

public:

  /// @brief The type of a variable.
  typedef typename C::Variable  variable_type;

  /// @brief The type of a set of values.
  typedef typename C::Values    values_type;

  /// @brief Constructor.
  values_function_derived(const User& f)
    : fun_(f)
  {
  }

  /// @brief Constructor.
  values_function_derived(User&& f)
    : fun_(std::move(f))
  {
  }

  /// @brief Tell if the user's function is a selector.
  bool
  selector()
  const noexcept
  {
    return selector_impl(fun_, 0);
  }

  /// @brief Apply the user function.
  values_type
  operator()(const values_type& val)
  const
  {
    return fun_(val);
  }

  /// @brief Compare values_derived.
  bool
  operator==(const values_function_base<C>& other)
  const noexcept
  {
    return typeid(*this) == typeid(other)
         ? fun_ == reinterpret_cast<const values_function_derived&>(other).fun_
         : false
         ;
  }

  /// @brief Get the user's function hash value.
  std::size_t
  hash()
  const noexcept
  {
    return std::hash<User>()(fun_);
  }

  /// @brief Get the user's values function textual representation.
  void
  print(std::ostream& os)
  const
  {
    os << fun_;
  }

private:

  /// @brief Called when the user's function has selector().
  ///
  /// Compile-time dispatch.
  template <typename T>
  static auto
  selector_impl(const T& x, int)
  noexcept
  -> decltype(x.selector())
  {
    return x.selector();
  }

  /// @brief Called when the user's function doesn't have selector().
  ///
  /// Compile-time dispatch.
  template <typename T>
  static auto
  selector_impl(const T&, long)
  noexcept
  -> decltype(false)
  {
    return false;
  }
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Values homomorphism.
template <typename C>
class _LIBSDD_ATTRIBUTE_PACKED values_function
{
private:

  /// @brief The type of a variable.
  typedef typename C::Variable variable_type;

  /// @brief The type of a valuation on a flat node.
  typedef typename C::Values values_type;

  /// @brief The variable on which the user function is applied.
  const variable_type variable_;

  /// @brief Ownership of the user's values function.
  const std::unique_ptr<const values_function_base<C>> fun_ptr_;

  /// @brief Dispatch the Values homomorphism evaluation.
  struct helper
  {
    typedef SDD<C> result_type;

    SDD<C>
    operator()( const zero_terminal<C>&
              , const values_function_base<C>&, context<C>&, const SDD<C>&)
    const noexcept
    {
      assert(false);
      __builtin_unreachable();
    }

    SDD<C>
    operator()( const one_terminal<C>&
              , const values_function_base<C>&, context<C>&, const SDD<C>&)
    const
    {
      return one<C>();
    }

    SDD<C>
    operator()( const hierarchical_node<C>& node
              , const values_function_base<C>&, context<C>&, const SDD<C>& s)
    const
    {
      throw evaluation_error<C>(s);
    }

    SDD<C>
    operator()( const flat_node<C>& node
              , const values_function_base<C>& fun, context<C>& cxt, const SDD<C>& s)
    const
    {
      if (fun.selector())
      {
        alpha_builder<C, values_type> ab;
        ab.reserve(node.size());
        for (const auto& arc : node)
        {
          values_type val = fun(arc.valuation());
          if (not val.empty())
          {
            ab.add(std::move(val), arc.successor());
          }
        }
        return SDD<C>(node.variable(), std::move(ab));
      }
      else
      {
        sum_builder<C, SDD<C>> sum_operands(node.size());
        for (const auto& arc : node)
        {
          sum_operands.add(SDD<C>(node.variable(), fun(arc.valuation()), arc.successor()));
        }
        return sdd::sum(cxt.sdd_context(), std::move(sum_operands));
      }
    }
  };

public:

  /// @brief Constructor.
  values_function(const variable_type& var, const values_function_base<C>* f_ptr)
    : variable_(var)
    , fun_ptr_(f_ptr)
  {
  }

  /// @brief Skip variable predicate.
  bool
  skip(const variable_type& var)
  const noexcept
  {
    return var != variable_;
  }

  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return fun_ptr_->selector();
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const SDD<C>& x)
  const
  {
    return apply_visitor(helper(), x->data(), *fun_ptr_, cxt, x);
  }

  /// @brief Get the variable on which the user's function is applied.
  const variable_type&
  variable()
  const noexcept
  {
    return variable_;
  }

  /// @brief Return the user's values function.
  const values_function_base<C>&
  fun()
  const noexcept
  {
    return *fun_ptr_;
  }
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Equality of two values.
/// @related values_function
template <typename C>
inline
bool
operator==(const values_function<C>& lhs, const values_function<C>& rhs)
noexcept
{
  return lhs.variable() == rhs.variable() and lhs.fun() == rhs.fun();
}

/// @related values_function
template <typename C>
std::ostream&
operator<<(std::ostream& os, const values_function<C>& x)
{
  os << "Function(" << x.variable() << ", ";
  x.fun().print(os);
  return os << ")";
}

/// @endcond

/*-------------------------------------------------------------------------------------------*/

/// @brief Create the Values Function homomorphism.
/// @related homomorphism
template <typename C, typename User>
homomorphism<C>
ValuesFunction(const typename C::Variable& v, const User& u)
{
  return homomorphism<C>::create( internal::mem::construct<values_function<C>>()
                                , v, new values_function_derived<C, User>(u));
}

/// @brief Create the Values Function homomorphism.
/// @related homomorphism
template <typename C, typename User>
homomorphism<C>
ValuesFunction(const typename C::Variable& v, User&& u)
{
  return homomorphism<C>::create( internal::mem::construct<values_function<C>>()
                                , v, new values_function_derived<C, User>(std::move(u)));
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

namespace std {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Hash specialization for sdd::hom::values.
template <typename C>
struct hash<sdd::hom::values_function<C>>
{
  std::size_t
  operator()(const sdd::hom::values_function<C>& x)
  const noexcept
  {
    std::size_t seed = x.fun().hash();
    sdd::internal::util::hash_combine(seed, x.variable());
    return seed;
  }
};

/// @endcond

/*-------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_VALUES_FUNCTION_HH_
