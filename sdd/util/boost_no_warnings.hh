#ifndef _SDD_UTIL_BOOST_HH_
#define _SDD_UTIL_BOOST_HH_

/*------------------------------------------------------------------------------------------------*/

#include <boost/iterator/transform_iterator.hpp>

/*------------------------------------------------------------------------------------------------*/
// Boost.MultiIndex

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wshadow"

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/key_extractors.hpp>
#include <boost/multi_index/ordered_index.hpp>

#pragma GCC diagnostic pop

/*------------------------------------------------------------------------------------------------*/
// Boost.Multiprecision

#pragma GCC diagnostic push

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif

#if defined(__clang__)
# pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif

#pragma GCC diagnostic ignored "-Wshadow"

#include <boost/multiprecision/cpp_int.hpp>

#pragma GCC diagnostic pop

/*------------------------------------------------------------------------------------------------*/

#endif // _SDD_UTIL_BOOST_HH_
