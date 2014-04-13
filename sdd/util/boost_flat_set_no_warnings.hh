#ifndef _SDD_UTIL_BOOST_FLAT_SET_NO_WARNINGS_HH_
#define _SDD_UTIL_BOOST_FLAT_SET_NO_WARNINGS_HH_

/*------------------------------------------------------------------------------------------------*/

#pragma GCC diagnostic push
#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#include <boost/container/flat_set.hpp>
#pragma GCC diagnostic pop

/*------------------------------------------------------------------------------------------------*/

#endif // _SDD_UTIL_BOOST_FLAT_SET_NO_WARNINGS_HH_
