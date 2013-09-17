#ifndef _SDD_TESTS_MACROS_HH_
#define _SDD_TESTS_MACROS_HH_

/*------------------------------------------------------------------------------------------------*/

// Ugly hack to avoid uglier types 

#define conf typename TestFixture::configuration_type
#define values_type typename TestFixture::configuration_type::Values
#define identifier_type typename TestFixture::configuration_type::Identifier
#define SDD typename sdd::SDD<conf>
#define zero this->zero
#define one this->one
#define cxt this->cxt
#define homomorphism sdd::homomorphism<conf>
#define order_builder sdd::order_builder<conf>
#define order sdd::order<conf>
#define id this->id

/*------------------------------------------------------------------------------------------------*/

#endif // _SDD_TESTS_MACROS_HH_
