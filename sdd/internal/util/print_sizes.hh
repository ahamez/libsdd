#ifndef _SDD_INTERNAL_UTIL_PRINT_SIZES_HH_
#define _SDD_INTERNAL_UTIL_PRINT_SIZES_HH_

#include <iosfwd>

#include "sdd/dd/definition.hh"
#include "sdd/hom/definition.hh"

/// @cond INTERNAL_DOC

namespace sdd { namespace internal { namespace util {

/*-------------------------------------------------------------------------------------------*/

template <typename C>
void
print_sizes(std::ostream& os)
{
#if defined LIBSDD_PACKED
  os << "** PACKED **" << std::endl;
#else
  os << "** NOT PACKED **" << std::endl;
#endif

  os << std::endl;
  os << "Variable " << sizeof(typename C::Variable) << std::endl;
  os << "Values   " << sizeof(typename C::Values) << std::endl;

  os << std::endl;
  os << "SDD::data          " << sizeof(typename SDD<C>::SDD_data) << std::endl;
  os << "SDD::data::storage " << sizeof(typename SDD<C>::SDD_data::storage_type) << std::endl;
  os << "SDD::unique        " << sizeof(typename SDD<C>::SDD_unique) << std::endl;
  os << "zero_terminal      " << sizeof(zero_terminal<C>) << std::endl;
  os << "one_terminal       " << sizeof(one_terminal<C>) << std::endl;
  os << "flat_node          " << sizeof(flat_node<C>) << std::endl;
  os << "hierarchical_node  " << sizeof(hierarchical_node<C>) << std::endl;

  typedef hom::homomorphism<C> hom_type;

  os << std::endl;
  os << "Hom::data             " << sizeof(typename hom_type::data_type) << std::endl;
  os << "Hom::data::storage    " << sizeof(typename hom_type::data_type::storage_type) << std::endl;
  os << "Hom::unique           " << sizeof(typename hom_type::unique_type) << std::endl;
  os << "Composition           " << sizeof(hom::composition<C>) << std::endl;
  os << "Flat Cons             " << sizeof(hom::cons<C, typename C::Values>) << std::endl;
  os << "Hierarchical Cons     " << sizeof(hom::cons<C, SDD<C>>) << std::endl;
  os << "Constant              " << sizeof(hom::constant<C>) << std::endl;
  os << "Fixpoint              " << sizeof(hom::fixpoint<C>) << std::endl;
  os << "Identity              " << sizeof(hom::identity<C>) << std::endl;
  os << "Inductive             " << sizeof(hom::inductive<C>) << std::endl;
  os << "Local                 " << sizeof(hom::local<C>) << std::endl;
  os << "Saturation Fixpoint   " << sizeof(hom::saturation_fixpoint<C>) << std::endl;
  os << "Saturation Sum        " << sizeof(hom::saturation_sum<C>) << std::endl;
  os << "Sum                   " << sizeof(hom::sum<C>) << std::endl;
}

/*-------------------------------------------------------------------------------------------*/

}}} // namespace sdd::internal::util

/// @endcond

#endif // _SDD_INTERNAL_UTIL_PRINT_SIZES_HH_