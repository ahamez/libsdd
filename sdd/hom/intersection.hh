/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <algorithm>  // all_of, copy
#include <deque>
#include <initializer_list>
#include <iosfwd>
#include <stdexcept>  //invalid_argument
#include <unordered_map>

#include <boost/container/flat_set.hpp>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/identity.hh"
#include "sdd/hom/local.hh"
#include "sdd/order/order.hh"
#include "sdd/util/packed.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief intersection homomorphism.
template <typename C>
struct _intersection
{
  /// @brief The type of the homomorphism operands' set.
  using operands_type = boost::container::flat_set<homomorphism<C>>;

  /// @brief The type of a const iterator on this intersection's operands.
  using const_iterator = typename operands_type::const_iterator;

  /// @brief The homomorphism operands' set.
  const operands_type operands;

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& x)
  const
  {
    dd::intersection_builder<C, SDD<C>> intersection_operands(cxt.sdd_context());
    intersection_operands.reserve(operands.size());
    for (const auto& op : operands)
    {
      auto res = op(cxt, o, x);
      if (res.empty())
      {
        return zero<C>();
      }
      intersection_operands.add(std::move(res));
    }
    return dd::intersection(cxt.sdd_context(), std::move(intersection_operands));
  }

  /// @brief Skip variable predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return std::all_of( operands.begin(), operands.end()
                      , [&o](const homomorphism<C>& h){return h.skip(o);});
  }

  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return std::all_of( operands.begin(), operands.end()
                      , [](const homomorphism<C>& h){return h.selector();});
  }

  /// @brief Get an iterator to the first operand.
  ///
  /// O(1).
  const_iterator
  begin()
  const noexcept
  {
    return operands.begin();
  }

  /// @brief Get an iterator to the end of operands.
  ///
  /// O(1).
  const_iterator
  end()
  const noexcept
  {
    return operands.end();
  }

  friend
  bool
  operator==(const _intersection& lhs, const _intersection& rhs)
  noexcept
  {
    return lhs.operands == rhs.operands;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const _intersection& s)
  {
    os << "(";
    std::copy( s.operands.begin(), std::prev(s.operands.end())
              , std::ostream_iterator<homomorphism<C>>(os, " & "));
    return os << *std::prev(s.operands.end()) << ")";
  }

};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Help optimize an intersection's operands.
template <typename C>
struct intersection_builder
{
  using operands_type = typename _intersection<C>::operands_type;
  using hom_list_type = std::deque<homomorphism<C>> ;
  using locals_type = std::unordered_map< typename C::variable_type, hom_list_type>;

  operands_type& operands_;
  locals_type& locals_;

  /// @brief Flatten nested intersections.
  void
  operator()(const _intersection<C>& s, const homomorphism<C>&)
  const
  {
    for (const auto& op : s.operands)
    {
      visit(*this, op, op);
    }
  }

  /// @brief Regroup locals.
  void
  operator()(const _local<C>& l, const homomorphism<C>&)
  const
  {
    auto insertion = locals_.emplace(l.target, hom_list_type{});
    insertion.first->second.emplace_back(l.h);
  }

  /// @brief Insert normally all other operands.
  template <typename T>
  void
  operator()(const T&, const homomorphism<C>& h)
  const
  {
    operands_.insert(h);
  }
};

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the intersection homomorphism.
/// @related homomorphism
template <typename C, typename InputIterator>
homomorphism<C>
intersection(const order<C>& o, InputIterator begin, InputIterator end)
{
  const auto size = std::distance(begin, end);

  if (size == 0)
  {
    throw std::invalid_argument("Empty operands at intersection construction.");
  }

  typename hom::_intersection<C>::operands_type operands;
  operands.reserve(size);

  typename hom::intersection_builder<C>::locals_type locals;
  hom::intersection_builder<C> ib {operands, locals};
  for (; begin != end; ++begin)
  {
    visit(ib, *begin, *begin);
  }

  // insert remaining locals
  for (const auto& l : locals)
  {
    operands.insert(local<C>(l.first, intersection(o, l.second.begin(), l.second.end())));
  }

  if (operands.size() == 1)
  {
    return *operands.begin();
  }
  else
  {
    operands.shrink_to_fit();
    return hom::make<C, hom::_intersection<C>>(std::move(operands));
  }
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the intersection homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
intersection(const order<C>& o, std::initializer_list<homomorphism<C>> operands)
{
  return intersection(o, operands.begin(), operands.end());
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::_intersection.
template <typename C>
struct hash<sdd::hom::_intersection<C>>
{
  std::size_t
  operator()(const sdd::hom::_intersection<C>& s)
  const
  {
    using namespace sdd::hash;
    return seed() (range(s.operands));
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
