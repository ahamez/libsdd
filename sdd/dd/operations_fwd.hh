#pragma once

#include "sdd/dd/context_fwd.hh"
#include "sdd/dd/definition_fwd.hh"
#include "sdd/dd/nary.hh"

// Forward declarations to let operations call each other recursively.

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

// Forward declaration of the SDD difference operation.
template <typename C>
SDD<C>
difference(context<C>&, const SDD<C>&, const SDD<C>&);

// Forward declaration of the Values difference operation.
template <typename C, typename Values>
Values
difference(context<C>&, const Values&, const Values&);

/*------------------------------------------------------------------------------------------------*/

// Forward declaration of the implementation of the intersection builder.
template <typename C, typename Valuation>
struct intersection_builder_impl;

// Forward declaration of the implementation of the SDD intersecton operation.
template <typename C>
struct intersection_op_impl;

/// @internal
/// @brief Use to build intersection operands.
template <typename C, typename Valuation>
using intersection_builder = nary_builder<C, Valuation, intersection_builder_impl<C, Valuation>>;

/// @internal
/// @brief Used as a template parameter by nary_op to implement the SDD intersection operation.
template <typename C>
using intersection_op = nary_op<C, intersection_op_impl<C>>;

// Forward declaration of the SDD intersection operation.
template <typename C>
SDD<C>
intersection(context<C>&, intersection_builder<C, SDD<C>>&&);

// Forward declaration of the Values intersection operation.
template <typename C, typename Values>
Values
intersection(context<C>&, const intersection_builder<C, Values>&);

/*------------------------------------------------------------------------------------------------*/

// Forward declaration of the sum of the intersection builder.
template <typename C, typename Valuation>
struct sum_builder_impl;

// Forward declaration of the implementation of the SDD sum operation.
template <typename C>
struct sum_op_impl;

/// @internal
/// @brief Use to build sum operands.
template <typename C, typename Valuation>
using sum_builder = nary_builder<C, Valuation, sum_builder_impl<C, Valuation>>;

/// @internal
/// @brief Used as a template parameter by nary_op to implement the SDD sum operation.
template <typename C>
using sum_op = nary_op<C, sum_op_impl<C>>;

// Forward declaration of the SDD sum operation.
template <typename C>
SDD<C>
sum(context<C>&, sum_builder<C, SDD<C>>&&);

// Forward declaration of the Values sum operation.
template <typename C, typename Values>
Values
sum(context<C>&, const sum_builder<C, Values>&);

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd
