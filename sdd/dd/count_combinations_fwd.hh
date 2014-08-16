#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wshadow"
#include <boost/multiprecision/cpp_int.hpp>
#pragma GCC diagnostic pop

#include "sdd/dd/definition.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
inline
boost::multiprecision::cpp_int
count_combinations(const SDD<C>& x);

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd
