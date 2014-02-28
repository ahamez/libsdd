#ifndef _SDD_DD_DEFINITION_HH_
#define _SDD_DD_DEFINITION_HH_

#include <cassert>

#include "sdd/dd/alpha.hh"
#include "sdd/dd/context_fwd.hh"
#include "sdd/dd/count_combinations_fwd.hh"
#include "sdd/dd/definition_fwd.hh"
#include "sdd/dd/node.hh"
#include "sdd/dd/path_generator_fwd.hh"
#include "sdd/dd/terminal.hh"
#include "sdd/dd/top.hh"
#include "sdd/mem/ptr.hh"
#include "sdd/mem/ref_counted.hh"
#include "sdd/mem/variant.hh"
#include "sdd/order/order.hh"
#include "sdd/values/empty.hh"

// Include for forwards at the end of the file.

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief SDD at the deepest level.
template <typename C>
using flat_node = node<C, typename C::Values>;

/// @brief All but SDD at the deepest level.
template <typename C>
using hierarchical_node = node<C, SDD<C>>;

/*------------------------------------------------------------------------------------------------*/

/// @brief Hierarchical Set Decision Diagram.
template <typename C>
class SDD
{
private:

  /// @brief A canonized SDD.
  ///
  /// This is the real recursive definition of an SDD: it can be a |0| or |1| terminal, or it
  /// can be a flat or an hierachical node.
  typedef mem::variant<zero_terminal<C>, one_terminal<C>, flat_node<C>, hierarchical_node<C>>
          data_type;

public:

  /// @internal
  static constexpr std::size_t zero_terminal_index
    = data_type::template index_for_type<zero_terminal<C>>();

  /// @internal
  static constexpr std::size_t one_terminal_index
    = data_type::template index_for_type<one_terminal<C>>();

  /// @internal
  static constexpr std::size_t flat_node_index
    = data_type::template index_for_type<flat_node<C>>();

  /// @internal
  static constexpr std::size_t hierarchical_node_index
    = data_type::template index_for_type<hierarchical_node<C>>();

  /// @internal
  /// @brief A unified and canonized SDD, meant to be stored in a unique table.
  ///
  /// It is automatically erased when there is no more reference to it.
  using unique_type = mem::ref_counted<data_type>;

  /// @internal
  /// @brief The type of the smart pointer around a unified SDD.
  ///
  /// It handles the reference counting as well as the deletion of the SDD when it is no longer
  /// referenced.
  using ptr_type = mem::ptr<unique_type>;

  /// @brief The type of variables.
  using variable_type = typename C::variable_type;

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

private:

  /// @brief The real smart pointer around a unified SDD.
  ptr_type ptr_;

public:

  /// @brief Copy constructor.
  ///
  /// O(1).
  SDD(const SDD&) noexcept = default;

  /// @brief Copy operator.
  ///
  /// O(1).
  SDD&
  operator=(const SDD&) noexcept = default;

  /// @internal
  /// @brief Construct a flat SDD in a given context.
  /// @param cxt  The construction context.
  /// @param var  The SDD's variable.
  /// @param val  The SDD's valuation, a set of values.
  /// @param succ The SDD's successor.
  ///
  /// O(1).
  SDD(dd::context<C>& cxt, const variable_type& var, values_type&& val, const SDD& succ)
    : ptr_(create_node(cxt, var, std::move(val), succ))
  {}

  /// @internal
  /// @brief Construct a flat SDD.
  /// @param var  The SDD's variable.
  /// @param val  The SDD's valuation, a set of values.
  /// @param succ The SDD's successor.
  ///
  /// O(1).
  SDD(variable_type var, values_type&& val, const SDD& succ)
    : SDD(global<C>().sdd_context, var, std::move(val), succ)
  {}

  /// @internal
  /// @brief Construct a flat SDD in a given context.
  /// @param cxt  The construction context.
  /// @param var  The SDD's variable.
  /// @param val  The SDD's valuation, a set of values.
  /// @param succ The SDD's successor.
  ///
  /// O(1).
  SDD(dd::context<C>& cxt, variable_type var, const values_type& val, const SDD& succ)
    : ptr_(create_node(cxt, var, val, succ))
  {}

  /// @internal
  /// @brief Construct a flat SDD.
  /// @param var  The SDD's variable.
  /// @param val  The SDD's valuation, a set of values.
  /// @param succ The SDD's successor.
  ///
  /// O(1).
  SDD(variable_type var, const values_type& val, const SDD& succ)
    : SDD(global<C>().sdd_context, var, val, succ)
  {}

  /// @internal
  /// @brief Construct a hierarchical SDD in a given context.
  /// @param cxt  The construction context.
  /// @param var  The SDD's variable.
  /// @param val  The SDD's valuation, an SDD in this case.
  /// @param succ The SDD's successor.
  ///
  /// O(1).
  SDD(dd::context<C>& cxt, variable_type var, const SDD& val, const SDD& succ)
    : ptr_(create_node(cxt, var, val, succ))
  {}

  /// @internal
  /// @brief Construct a hierarchical SDD.
  /// @param var  The SDD's variable.
  /// @param val  The SDD's valuation, an SDD in this case.
  /// @param succ The SDD's successor.
  ///
  /// O(1).
  SDD(variable_type var, const SDD& val, const SDD& succ)
    : SDD(global<C>().sdd_context, var, val, succ)
  {}

  /// @internal
  /// @brief Construct an SDD with an order in a given context.
  template <typename Initializer>
  SDD(dd::context<C>& cxt, const order<C>& o, const Initializer& init)
    : ptr_(one_ptr())
  {
    if (o.empty()) // base case of the recursion, ptr_ is defaulted to |1|
    {
      return;
    }
    else if (o.nested().empty()) // flat
    {
      // We can safely pass the order_identifier as a user one because only hierarchical levels
      // can be artificial.
      assert(not o.identifier().artificial());
      ptr_ = create_node(cxt, o.variable(), init(o.identifier().user()), SDD(cxt, o.next(), init));
    }
    else // hierarchical
    {
      ptr_ = create_node(cxt, o.variable(), SDD(cxt, o.nested(), init), SDD(cxt, o.next(), init));
    }
  }

  /// @brief Construct an SDD with an order.
  template <typename Initializer>
  SDD(const order<C>& o, const Initializer& init)
    : SDD(global<C>().sdd_context, o, init)
  {}

#if !defined(HAS_NO_BOOST_COROUTINE)
  /// @brief Return an iterable object which generates all paths of this SDD.
  path_generator<C>
  paths()
  const
  {
    boost::coroutines::attributes attrs(boost::coroutines::fpu_not_preserved);
    return path_generator<C>(std::bind(dd::paths<C>, std::placeholders::_1, *this), attrs);
  }
#endif

  /// @brief Indicate if the SDD is |0|.
  /// @return true if the SDD is |0|, false otherwise.
  ///
  /// O(1).
  bool
  empty()
  const noexcept
  {
    return ptr_ == zero_ptr();
  }

  /// @brief Swap two SDD.
  ///
  /// O(1).
  friend void
  swap(SDD& lhs, SDD& rhs)
  noexcept
  {
    using std::swap;
    swap(lhs.ptr_, rhs.ptr_);
  }

  /// @internal
  /// @brief Construct an SDD from a ptr.
  ///
  /// O(1).
  SDD(const ptr_type& ptr)
  noexcept
    : ptr_(ptr)
  {}

  /// @internal
  /// @brief  Construct an SDD, flat or hierarchical, with an alpha.
  /// \tparam Valuation If an SDD, constructs a hierarchical SDD; if a set of values,
  /// constructs a flat SDD.
  ///
  /// O(n) where n is the number of arcs in the builder.
  template <typename Valuation>
  SDD(variable_type var, dd::alpha_builder<C, Valuation>&& builder)
    : ptr_(create_node(var, std::move(builder)))
  {}

  /// @internal
  /// @brief Get the content of the SDD (an mem::variant).
  ///
  /// O(1).
  const data_type&
  operator*()
  const noexcept
  {
    return ptr_->data();
  }

  /// @internal
  /// @brief Get a pointer to the content of the SDD (an mem::variant).
  ///
  /// O(1).
  const data_type*
  operator->()
  const noexcept
  {
    return &ptr_->data();
  }

  /// @internal
  /// @brief Get the real smart pointer of the unified data.
  ///
  /// O(1).
  ptr_type
  ptr()
  const noexcept
  {
    return ptr_;
  }

  /// @internal
  /// @brief Return the globally cached |0| terminal.
  ///
  /// O(1).
  static
  ptr_type
  zero_ptr()
  noexcept
  {
    return global<C>().zero;
  }

  /// @internal
  /// @brief Return the globally cached |1| terminal.
  ///
  /// O(1).
  static
  ptr_type
  one_ptr()
  noexcept
  {
    return global<C>().one;
  }

  /// @internal
  std::size_t
  index()
  const noexcept
  {
    return ptr_->data().index();
  }

  /// @internal
  std::size_t
  hash()
  const noexcept
  {
    return std::hash<SDD>()(*this);
  }

  /// @brief Get the number of combinations stored in this SDD.
  boost::multiprecision::cpp_int
  size()
  const
  {
    return dd::count_combinations(*this);
  }

private:

  /// @internal
  /// @brief Helper function to create a node, flat or hierarchical, with only one arc.
  ///
  /// O(1).
  template <typename Valuation>
  static
  ptr_type
  create_node(dd::context<C>& cxt, variable_type var, Valuation&& val, const SDD& succ)
  {
    if (succ.empty() or values::empty_values(val))
    {
      return zero_ptr();
    }
    else
    {
      dd::alpha_builder<C, Valuation> builder(cxt);
      builder.add(std::move(val), succ);
      return ptr_type(unify_node<Valuation>(var, std::move(builder)));
    }
  }

  /// @internal
  /// @brief Helper function to create a node, flat or hierarchical, with only one arc.
  ///
  /// O(1).
  template <typename Valuation>
  static
  ptr_type
  create_node(dd::context<C>& cxt, variable_type var, const Valuation& val, const SDD& succ)
  {
    if (succ.empty() or values::empty_values(val))
    {
      return zero_ptr();
    }
    else
    {
      dd::alpha_builder<C, Valuation> builder(cxt);
      builder.add(val, succ);
      return ptr_type(unify_node<Valuation>(var, std::move(builder)));
    }
  }

  /// @internal
  /// @brief Helper function to create a node, flat or hierarchical, from an alpha.
  ///
  /// O(n) where n is the number of arcs in the builder.
  template <typename Valuation>
  static
  ptr_type
  create_node(variable_type var, dd::alpha_builder<C, Valuation>&& builder)
  {
    if (builder.empty())
    {
      return zero_ptr();
    }
    else
    {
      return ptr_type(unify_node<Valuation>(var, std::move(builder)));
    }
  }

  /// @internal
  /// @brief Helper function to unify a node, flat or hierarchical, from an alpha.
  ///
  /// O(n) where n is the number of arcs in the builder.
  template <typename Valuation>
  static
  unique_type&
  unify_node(variable_type var, dd::alpha_builder<C, Valuation>&& builder)
  {
    // Will be erased by the unicity table, either it's an already existing node or a deletion
    // is requested by ptr.
    // Note that the alpha function is allocated right behind the node, thus extra care must be
    // taken. This is also why we use Boost.Intrusive in order to be able to manage memory
    // exactly the way we want.
    auto& ut = global<C>().sdd_unique_table;
    char* addr = ut.allocate(builder.size_to_allocate());
    unique_type* u =
      new (addr) unique_type(mem::construct<node<C, Valuation>>(), var, builder);
    return ut(u);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @brief   Equality of two SDD.
/// @related SDD
///
/// O(1).
template <typename C>
inline
bool
operator==(const SDD<C>& lhs, const SDD<C>& rhs)
noexcept
{
  return lhs.ptr() == rhs.ptr();
}

/// @brief   Inequality of two SDD.
/// @related SDD
///
/// O(1).
template <typename C>
inline
bool
operator!=(const SDD<C>& lhs, const SDD<C>& rhs)
noexcept
{
  return not (lhs.ptr() == rhs.ptr());
}

/// @brief   Comparison of two SDD.
/// @related SDD
///
/// O(1). The order of SDD is arbitrary and can change at each run.
template <typename C>
inline
bool
operator<(const SDD<C>& lhs, const SDD<C>& rhs)
noexcept
{
  return lhs.ptr() < rhs.ptr();
}

/// @brief   Export the textual representation of an SDD to a stream.
/// @related SDD
///
/// Use only with small SDD, output can be huge.
template <typename C>
std::ostream&
operator<<(std::ostream& os, const SDD<C>& x)
{
  return os << *x;
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Return the |0| terminal.
/// @related SDD
///
/// O(1).
template <typename C>
inline
SDD<C>
zero()
noexcept
{
  return {SDD<C>::zero_ptr()};
}

/// @brief Return the |1| terminal.
/// @related SDD
///
/// O(1).
template <typename C>
inline
SDD<C>
one()
noexcept
{
  return {SDD<C>::one_ptr()};
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related SDD
template <typename C>
std::size_t
check_compatibility(const SDD<C>& lhs, const SDD<C>& rhs)
{
  const auto lhs_index = lhs.index();
  const auto rhs_index = rhs.index();

  if (lhs_index != rhs_index)
  {
    // different type of nodes
    throw top<C>(lhs, rhs);
  }

  typename SDD<C>::variable_type lhs_variable;
  typename SDD<C>::variable_type rhs_variable;

  // we must convert to the right type before comparing variables
  if (lhs_index == SDD<C>::flat_node_index)
  {
    lhs_variable = mem::variant_cast<flat_node<C>>(*lhs).variable();
    rhs_variable = mem::variant_cast<flat_node<C>>(*rhs).variable();
  }
  else
  {
    lhs_variable = mem::variant_cast<hierarchical_node<C>>(*lhs).variable();
    rhs_variable = mem::variant_cast<hierarchical_node<C>>(*rhs).variable();
  }

  if (lhs_variable != rhs_variable)
  {
    throw top<C>(lhs, rhs);
  }

  return lhs_index;
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::dd::SDD.
template <typename C>
struct hash<sdd::SDD<C>>
{
  std::size_t
  operator()(const sdd::SDD<C>& x)
  const noexcept
  {
    return sdd::util::hash(x.ptr());
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#include "sdd/dd/count_combinations.hh"
#include "sdd/dd/path_generator.hh"

#endif // _SDD_DD_DEFINITION_HH_
