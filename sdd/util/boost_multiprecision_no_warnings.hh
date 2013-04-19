#ifndef _SDD_UTIL_BOOST_MULTIPRECISION_NO_WARNINGS_HH_
#define _SDD_UTIL_BOOST_MULTIPRECISION_NO_WARNINGS_HH_

/*------------------------------------------------------------------------------------------------*/

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

#endif // _SDD_UTIL_BOOST_MULTIPRECISION_NO_WARNINGS_HH_
