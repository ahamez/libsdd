#pragma once

#include "sdd/dd/definition.hh"
#include "sdd/util/boost_multiprecision_no_warnings.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
inline
boost::multiprecision::cpp_int
count_combinations(const SDD<C>& x);

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd
