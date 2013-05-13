#ifndef _SDD_HOM_INTERSECTION_HH_
#define _SDD_HOM_INTERSECTION_HH_

#include <algorithm>  // all_of, copy
#include <deque>
#include <initializer_list>
#include <iosfwd>
#include <stdexcept>  //invalid_argument
#include <unordered_map>

#include <boost/container/flat_set.hpp>

#include "sdd/dd/definition.hh"
#include "sdd/dd/top.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation_error.hh"
#include "sdd/hom/identity.hh"
#include "sdd/hom/local.hh"
#include "sdd/order/order.hh"
#include "sdd/util/packed.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Intersection homomorphism.
template <typename C>
class intersection
{
public:

  /// @brief The type of the homomorphism operands' set.
  typedef boost::container::flat_set<homomorphism<C>> operands_type;

private:

  /// @brief The homomorphism operands' set.
  const operands_type operands_;

public:

  /// @brief Constructor.
  intersection(operands_type&& operands)
    : operands_(std::move(operands))
  {
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& x)
  const
  {
    dd::intersection_builder<C, SDD<C>> intersection_operands(operands_.size());
    for (const auto& op : operands_)
    {
      intersection_operands.add(op(cxt, o, x));
    }
    try
    {
      return dd::intersection(cxt.sdd_context(), std::move(intersection_operands));
    }
    catch (top<C>& t)
    {
      evaluation_error<C> e(x);
      e.add_top(t);
      throw e;
    }
  }

  /// @brief Skip variable predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return std::all_of( operands_.begin(), operands_.end()
                      , [&o](const homomorphism<C>& h){return h.skip(o);});
  }


  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return std::all_of( operands_.begin(), operands_.end()
                      , [](const homomorphism<C>& h){return h.selector();});
  }

  const operands_type&
  operands()
  const noexcept
  {
    return operands_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Equality of two intersection.
/// @related intersection
template <typename C>
inline
bool
operator==(const intersection<C>& lhs, const intersection<C>& rhs)
noexcept
{
  return lhs.operands() == rhs.operands();
}

/// @internal
/// @related intersection
template <typename C>
std::ostream&
operator<<(std::ostream& os, const intersection<C>& s)
{
  os << "(";
  std::copy( s.operands().begin(), std::prev(s.operands().end())
           , std::ostream_iterator<homomorphism<C>>(os, " & "));
  return os << *std::prev(s.operands().end()) << ")";
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Help optimize an intersection's operands.
template <typename C>
struct intersection_builder_helper
{
  typedef void result_type;
  typedef typename intersection<C>::operands_type operands_type;
  typedef std::deque<homomorphism<C>> hom_list_type;
  typedef std::unordered_map<typename C::Identifier, hom_list_type> locals_type;

  /// @brief Flatten nested intersections.
  void
  operator()( const intersection<C>& s
            , const homomorphism<C>&, operands_type& operands, locals_type& locals)
  const
  {
    for (const auto& op : s.operands())
    {
      apply_visitor(*this, op->data(), op, operands, locals);
    }
  }

  /// @brief Regroup locals.
  void
  operator()( const local<C>& l
            , const homomorphism<C>&, operands_type&, locals_type& locals)
  const
  {
    auto insertion = locals.emplace(l.identifier(), hom_list_type());
    insertion.first->second.emplace_back(l.hom());
  }

  /// @brief Insert normally all other operands.
  template <typename T>
  void
  operator()(const T&, const homomorphism<C>& h, operands_type& operands, locals_type&)
  const
  {
    operands.insert(h);
  }

};

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the Intersection homomorphism.
/// @related homomorphism
template <typename C, typename InputIterator>
homomorphism<C>
Intersection(const order<C>& o, InputIterator begin, InputIterator end)
{
  const std::size_t size = std::distance(begin, end);

  if (size == 0)
  {
    throw std::invalid_argument("Empty operands at Intersection construction.");
  }

  typename hom::intersection<C>::operands_type operands;
  operands.reserve(size);

  hom::intersection_builder_helper<C> ib;
  typename hom::intersection_builder_helper<C>::locals_type locals;
  for (; begin != end; ++begin)
  {
    apply_visitor(ib, (*begin)->data(), *begin, operands, locals);
  }

  // insert remaining locals
  for (const auto& l : locals)
  {
    operands.insert(Local<C>(l.first, o, Intersection<C>(o, l.second.begin(), l.second.end())));
  }

  if (operands.size() == 1)
  {
    return *operands.begin();
  }
  else
  {
    operands.shrink_to_fit();
    return homomorphism<C>::create(mem::construct<hom::intersection<C>>(), std::move(operands));
  }
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the Intersection homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
Intersection(const order<C>& o, std::initializer_list<homomorphism<C>> operands)
{
  return Intersection<C>(o, operands.begin(), operands.end());
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::intersection.
template <typename C>
struct hash<sdd::hom::intersection<C>>
{
  std::size_t
  operator()(const sdd::hom::intersection<C>& s)
  const noexcept
  {
    std::size_t seed = 0;
    for (const auto& op : s.operands())
    {
      sdd::util::hash_combine(seed, op);
    }
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_INTERSECTION_HH_
