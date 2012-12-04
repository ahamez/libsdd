#ifndef _SDD_TESTS_HOM_COMMON_HH_
#define _SDD_TESTS_HOM_COMMON_HH_

#include <iostream>

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"
#include "sdd/order/order.hh"

/*------------------------------------------------------------------------------------------------*/

typedef sdd::conf0 conf;
typedef sdd::SDD<conf> SDD;
typedef sdd::homomorphism<conf> hom;
typedef sdd::values::bitset<64> bitset;

using sdd::Composition;
using sdd::Cons;
using sdd::Expression;
using sdd::Fixpoint;
using sdd::Inductive;
using sdd::Local;
using sdd::hom::SaturationFixpoint;
using sdd::hom::SaturationSum;
using sdd::Sum;
using sdd::ValuesFunction;

typedef sdd::order_builder<conf> order_builder;
typedef sdd::order<conf> order;

/*------------------------------------------------------------------------------------------------*/

#endif // _SDD_TESTS_HOM_COMMON_HH_
