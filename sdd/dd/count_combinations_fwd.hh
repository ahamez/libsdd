#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wshadow"
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
#include <boost/multiprecision/cpp_int.hpp>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#pragma GCC diagnostic pop

#include "sdd/dd/definition.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
boost::multiprecision::cpp_int
count_combinations(const SDD<C>& x);

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd
