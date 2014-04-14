#ifndef _SDD_DD_COUNT_COMBINATIONS_FWD_HH_
#define _SDD_DD_COUNT_COMBINATIONS_FWD_HH_

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

#endif // _SDD_DD_COUNT_COMBINATIONS_FWD_HH_
