#ifndef _SDD_HOM_VALUES_FUNCTION_HH_
#define _SDD_HOM_VALUES_FUNCTION_HH_

#include <algorithm> // find
#include <iosfwd>
#include <memory>    // unique_ptr
#include <typeinfo>  // typeid

#include "sdd/dd/definition.hh"
#include "sdd/dd/top.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation_error.hh"
#include "sdd/util/packed.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Used to wrap user's values function.
template <typename C>
class values_function_base
{
public:

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief Destructor.
  virtual
  ~values_function_base()
  {}

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

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Used to wrap user's values function.
template <typename C, typename User>
class values_function_derived
  : public values_function_base<C>
{
private:

  /// @brief The user's values function.
  const User fun_;

public:

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief Constructor.
  values_function_derived(const User& f)
    : fun_(f)
  {}

  /// @brief Constructor.
  values_function_derived(User&& f)
    : fun_(std::move(f))
  {}

  /// @brief Tell if the user's function is a selector.
  bool
  selector()
  const noexcept override
  {
    return selector_impl(fun_, 0);
  }

  /// @brief Apply the user function.
  values_type
  operator()(const values_type& val)
  const override
  {
    return fun_(val);
  }

  /// @brief Compare values_derived.
  bool
  operator==(const values_function_base<C>& other)
  const noexcept override
  {
    return typeid(*this) == typeid(other)
         ? fun_ == reinterpret_cast<const values_function_derived&>(other).fun_
         : false;
  }

  /// @brief Get the user's function hash value.
  std::size_t
  hash()
  const noexcept override
  {
    return std::hash<User>()(fun_);
  }

  /// @brief Get the user's values function textual representation.
  void
  print(std::ostream& os)
  const override
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

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Values homomorphism.
template <typename C>
class LIBSDD_ATTRIBUTE_PACKED values_function
{
private:

  /// @brief The type of a valuation on a flat node.
  using values_type = typename C::Values;

  /// @brief The identifier on which the user function is applied.
  const order_position_type target_;

  /// @brief Ownership of the user's values function.
  const std::unique_ptr<const values_function_base<C>> fun_ptr_;

  /// @brief Dispatch the Values homomorphism evaluation.
  struct helper
  {
    using result_type = SDD<C>;

    /// @brief |0| case, should never happen.
    SDD<C>
    operator()( const zero_terminal<C>&, const SDD<C>&
              , const values_function_base<C>&, context<C>&, const order<C>&)
    const noexcept
    {
      assert(false);
      __builtin_unreachable();
    }

    /// @brief |1| case.
    SDD<C>
    operator()( const one_terminal<C>&, const SDD<C>&
              , const values_function_base<C>&, context<C>&, const order<C>&)
    const
    {
      return one<C>();
    }

    /// @brief A ValuesFunction can't be applied on an hierarchical node.
    SDD<C>
    operator()( const hierarchical_node<C>&, const SDD<C>& s
              , const values_function_base<C>&, context<C>&, const order<C>&)
    const
    {
      throw evaluation_error<C>(s);
    }

    /// @brief Evaluation on a flat node.
    SDD<C>
    operator()( const flat_node<C>& node, const SDD<C>& s
              , const values_function_base<C>& fun, context<C>& cxt, const order<C>& o)
    const
    {
      if (fun.selector())
      {
        dd::alpha_builder<C, values_type> alpha_buidler;
        alpha_buidler.reserve(node.size());
        for (const auto& arc : node)
        {
          values_type val = fun(arc.valuation());
          if (not val.empty())
          {
            alpha_buidler.add(std::move(val), arc.successor());
          }
        }
        return {o.variable(), std::move(alpha_buidler)};
      }
      else
      {
        dd::sum_builder<C, SDD<C>> sum_operands;
        sum_operands.reserve(node.size());
        for (const auto& arc : node)
        {
          sum_operands.add(SDD<C>(o.variable(), fun(arc.valuation()), arc.successor()));
        }
        try
        {
          return dd::sum(cxt.sdd_context(), std::move(sum_operands));
        }
        catch (top<C>& t)
        {
          evaluation_error<C> e(s);
          e.add_top(t);
          throw e;
        }
      }
    }
  };

public:

  /// @brief Constructor.
  values_function(order_position_type pos, const values_function_base<C>* f_ptr)
    : target_(pos)
    , fun_ptr_(f_ptr)
  {}

  /// @brief Skip variable predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return target_ != o.position();
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
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& x)
  const
  {
    return visit_self(helper(), x, *fun_ptr_, cxt, o);
  }

  /// @brief Get the variable on which the user's function is applied.
  order_position_type
  target()
  const noexcept
  {
    return target_;
  }

  /// @brief Return the user's values function.
  const values_function_base<C>&
  fun()
  const noexcept
  {
    return *fun_ptr_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Equality of two values_function.
/// @related values_function
template <typename C>
inline
bool
operator==(const values_function<C>& lhs, const values_function<C>& rhs)
noexcept
{
  return lhs.target() == rhs.target() and lhs.fun() == rhs.fun();
}

/// @internal
/// @related values_function
template <typename C>
std::ostream&
operator<<(std::ostream& os, const values_function<C>& x)
{
  os << "Function(" << x.target() << ", ";
  x.fun().print(os);
  return os << ")";
}

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Create the Values Function homomorphism.
/// @related homomorphism
template <typename C, typename User>
homomorphism<C>
ValuesFunction(order_position_type pos, const User& u)
{
  return homomorphism<C>::create( mem::construct<hom::values_function<C>>()
                                , pos, new hom::values_function_derived<C, User>(u));
}

/// @internal
/// @brief Create the Values Function homomorphism.
/// @related homomorphism
template <typename C, typename User>
homomorphism<C>
ValuesFunction(order_position_type pos, User&& u)
{
  return homomorphism<C>::create( mem::construct<hom::values_function<C>>()
                                , pos, new hom::values_function_derived<C, User>(std::move(u)));
}

/// @brief Create the Values Function homomorphism.
/// @param i The target identifier, must belong to o.
/// @related homomorphism
template <typename C, typename User>
homomorphism<C>
ValuesFunction(const order<C>& o, const typename C::Identifier& i, const User& u)
{
  return ValuesFunction<C>(o.node(i).position(), u);
}

/// @brief Create the Values Function homomorphism.
/// @param i The target identifier, must belong to o.
/// @related homomorphism
template <typename C, typename User>
homomorphism<C>
ValuesFunction(const order<C>& o, const typename C::Identifier& i, User&& u)
{
  return ValuesFunction<C>(o.node(i).position(), std::move(u));
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::values.
template <typename C>
struct hash<sdd::hom::values_function<C>>
{
  std::size_t
  operator()(const sdd::hom::values_function<C>& x)
  const noexcept
  {
    std::size_t seed = x.fun().hash();
    sdd::util::hash_combine(seed, x.target());
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_VALUES_FUNCTION_HH_
