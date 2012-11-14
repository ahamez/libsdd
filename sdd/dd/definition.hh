#ifndef _SDD_DD_DEFINITION_HH_
#define _SDD_DD_DEFINITION_HH_

/// @file definition.hh
/// @brief Contain all stuff necessary to describe and build SDD.

#include <initializer_list>

#include "sdd/dd/alpha.hh"
#include "sdd/dd/definition_fwd.hh"
#include "sdd/dd/node.hh"
#include "sdd/dd/terminal.hh"
#include "sdd/internal/mem/ptr.hh"
#include "sdd/internal/mem/ref_counted.hh"
#include "sdd/internal/mem/variant.hh"
#include "sdd/internal/util/print_sizes_fwd.hh"

namespace sdd {

/*-------------------------------------------------------------------------------------------*/

/// @brief SDD at the deepest level.
template <typename C>
using flat_node = node<C, typename C::Values>;

/// @brief All but SDD at the deepest level.
template <typename C>
using hierarchical_node = node<C, SDD<C>>;

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Tag to describe the type of a node.
enum class node_tag {flat, hierarchical};

/// @brief Signature of the meta-function that returns the node's type corresponding to the
/// given tag.
template <typename C, enum node_tag>
struct node_for_tag;

/// @brief Specialization for flat node.
template <typename C>
struct node_for_tag<C, node_tag::flat>
{
  typedef flat_node<C> type;
};

/// @brief Specialization for hierarchical node.
template <typename C>
struct node_for_tag<C, node_tag::hierarchical>
{
  typedef hierarchical_node<C> type;
};

/// @endcond

/*-------------------------------------------------------------------------------------------*/

/// @brief Hierarchical Set Decision Diagram.
template <typename C>
class SDD
{
private:

  /// @brief A canonized SDD.
  ///
  /// This is the real recursive definition of an SDD: it can be a |0| or |1| terminal, or it
  /// can be a flat or an hierachical node.
  typedef internal::mem::variant< const zero_terminal<C>, const one_terminal<C>
                                , const flat_node<C>, const hierarchical_node<C>>
          SDD_data;

  /// @brief A unified and canonized SDD, meant to be stored in a unique table.
  ///
  /// It is automatically erased when there is no more reference to it.
  typedef internal::mem::ref_counted<const SDD_data> SDD_unique;

  /// @brief Define the smart pointer around a unified SDD.
  ///
  /// It handles the reference counting as well as the deletion of the SDD when it is no longer
  /// referenced.
  typedef internal::mem::ptr<const SDD_unique> ptr_type;

public:

  /// @brief The type of variables.
  typedef typename C::Variable variable_type;

  /// @brief The type of a set of values.
  typedef typename C::Values values_type;

  /// @brief The type of a value in a set of values.
  typedef typename C::Values::value_type value_type;

private:

  /// @brief The real smart pointer around a unified SDD.
  ptr_type ptr_;

public:

  /// @brief Move constructor.
  ///
  /// O(1).
  SDD(SDD&&) noexcept = default;

  /// @brief Move operator.
  ///
  /// O(1).
  SDD&
  operator=(SDD&&) noexcept = default;

  /// @brief Copy constructor.
  ///
  /// O(1).
  SDD(const SDD&) noexcept = default;

  /// @brief Copy operator.
  ///
  /// O(1).
  SDD&
  operator=(const SDD&) noexcept = default;

  /// @brief  Construct a terminal.
  /// @param  terminal If true, create the |1| terminal; the |0| terminal otherwise.
  /// @return The terminal |0| or |1|.
  ///
  /// O(1).
  SDD(bool terminal)
  noexcept
    : ptr_(terminal ? one_ptr() : zero_ptr())
  {
  }

  /// @brief Construct a hierarchical SDD.
  /// @param var  The SDD's variable.
  /// @param val  The SDD's valuation, a set of values constructed from an initialization list.
  /// @param succ The SDD's successor.
  ///
  /// O(1), for the creation of the SDD itself, but the complexity of the construction of the
  /// set of values depends on values_type.
  SDD(const variable_type& var, std::initializer_list<value_type> values, const SDD& succ)
    : ptr_(create_node(var, values_type(values), SDD(succ)))
  {
  }

  /// @brief Construct a flat SDD.
  /// @param var  The SDD's variable.
  /// @param val  The SDD's valuation, a set of values.
  /// @param succ The SDD's successor.
  ///
  /// O(1).
  SDD(const variable_type& var, values_type&& val, const SDD& succ)
    : ptr_(create_node(var, std::move(val), succ))
  {
  }

  /// @brief Construct a flat SDD.
  /// @param var  The SDD's variable.
  /// @param val  The SDD's valuation, a set of values.
  /// @param succ The SDD's successor.
  ///
  /// O(1).
  SDD(const variable_type& var, const values_type& val, const SDD& succ)
    : ptr_(create_node(var, val, succ))
  {
  }

  /// @brief Construct a hierarchical SDD.
  /// @param var  The SDD's variable.
  /// @param val  The SDD's valuation, an SDD in this case.
  /// @param succ The SDD's successor.
  ///
  /// O(1).
  SDD(const variable_type& var, const SDD& val, const SDD& succ)
    : ptr_(create_node(var, val, succ))
  {
  }

  /// @brief  Indicate if the SDD is |0|.
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

/// @cond INTERNAL_DOC

  /// @brief Construct an SDD from a ptr.
  ///
  /// O(1).
  SDD(const ptr_type& ptr)
  noexcept
    : ptr_(ptr)
  {
  }

  /// @brief Construct an SDD from a moved ptr.
  ///
  /// O(1).
  SDD(ptr_type&& ptr)
  noexcept
    : ptr_(std::move(ptr))
  {
  }

  /// @brief  Construct an SDD, flat or hierarchical, with an alpha.
  /// \tparam Valuation If an SDD, constructs a hierarchical SDD; if a set of values,
  /// constructs a flat SDD.
  ///
  /// O(n) where n is the number of arcs in the builder.
  template <typename Valuation>
  SDD(const variable_type& var, alpha_builder<C, Valuation>&& builder)
    : ptr_(create_node(var, std::move(builder)))
  {
  }

  /// @brief Get the content of the SDD (an internal::mem::ref_counted).
  ///
  /// O(1).
  const SDD_unique&
  operator*()
  const noexcept
  {
    return *ptr_;
  }

  /// @brief Get a pointer to the content of the SDD (an internal::mem::ref_counted).
  ///
  /// O(1).
  const SDD_unique*
  operator->()
  const noexcept
  {
    return ptr_.operator->();
  }

  /// @brief Get the real smart pointer of the unified data.
  ///
  /// O(1).
  ptr_type
  ptr()
  const noexcept
  {
    return ptr_;
  }

  /// @brief Create the |0| terminal.
  ///
  /// O(1). The |0| is cached in a static variable.
  static
  ptr_type
  zero_ptr()
  {
    static SDD_unique* z = new SDD_unique(internal::mem::construct<zero_terminal<C>>());
    static const ptr_type zero(internal::mem::unify(z, sizeof(SDD_unique)));
    return zero;
  }

  /// @brief Create the |1| terminal.
  ///
  /// O(1). The |1| is cached in a static variable.
  static
  ptr_type
  one_ptr()
  {
    static SDD_unique* o = new SDD_unique(internal::mem::construct<one_terminal<C>>());
    static const ptr_type one(internal::mem::unify(o, sizeof(SDD_unique)));
    return one;
  }

private:

  /// @brief Helper function to create a node, flat or hierarchical, with only one arc.
  ///
  /// O(1).
  template <typename Valuation>
  static
  ptr_type
  create_node(const variable_type& var, Valuation&& val, const SDD& succ)
  {
    if (succ.empty() or val.empty())
    {
      return zero_ptr();
    }
    else
    {
      alpha_builder<C, Valuation> builder;
      builder.add(std::move(val), succ);
      return unify_node<Valuation>(var, std::move(builder));
    }
  }

  /// @brief Helper function to create a node, flat or hierarchical, with only one arc.
  ///
  /// O(1).
  template <typename Valuation>
  static
  ptr_type
  create_node(const variable_type& var, const Valuation& val, const SDD& succ)
  {
    if (succ.empty() or val.empty())
    {
      return zero_ptr();
    }
    else
    {
      alpha_builder<C, Valuation> builder;
      builder.add(val, succ);
      return unify_node<Valuation>(var, std::move(builder));
    }
  }

  /// @brief Helper function to create a node, flat or hierarchical, from an alpha.
  ///
  /// O(n) where n is the number of arcs in the builder.
  template <typename Valuation>
  static
  ptr_type
  create_node(const variable_type& var, alpha_builder<C, Valuation>&& builder)
  {
    if (builder.empty())
    {
      return zero_ptr();
    }
    else
    {
      return unify_node<Valuation>(var, std::move(builder));
    }
  }

  /// @brief Helper function to unify a node, flat or hierarchical, from an alpha.
  ///
  /// O(n) where n is the number of arcs in the builder.
  template <typename Valuation>
  static
  const SDD_unique&
  unify_node(const variable_type& var, alpha_builder<C, Valuation>&& builder)
  {
    // Will be erased by the unicity table, either it's an already existing node or a deletion
    // is requested by ptr.
    // Note that the alpha function is allocated right behind the node, thus extra care must be
    // taken. This is also why we use Boost.Intrusive in order to be able to manage memory
    // exactly the way we want.
    const std::size_t size = sizeof(SDD_unique) + builder.size_to_allocate();
    char* addr = internal::mem::allocate<SDD_unique>(size);
    SDD_unique* u =
      new (addr) SDD_unique(internal::mem::construct<node<C, Valuation>>(), var, builder);
    return internal::mem::unify(u, size);
  }

  friend void internal::util::print_sizes<C>(std::ostream&);

/// @endcond
};

/*-------------------------------------------------------------------------------------------*/

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
  return os << x->data();
}

/*-------------------------------------------------------------------------------------------*/

/// @brief   Return the |0| terminal.
/// @related SDD
///
/// O(1).
template <typename C>
inline
SDD<C>
zero()
noexcept
{
  return SDD<C>(false);
}

/// @brief   Return the |1| terminal.
/// @related SDD
///
/// O(1).
template <typename C>
inline
SDD<C>
one()
noexcept
{
  return SDD<C>(true);
}

/*-------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*-------------------------------------------------------------------------------------------*/

/// @brief Hash specialization for sdd::dd::SDD.
template <typename C>
struct hash<sdd::SDD<C>>
{
  std::size_t
  operator()(const sdd::SDD<C>& x)
  const noexcept
  {
    return std::hash<decltype(x.ptr())>()(x.ptr());
  }
};

/*-------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_DD_DEFINITION_HH_
