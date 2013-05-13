#ifndef _SDD_DD_NARY_HH_
#define _SDD_DD_NARY_HH_

#include <algorithm> // copy, equal
#include <cassert>
#include <initializer_list>
#include <iosfwd>

#include <boost/container/flat_set.hpp>

#include "sdd/dd/context_fwd.hh"
#include "sdd/dd/definition.hh"
#include "sdd/dd/top.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Check that all operands are compatible and determine the nodes' type (hierarchical
/// or flat).
template<typename C>
struct check_visitor
{
  typedef node_tag result_type;

  node_tag
  operator()( const flat_node<C>& lhs, const flat_node<C>& rhs
            , const SDD<C>& sdd_lhs, const SDD<C>& sdd_rhs)
  const
  {
    if (not (lhs.variable() == rhs.variable()))
    {
      throw top<C>(sdd_lhs, sdd_rhs);
    }
    return node_tag::flat;
  }

  node_tag
  operator()( const hierarchical_node<C>& lhs, const hierarchical_node<C>& rhs
            , const SDD<C>& sdd_lhs, const SDD<C>& sdd_rhs)
  const
  {
    if (not (lhs.variable() == rhs.variable()))
    {
      throw top<C>(sdd_lhs, sdd_rhs);
    }
    return node_tag::hierarchical;
  }

  node_tag
  operator()(const one_terminal<C>&, const one_terminal<C>&, const SDD<C>&, const SDD<C>&)
  const noexcept
  {
    assert(false && "More than one |1| in operands.");
    __builtin_unreachable();
  }

  template <typename T, typename U>
  __attribute__((noreturn))
  node_tag
  operator()(const T&, const U&, const SDD<C>& sdd_lhs, const SDD<C>& sdd_rhs)
  const
  {
    throw top<C>(sdd_lhs, sdd_rhs);
  }
};

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

  /// @brief Used by the cache to know the type of the result.
  typedef SDD<C> result_type;

  /// @brief Define an iterator on operands.
  typedef const SDD<C>* const_iterator;

  /// @brief The dynamically allocated array of operands.
  ///
  /// The concrete type will always be an SDD<C>. But we need a raw memory storage as SDD<C> doesn't
  /// have a default constructor. This storage will be filled by the constructor.
  char* operands;

  /// @brief The number of operands.
  const typename C::operands_size_type size;

  /// @brief Constructor from an nary_builder.
  template <typename Builder>
  nary_op(Builder& builder)
    : operands(new char[builder.size_to_allocate()])
    , size(static_cast<typename C::operands_size_type>(builder.size()))
  {
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
    // Check compatibility of operands, size is necessarily at least 2.
    auto cit = begin();
    auto last = end() - 1;
    node_tag tag = node_tag::flat;
    for (; cit != last; ++cit)
    {
      tag = apply_binary_visitor( check_visitor<C>()
                                , (*cit)->data(), (*(cit + 1))->data()
                                , *cit, *(cit + 1));
    }

    if (tag == node_tag::flat)
    {
      typedef typename node_for_tag<C, node_tag::flat>::type node_type;
      return Operation::template work<const_iterator, node_type>(begin(), end(), cxt);
    }
    else
    {
      typedef typename node_for_tag<C, node_tag::hierarchical>::type node_type;
      return Operation::template work<const_iterator, node_type>(begin(), end(), cxt);
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Equality of two operations based on nary_op.
/// @related nary_op
template <typename C, typename Operation>
inline
bool
operator==(const nary_op<C, Operation>& lhs, const nary_op<C, Operation>& rhs)
noexcept
{
  return lhs.size == rhs.size and std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

/// @internal
/// @related nary_op
template <typename C, typename Operation>
std::ostream&
operator<<(std::ostream& os, const nary_op<C, Operation>& x)
{
  os << Operation::symbol() << " (";
  std::copy(x.begin(), std::prev(x.end()), std::ostream_iterator<SDD<C>>(os, ", "));
  return os << *std::prev(x.end()) << ")";
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Base type for builders of sum and intersection operations.
///
/// The goal of a this builder is ensure that operands are always stored in the same order
/// (to improve cache hits); and to know the exact number of operands in order to allocate the
/// smallest possible memory to store all of them (this allocation is performed in the
/// construction of operations in nary_op).
template <typename Valuation, typename Builder>
struct LIBSDD_ATTRIBUTE_PACKED nary_builder
{
  typedef boost::container::flat_set<Valuation> set_type;
  typedef typename set_type::const_iterator const_iterator;

  /// @brief The policy to add new operands.
  ///
  /// An instance is needed for builders with a state (actually, the intersection builder).
  Builder builder_;

  /// @brief Sorted container of operands.
  set_type set_;

  /// @brief Default constructor.
  nary_builder()
    : builder_()
    , set_()
  {
  }

  /// @brief Construction with a reserve request.
  nary_builder(std::size_t size)
    : builder_()
    , set_()
  {
    set_.reserve(size);
  }

  /// @brief Construction from a list of operands.
  nary_builder(std::initializer_list<Valuation> operands)
    : nary_builder(operands.size())
  {
    for (const auto& op : operands)
    {
      add(op);
    }
  }

  /// @brief Add a new operand.
  void
  add(Valuation&& operand)
  {
    builder_.add(set_, std::move(operand));
  }

  /// @brief Add a new operand.
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
    std::size_t i = 0;
    for (auto it = set_.begin(); it != set_.end(); ++it, ++i)
    {
      new (base + i) Valuation(std::move(*it));
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
  const noexcept
  {
    std::size_t seed = 0;
    for (const auto& operand : op)
    {
      sdd::util::hash_combine(seed, operand);
    }
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_DD_NARY_HH_
