/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <algorithm>  // all_of, copy, equal
#include <deque>
#include <initializer_list>
#include <iosfwd>
#include <stdexcept>  //invalid_argument
#include <unordered_map>

#include <boost/container/flat_set.hpp>

#include "sdd/dd/definition.hh"
#include "sdd/hom/consolidate.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/identity.hh"
#include "sdd/hom/local.hh"
#include "sdd/order/order.hh"
#include "sdd/util/packed.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief sum homomorphism.
template <typename C>
struct LIBSDD_ATTRIBUTE_PACKED _sum
{
  /// @brief The type of a const iterator on this sum's operands.
  using const_iterator = const homomorphism<C>*;

  /// @brief The type deduced from configuration of the number of operands.
  using operands_size_type = typename C::operands_size_type;

  /// @brief The homomorphism's number of operands.
  const operands_size_type size;

  /// @brief Constructor.
  _sum(boost::container::flat_set<homomorphism<C>>& operands)
    : size(static_cast<operands_size_type>(operands.size()))
  {
    // Put all homomorphisms operands right after this sum instance.
    hom::consolidate(operands_addr(), operands.begin(), operands.end());
  }

  /// @brief Destructor.
  ~_sum()
  {
    for (const auto& h : *this)
    {
      h.~homomorphism<C>();
    }
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& x)
  const
  {
    dd::sum_builder<C, SDD<C>> sum_operands(cxt.sdd_context());
    sum_operands.reserve(size);
    for (const auto& op : *this)
    {
      sum_operands.add(op(cxt, o, x));
    }
    return dd::sum(cxt.sdd_context(), std::move(sum_operands));
  }

  /// @brief Skip variable predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return std::all_of(begin(), end(), [&o](const homomorphism<C>& h){return h.skip(o);});
  }


  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return std::all_of(begin(), end(), [](const homomorphism<C>& h){return h.selector();});
  }

  /// @brief Get an iterator to the first operand.
  ///
  /// O(1).
  const_iterator
  begin()
  const noexcept
  {
    return reinterpret_cast<const homomorphism<C>*>(operands_addr());
  }

  /// @brief Get an iterator to the end of operands.
  ///
  /// O(1).
  const_iterator
  end()
  const noexcept
  {
    return reinterpret_cast<const homomorphism<C>*>(operands_addr()) + size;
  }

  /// @brief Get the number of extra bytes.
  ///
  /// These extra bytes correspond to the operands allocated right after this homomorphism.
  std::size_t
  extra_bytes()
  const noexcept
  {
    return size * sizeof(homomorphism<C>);
  }

  friend
  bool
  operator==(const _sum& lhs, const _sum& rhs)
  noexcept
  {
    return lhs.size == rhs.size and std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const _sum& s)
  {
    os << "(";
    std::copy(s.begin(), std::prev(s.end()), std::ostream_iterator<homomorphism<C>>(os, " + "));
    return os << *std::prev(s.end()) << ")";
  }

private:

  /// @brief Return the address of the beginning of the operands.
  char*
  operands_addr()
  const noexcept
  {
    return reinterpret_cast<char*>(const_cast<_sum*>(this)) + sizeof(_sum);
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Help optimize an union's operands.
template <typename C>
struct sum_builder
{
  /// @brief The type of th flat sorted set of operands.
  using operands_type = boost::container::flat_set<homomorphism<C>>;

  /// @brief We use a deque to store the list of homomorphisms as the needed size is unknown.
  using hom_list_type = std::deque<homomorphism<C>>;

  /// @brief Map local homomorphisms to the identifiers they work on.
  using locals_type = std::unordered_map< typename C::variable_type, hom_list_type>;

  /// @brief Store local homomorphisms.
  locals_type& locals_;

  /// @brief Store all other operands.
  operands_type& operands_;

  /// @brief Flatten nested sums.
  void
  operator()(const _sum<C>& s, const homomorphism<C>&)
  const
  {
    for (const auto& op : s)
    {
      visit(*this, op, op);
    }
  }

  /// @brief Regroup locals.
  void
  operator()(const _local<C>& l, const homomorphism<C>&)
  const
  {
    auto insertion = locals_.emplace(l.target, hom_list_type());
    insertion.first->second.emplace_back(l.h);
  }

  /// @brief Insert normally all other operands.
  template <typename T>
  void
  operator()(const T&, const homomorphism<C>& orig)
  const
  {
    operands_.insert(orig);
  }
};

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the sum homomorphism.
/// @related homomorphism
template <typename C, typename InputIterator>
homomorphism<C>
sum(const order<C>& o, InputIterator begin, InputIterator end)
{
  const std::size_t size = std::distance(begin, end);

  if (size == 0)
  {
    throw std::invalid_argument("Empty operands at sum construction.");
  }

  boost::container::flat_set<homomorphism<C>> operands;
  operands.reserve(size);
  typename hom::sum_builder<C>::locals_type locals;
  hom::sum_builder<C> sbv{locals, operands};
  for (; begin != end; ++begin)
  {
    visit(sbv, *begin, *begin);
  }

  // insert remaining locals
  for (const auto& l : locals)
  {
    operands.insert(local<C>(l.first, sum(o, l.second.begin(), l.second.end())));
  }

  if (operands.size() == 1)
  {
    return *operands.begin();
  }
  else
  {
    const std::size_t extra_bytes = operands.size() * sizeof(homomorphism<C>);
    return hom::make_variable_size<C, hom::_sum<C>>(extra_bytes, operands);
  }
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the sum homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
sum(const order<C>& o, std::initializer_list<homomorphism<C>> operands)
{
  return sum(o, operands.begin(), operands.end());
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::_sum.
template <typename C>
struct hash<sdd::hom::_sum<C>>
{
  std::size_t
  operator()(const sdd::hom::_sum<C>& s)
  const
  {
    using namespace sdd::hash;
    return seed() (range(s));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
