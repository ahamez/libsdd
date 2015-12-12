/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <algorithm> // copy, equal
#include <initializer_list>
#include <iosfwd>

#include <boost/container/flat_set.hpp>

#include "sdd/dd/context_fwd.hh"
#include "sdd/dd/definition.hh"
#include "sdd/mem/linear_alloc.hh"
#include "sdd/util/hash.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Base class for sum and intersection operations, used by the cache.
/// @tparam Operation The implementation of the sum or intersection algorithm.
///
/// It manages the allocation and deallocation of operands, as well as the dispatch on the
/// correct type (flat or hierarchical node).
template <typename C, typename Operation>
struct LIBSDD_ATTRIBUTE_PACKED nary_op
{
  // Can't copy a nary_op.
  nary_op(const nary_op&) = delete;
  nary_op& operator=(const nary_op&) = delete;

  /// @brief Define an iterator on operands.
  using const_iterator = const SDD<C>*;

  /// @brief The dynamically allocated array of operands.
  ///
  /// The concrete type will always be an SDD<C>. We use a raw memory storage as we don't want to
  /// build useless default SDD (the |0| terminal). This storage will be filled by the constructor.
  /// Also, as an operation is moved by the cache, we stick to a pointer (contrary to SDD nodes or
  /// homomorphisms n-ary operations) to avoid to move all operands.
  char* operands;

  /// @brief The number of operands.
  const typename C::operands_size_type size;

  /// @brief Constructor from an nary_builder.
  template <typename Builder>
  nary_op(Builder& builder)
    : operands(new char[builder.size_to_allocate()])
    , size(static_cast<typename C::operands_size_type>(builder.size()))
  {
    assert(size > 1);
    // Will place (with a placement new combined with a move) operands in the raw storage.
    builder.consolidate(operands);
  }

  /// @brief Move constructor.
  nary_op(nary_op&& other)
  noexcept
    : operands(other.operands)
    , size(other.size)
  {
    other.operands = nullptr;
  }

  /// @brief Destructor.
  ~nary_op()
  {
    if (operands != nullptr)
    {
      for (auto& operand : *this)
      {
        operand.~SDD<C>();
      }
      delete[] operands;
    }
  }

  /// @brief Get an iterator to the first operand.
  const_iterator
  begin()
  const noexcept
  {
    return reinterpret_cast<const SDD<C>*>(operands);
  }

  /// @brief Get an iterator to the end.
  const_iterator
  end()
  const noexcept
  {
    return reinterpret_cast<const SDD<C>*>(operands) + size;
  }

  /// @brief Apply the operation.
  ///
  /// Called by the cache.
  SDD<C>
  operator()(context<C>& cxt)
  const
  {
    // Compatibility of nodes is checked on the fly by operations.
    // It avoids to perform an iteration only for this task.
    if (mem::is<flat_node<C>>(*begin()))
    {
      return Operation::template work<const_iterator, flat_node<C>>(begin(), end(), cxt);
    }
    else if (mem::is<hierarchical_node<C>>(*begin()))
    {
      return Operation::template work<const_iterator, hierarchical_node<C>>(begin(), end(), cxt);
    }
    else
    {
      throw top<C>(*begin(), *(begin() + 1));
    }
  }

  friend
  bool
  operator==(const nary_op& lhs, const nary_op& rhs)
  noexcept
  {
    return lhs.size == rhs.size and std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const nary_op& x)
  {
    os << Operation::symbol << " (";
    std::copy(x.begin(), std::prev(x.end()), std::ostream_iterator<SDD<C>>(os, ", "));
    return os << *std::prev(x.end()) << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Base type for builders of sum and intersection operations.
///
/// The goal of a this builder is ensure that operands are always stored in the same order
/// (to improve cache hits); and to know the exact number of operands in order to allocate the
/// smallest possible memory to store all of them (this allocation is performed in the
/// construction of operations in nary_op).
template <typename C, typename Valuation, typename Builder>
struct LIBSDD_ATTRIBUTE_PACKED nary_builder
  : private Builder
{
  using builder_type = Builder;
  using set_type = boost::container::flat_set< Valuation, std::less<Valuation>
                                             , mem::linear_alloc<Valuation>>;
  using const_iterator = typename set_type::const_iterator;

  /// @brief Sorted container of operands.
  set_type set_;

  /// @brief Default constructor.
  nary_builder(context<C>& cxt)
    : builder_type()
    , set_(std::less<Valuation>(), mem::linear_alloc<Valuation>(cxt.arena()))
  {}

  /// @brief Construction from a list of operands.
  nary_builder(context<C>& cxt, std::initializer_list<Valuation> operands)
    : builder_type()
    , set_(std::less<Valuation>(), mem::linear_alloc<Valuation>(cxt.arena()))
  {
    set_.reserve(operands.size());
    for (const auto& op : operands)
    {
      add(op);
    }
  }

  /// @brief Request for allocation of additional memory.
  void
  reserve(std::size_t size)
  {
    set_.reserve(size);
  }

  /// @brief Add a new operand.
  void
  add(Valuation&& operand)
  {
    builder_type::add(set_, std::move(operand));
  }

  /// @brief Add a new operand.
  void
  add(const Valuation& operand)
  {
    builder_type::add(set_, operand);
  }

  /// @brief Get an iterator to the first operand.
  const_iterator
  begin()
  const noexcept
  {
    return set_.begin();
  }

  /// @brief Get an iterator to the end.
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

  /// @brief Get the number of contained elements.
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
  consolidate(char* addr)
  noexcept
  {
    Valuation* base = reinterpret_cast<Valuation*>(addr);
    for (auto& elem : set_)
    {
      new (base++) Valuation{std::move(elem)};
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::dd::nary_op
template <typename C, typename Operation>
struct hash<sdd::dd::nary_op<C, Operation>>
{
  std::size_t
  operator()(const sdd::dd::nary_op<C, Operation>& op)
  const
  {
    using namespace sdd::hash;
    return seed() (range(op));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
