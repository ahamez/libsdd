/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <algorithm> // find
#include <iosfwd>
#include <memory>    // unique_ptr
#include <typeinfo>  // typeid

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/util/packed.hh"
#include "sdd/order/carrier.hh"
#include "sdd/order/order.hh"
#include "sdd/order/order_node.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct function_base
{
  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief Destructor.
  virtual
  ~function_base()
  {}

  /// @brief Tell if the user's function is a selector.
  virtual
  bool
  selector() const noexcept = 0;

  /// @brief Tell if the user's function is a shifter.
  virtual
  bool
  shifter() const noexcept = 0;

  /// @brief Apply the user function.
  virtual
  values_type
  operator()(const values_type&) const = 0;

  /// @brief Compare values_base.
  virtual
  bool
  operator==(const function_base&) const noexcept = 0;

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
template <typename C, typename User>
struct function_derived
  : public function_base<C>
{
  /// @brief The user's values function.
  const User fun;

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief Constructor.
  function_derived(User f)
    : fun(std::move(f))
  {}

  /// @brief Tell if the user's function is a selector.
  bool
  selector()
  const noexcept override
  {
    return selector_impl(fun, 0);
  }

  /// @brief Tell if the user's function is a shifter.
  bool
  shifter()
  const noexcept override
  {
    return shifter_impl(fun, 0);
  }

  /// @brief Apply the user function.
  values_type
  operator()(const values_type& val)
  const override
  {
    return fun(val);
  }

  /// @brief Compare values_derived.
  bool
  operator==(const function_base<C>& other)
  const noexcept override
  {
    return typeid(*this) == typeid(other)
           and fun == static_cast<const function_derived&>(other).fun;
  }

  /// @brief Get the user's function hash value.
  std::size_t
  hash()
  const noexcept override
  {
    return std::hash<User>()(fun);
  }

  /// @brief Get the user's values function textual representation.
  void
  print(std::ostream& os)
  const override
  {
    print_impl(os, fun, 0);
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

  /// @brief Called when the user's function has shifter().
  ///
  /// Compile-time dispatch.
  template <typename T>
  static auto
  shifter_impl(const T& x, int)
  noexcept
  -> decltype(x.shifter())
  {
    return x.shifter();
  }

  /// @brief Called when the user's function doesn't have shifter().
  ///
  /// Compile-time dispatch.
  template <typename T>
  static auto
  shifter_impl(const T&, long)
  noexcept
  -> decltype(false)
  {
    return false;
  }

  /// @brief Called when the user's function has operator<<(ostream&).
  ///
  /// Compile-time dispatch.
  template <typename T>
  static auto
  print_impl(std::ostream& os, const T& x, int)
  -> decltype(operator<<(os, x))
  {
    return os << x;
  }

  /// @brief Called when the user's function doesn't have operator<<(ostream&).
  ///
  /// Compile-time dispatch.
  template <typename T>
  static auto
  print_impl(std::ostream& os, const T& x, long)
  -> decltype(void())
  {
    os << "function(" << &x << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct LIBSDD_ATTRIBUTE_PACKED _function
{
  /// @brief The type of a valuation on a flat node.
  using values_type = typename C::Values;

  /// @brief The identifier on which the user function is applied.
  const typename C::variable_type target;

  /// @brief Ownership of the user's values function.
  const std::unique_ptr<const function_base<C>> fun_ptr;

  /// @brief Dispatch the Values homomorphism evaluation.
  struct evaluation
  {
    /// @brief |0| case, should never happen.
    SDD<C>
    operator()(const zero_terminal<C>&, const function_base<C>&, context<C>&, const order<C>&)
    const noexcept
    {
      assert(false);
      __builtin_unreachable();
    }

    /// @brief |1| case.
    SDD<C>
    operator()(const one_terminal<C>&, const function_base<C>&, context<C>&, const order<C>&)
    const
    {
      return one<C>();
    }

    /// @brief A function can't be applied on an hierarchical node.
    SDD<C>
    operator()(const hierarchical_node<C>&, const function_base<C>&, context<C>&, const order<C>&)
    const
    {
      assert(false && "Apply function on an hierarchical node");
      __builtin_unreachable();
    }

    /// @brief Evaluation on a flat node.
    SDD<C>
    operator()( const flat_node<C>& node, const function_base<C>& fun, context<C>& cxt
              , const order<C>& o)
    const
    {
      if (fun.selector() or fun.shifter())
      {
        dd::alpha_builder<C, values_type> alpha_builder(cxt.sdd_context());
        alpha_builder.reserve(node.size());
        for (const auto& arc : node)
        {
          values_type val = fun(arc.valuation());
          if (not val.empty())
          {
            alpha_builder.add(std::move(val), arc.successor());
          }
        }
        return {o.variable(), std::move(alpha_builder)};
      }
      else
      {
        dd::sum_builder<C, SDD<C>> sum_operands(cxt.sdd_context());
        sum_operands.reserve(node.size());
        for (const auto& arc : node)
        {
          sum_operands.add(SDD<C>(o.variable(), fun(arc.valuation()), arc.successor()));
        }
        return dd::sum(cxt.sdd_context(), std::move(sum_operands));
      }
    }
  };

  /// @brief Skip variable predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return target != o.variable();
  }

  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return fun_ptr->selector();
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& x)
  const
  {
    return visit(evaluation(), x, *fun_ptr, cxt, o);
  }

  friend
  bool
  operator==(const _function& lhs, const _function& rhs)
  noexcept
  {
    return lhs.target == rhs.target and *lhs.fun_ptr == *rhs.fun_ptr;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const _function& x)
  {
    os << "fun(" << x.target << ", ";
    x.fun_ptr->print(os);
    return os << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the Function homomorphism.
/// @param i The target identifier, must belong to o.
/// @related homomorphism
///
/// If the target is in a nested hierarchy, the succession of Local to access it is automatically
/// created.
template <typename C, typename User>
homomorphism<C>
function(const order<C>& o, const typename C::Identifier& id, User&& u)
{
  /// @todo Check that id is a flat identifier.
  const auto var = o.node(id).variable();
  const auto f = hom::make<C, hom::_function<C>>
    (var, std::make_unique<hom::function_derived<C, User>>(std::forward<User>(u)));
  return carrier(o, id, std::move(f));
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::_function.
template <typename C>
struct hash<sdd::hom::_function<C>>
{
  std::size_t
  operator()(const sdd::hom::_function<C>& x)
  const noexcept
  {
    using namespace sdd::hash;
    return seed(x.fun_ptr->hash()) (val(x.target));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
