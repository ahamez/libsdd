#ifndef _SDD_TESTS_HOM_COMMON_HH_
#define _SDD_TESTS_HOM_COMMON_HH_

#include <iostream>

#include "sdd/conf/default_configurations.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"

/*-------------------------------------------------------------------------------------------*/

typedef sdd::conf::conf0 conf;
typedef sdd::SDD<conf> SDD;
typedef sdd::hom::homomorphism<conf> hom;
typedef sdd::values::bitset<64> bitset;

using sdd::hom::Composition;
using sdd::hom::Cons;
using sdd::hom::Fixpoint;
using sdd::hom::Inductive;
using sdd::hom::Local;
using sdd::hom::SaturationFixpoint;
using sdd::hom::SaturationSum;
using sdd::hom::Sum;

/*-------------------------------------------------------------------------------------------*/

#endif // _SDD_TESTS_HOM_COMMON_HH_
