#ifndef _SDD_DD_NARY_HH_
#define _SDD_DD_NARY_HH_

/// @cond INTERNAL_DOC

#include <algorithm> // copy, equal
#include <cassert>
#include <initializer_list>
#include <iosfwd>

#include <boost/container/flat_set.hpp>

#include "sdd/dd/definition.hh"
#include "sdd/dd/top.hh"

namespace sdd {

/*-------------------------------------------------------------------------------------------*/

/// @brief Check that all operands are compatible and determine the nodes' type (hierarchical
/// or flat).
template<typename C>
struct check_visitor
{
  typedef node_tag result_type;

  const SDD<C> lhs_;
  const SDD<C> rhs_;

  check_visitor(const SDD<C>& lhs, const SDD<C>& rhs)
  noexcept
    : lhs_(lhs)
    , rhs_(rhs)
  {
  }

  node_tag
  operator()(const flat_node<C>& lhs, const flat_node<C>& rhs)
  const
  {
    if (not (lhs.variable() == rhs.variable()))
    {
      throw top<C>(lhs_, rhs_);
    }
    return node_tag::flat;
  }

  node_tag
  operator()(const hierarchical_node<C>& lhs, const hierarchical_node<C>& rhs)
  const
  {
    if (not (lhs.variable() == rhs.variable()))
    {
      throw top<C>(lhs_, rhs_);
    }
    return node_tag::hierarchical;
  }

  node_tag
  operator()(const one_terminal<C>&, const one_terminal<C>&)
  const noexcept
  {
    assert(false && "More than one |1| in operands.");
    __builtin_unreachable();
  }

  template <typename T, typename U>
  __attribute__((noreturn))
  node_tag
  operator()(const T&, const U&)
  const
  {
    throw top<C>(lhs_, rhs_);
  }
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Base class for sum and intersection operations, used by the cache.
///
/// It manages the allocation and deallocation of operands, as well as the dispatch on the
/// correct type (flat or hierarchical node). Use CRTP.
/// As it's an internally used structure, we don't bother with private and public sections.
template <typename C, typename Operation>
struct _LIBSDD_ATTRIBUTE_PACKED nary_base
{
  // Can't copy a nary_base.
  nary_base(const nary_base&) = delete;
  nary_base& operator=(const nary_base&) = delete;

  /// Used by the cache to know the type of the result.
  typedef SDD<C> result_type;

  /// To iterate on operands.
  typedef const SDD<C>* const_iterator;

  /// The evaluation context.
  context<C>& cxt_;

  /// The dynamically allocated array of operands.
  char* operands_;

  /// The number of operands.
  const typename C::operands_size_type size_;

  template <typename Builder>
  nary_base(context<C>& cxt, Builder& builder)
    : cxt_(cxt)
    , operands_(new char[builder.size_to_allocate()])
    , size_(static_cast<typename C::operands_size_type>(builder.size()))
  {
    builder.consolidate(operands_);
  }

  nary_base(nary_base&& other)
  noexcept
    : cxt_(other.cxt_)
    , operands_(other.operands_)
    , size_(other.size_)
  {
    other.operands_ = nullptr;
  }

  ~nary_base()
  {
    if (operands_ != nullptr)
    {
      for (auto& operand : *this)
      {
        operand.~SDD<C>();
      }
      delete[] operands_;
    }
  }

  const_iterator
  begin()
  const noexcept
  {
    return reinterpret_cast<const SDD<C>*>(operands_);
  }

  const_iterator
  end()
  const noexcept
  {
    return reinterpret_cast<const SDD<C>*>(operands_) + size_;
  }

  SDD<C>
  operator()()
  const
  {
    // Check compatibility of operands, size is necessarily at least 2.
    auto cit = begin();
    auto last = end() - 1;
    node_tag tag = node_tag::flat;
    for (; cit != last; ++cit)
    {
      tag = apply_visitor( check_visitor<C>(*cit, *(cit + 1))
                         , (*cit)->data(), (*(cit + 1))->data());
    }
    // Dispatch on the function that does the real work with the deduced type.
    const Operation* impl = static_cast<const Operation*>(this);
    if (tag == node_tag::flat)
    {
      return impl->template work<node_tag::flat>();
    }
    else
    {
      return impl->template work<node_tag::hierarchical>();
    }
  }
};

/*-------------------------------------------------------------------------------------------*/

/// @brief   Equality of two operations based on nary_base.
/// @related nary_base
template <typename C, typename Operation>
inline
bool
operator==(const nary_base<C, Operation>& lhs, const nary_base<C, Operation>& rhs)
noexcept
{
  return lhs.size_ == rhs.size_ and std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

/// @related nary_base
template <typename C, typename Operation>
std::ostream&
operator<<(std::ostream& os, const nary_base<C, Operation>& x)
{
  os << Operation::symbol() << " (";
  std::copy(x.begin(), std::prev(x.end()), std::ostream_iterator<SDD<C>>(os, ", "));
  return os << std::prev(x.end()) << ")";
}

/*-------------------------------------------------------------------------------------------*/

/// @brief Base type for builders of sum and intersection operations.
///
/// The goal of a this builder is ensure that operands are always stored in the same order
/// (to improve cache hits); and to know the exact number of operands in order to allocate the
/// smallest possible memory to store all of them (this allocation is performed in the
/// construction of operations in nary_base).
template <typename Valuation, typename Builder>
struct _LIBSDD_ATTRIBUTE_PACKED nary_builder
{
  typedef boost::container::flat_set<Valuation> set_type;
  typedef typename set_type::const_iterator const_iterator;

  Builder builder_;
  set_type set_;

  nary_builder()
    : builder_()
    , set_()
  {
  }

  nary_builder(std::initializer_list<Valuation> operands)
    : builder_()
    , set_()
  {
    set_.reserve(operands.size());
    for (const auto& op : operands)
    {
      add(op);
    }
  }

  void
  add(Valuation&& operand)
  {
    builder_.add(set_, std::move(operand));
  }

  void
  add(const Valuation& operand)
  {
    builder_.add(set_, operand);
  }

  /// @brief Request for allocation of additional memory.
  void
  reserve(std::size_t size)
  noexcept
  {
    set_.reserve(size);
  }

  const_iterator
  begin()
  const noexcept
  {
    return set_.begin();
  }

  const_iterator
  end()
  const noexcept
  {
    return set_.end();
  }

  /// @brief Tell if this build doesn't contain any node.
  bool
  empty()
  const noexcept
  {
    return set_.empty();
  }

  std::size_t
  size()
  const noexcept
  {
    return set_.size();
  }

  /// @brief Compute the size needed to store all the operands contained by this builder.
  std::size_t
  size_to_allocate()
  const noexcept
  {
    return set_.size() * sizeof(Valuation);
  }

  /// @brief Move operands of this builder to a given memory location.
  /// @param addr shall point to an allocated memory location of the size returned by
  /// size_to_allocate().
  ///
  /// Once performed, all subsequent calls to this instance are invalid.
  void
  consolidate(void* addr)
  noexcept
  {
    Valuation* base = reinterpret_cast<Valuation*>(addr);
    std::size_t i = 0;
    for (auto it = set_.begin(); it != set_.end(); ++it, ++i)
    {
      new (base + i) Valuation(std::move(*it));
    }
  }
};

/*-------------------------------------------------------------------------------------------*/

} // namespace sdd

/// @endcond

#endif // _SDD_DD_NARY_HH_
