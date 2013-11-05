#ifndef _SDD_DD_PATH_GENERATOR_FWD_HH_
#define _SDD_DD_PATH_GENERATOR_FWD_HH_

#if !defined(HAS_NO_BOOST_COROUTINE)

#include <memory>
#include <vector>

#include <boost/coroutine/coroutine.hpp>

#include "sdd/dd/definition_fwd.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief Describe a path in an SDD.
///
/// Values are indexed from the top of the SDD to its bottom.
template <typename C>
using path = std::vector<typename C::Values>;

/// @brief An on-the-fly generator of all paths contained in an SDD.
///
/// Iterators on it return a const path<C>&.
template <typename C>
using path_generator = boost::coroutines::coroutine<const path<C>&()>;

/*------------------------------------------------------------------------------------------------*/

namespace dd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
struct sdd_stack
{
  const SDD<C> sdd;
  std::shared_ptr<sdd_stack> next;

  sdd_stack(const SDD<C>& s, const std::shared_ptr<sdd_stack>& n)
    : sdd(s), next(n)
  {}
};

// Forward declaration.
template <typename C>
void
xpaths_impl( typename path_generator<C>::caller_type&
           , SDD<C>, std::shared_ptr<path<C>>, std::shared_ptr<sdd_stack<C>>);

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd

#endif // !defined(HAS_NO_BOOST_COROUTINE)

#endif // _SDD_DD_PATH_GENERATOR_FWD_HH_
