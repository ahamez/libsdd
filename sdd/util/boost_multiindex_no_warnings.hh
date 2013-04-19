#ifndef _SDD_UTIL_BOOST_MULTIINDEX_NO_WARNINGS_HH_
#define _SDD_UTIL_BOOST_MULTIINDEX_NO_WARNINGS_HH_

/*------------------------------------------------------------------------------------------------*/

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

#endif // _SDD_UTIL_BOOST_MULTIINDEX_NO_WARNINGS_HH_
