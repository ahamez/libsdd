#ifndef _SDD_HOM_SUM_HH_
#define _SDD_HOM_SUM_HH_

#include <algorithm>  // all_of, equal
#include <initializer_list>

#include <boost/container/flat_set.hpp>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/identity.hh"

namespace sdd { namespace hom {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Sum homomorphism.
template <typename C>
class sum
{
public:

  /// @brief The set of homomorphism operands
  typedef boost::container::flat_set<homomorphism<C>> operands_type;

private:

  const operands_type operands_;

public:

  /// @brief Constructor.
  sum(operands_type&& operands)
    : operands_(std::move(operands))
  {
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const SDD<C>& x)
  const
  {
    sum_builder<C, SDD<C>> sum_operands(operands_.size());
    for (const auto& op : operands_)
    {
      sum_operands.add(op(cxt, x));
    }
    return sdd::sum(cxt.sdd_context(), std::move(sum_operands));
  }

  /// @brief Skip variable predicate.
  bool
  skip(const typename C::Variable& v)
  const noexcept
  {
    return std::all_of( operands_.begin(), operands_.end()
                      , [&](const homomorphism<C>& h){return h.skip(v);});
  }

  const operands_type&
  operands()
  const noexcept
  {
    return operands_;
  }
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Equality of two Sum homomorphisms.
/// @related Sum
template <typename C>
inline
bool
operator==(const sum<C>& lhs, const sum<C>& rhs)
noexcept
{
  return lhs.operands().size() == rhs.operands().size()
     and std::equal(lhs.operands().begin(), lhs.operands().end(), rhs.operands().begin());
}

/// @endcond

/*-------------------------------------------------------------------------------------------*/

/// @brief Create the Sum homomorphism.
/// @related homomorphism
template <typename C, typename InputIterator>
homomorphism<C>
Sum(InputIterator begin, InputIterator end)
{
  const std::size_t size = std::distance(begin, end);

  if (size == 0)
  {
    return Id<C>();
  }

  typename sum<C>::operands_type operands;
  operands.reserve(size);

  for (; begin != end; ++begin)
  {
    operands.insert(*begin);
  }

  if (operands.size() == 1)
  {
    return *operands.begin();
  }
  else
  {
    return homomorphism<C>::create(internal::mem::construct<sum<C>>(), std::move(operands));
  }
}

/*-------------------------------------------------------------------------------------------*/

/// @brief Create the Sum homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
Sum(std::initializer_list<homomorphism<C>> operands)
{
  return Sum<C>(operands.begin(), operands.end());
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

namespace std {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Hash specialization for sdd::hom::sum.
template <typename C>
struct hash<sdd::hom::sum<C>>
{
  std::size_t
  operator()(const sdd::hom::sum<C>& s)
  const noexcept
  {
    std::size_t seed = 0;
    for (const auto& op : s.operands())
    {
      sdd::internal::util::hash_combine(seed, op);
    }
    return seed;
  }
};

/// @endcond

/*-------------------------------------------------------------------------------------------*/

} // namespace std


#endif // _SDD_HOM_SUM_HH_
