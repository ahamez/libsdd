#ifndef _SDD_DD_OPERATIONS_FWD_HH_
#define _SDD_DD_OPERATIONS_FWD_HH_

#include "sdd/dd/context_fwd.hh"
#include "sdd/dd/definition_fwd.hh"
#include "sdd/dd/nary_fwd.hh"

/// @file operations_fwd.hh
/// @brief Contain all necessary forward declarations of operations.
///
/// It permits operations to call each other recursively.

namespace sdd { namespace dd {

/*-------------------------------------------------------------------------------------------*/

template <typename C>
SDD<C>
difference(context<C>&, const SDD<C>&, const SDD<C>&);

template <typename C, typename Values>
Values
difference(context<C>&, const Values&, const Values&);

/*-------------------------------------------------------------------------------------------*/

template <typename C, typename Valuation>
struct intersection_builder_impl;

template <typename C, typename Valuation>
using intersection_builder = nary_builder<Valuation, intersection_builder_impl<C, Valuation>>;

template <typename C>
SDD<C>
intersection(context<C>&, intersection_builder<C, SDD<C>>&&);

template <typename C, typename Values>
Values
intersection(context<C>&, const intersection_builder<C, Values>&);

/*-------------------------------------------------------------------------------------------*/

template <typename C, typename Valuation>
struct sum_builder_impl;

template <typename C, typename Valuation>
using sum_builder = nary_builder<Valuation, sum_builder_impl<C, Valuation>>;

template <typename C>
SDD<C>
sum(context<C>&, sum_builder<C, SDD<C>>&&);

template <typename C, typename Values>
Values
sum(context<C>&, const sum_builder<C, Values>&);


/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd

#endif // _SDD_DD_OPERATIONS_FWD_HH_
