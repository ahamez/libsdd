#pragma once

#include <memory>
#include <vector>

#include <boost/version.hpp>
#if BOOST_VERSION >= 105600
#include <boost/coroutine/asymmetric_coroutine.hpp>
#else
#include <boost/coroutine/all.hpp>
#endif

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
/// Usable in C++11 range-based for loop.
template <typename C>
#if BOOST_VERSION >= 105600
using path_generator = typename boost::coroutines::asymmetric_coroutine<path<C>>::pull_type;
#else
using path_generator = typename boost::coroutines::coroutine<path<C>>::pull_type;
#endif

/// @internal
template <typename C>
#if BOOST_VERSION >= 105600
using path_push_type = typename boost::coroutines::asymmetric_coroutine<path<C>>::push_type;
#else
using path_push_type = typename boost::coroutines::coroutine<path<C>>::push_type;
#endif

/*------------------------------------------------------------------------------------------------*/

namespace dd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @todo Extract in its own file as it also used by Expression.
template <typename C>
struct sdd_stack
{
  const SDD<C> sdd;
  std::shared_ptr<sdd_stack> next;

  sdd_stack(const SDD<C>& s, const std::shared_ptr<sdd_stack>& n)
    : sdd(s), next(n)
  {}
};

/// @internal
template <typename C>
void
paths(path_push_type<C>&, const SDD<C>&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd
