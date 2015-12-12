/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <boost/multiprecision/cpp_int.hpp>

#include "sdd/dd/definition.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
boost::multiprecision::cpp_int
count_combinations(const SDD<C>& x);

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd
