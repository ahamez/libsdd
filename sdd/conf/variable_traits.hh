#ifndef _SDD_CONF_VARIABLE_TRAITS_HH_
#define _SDD_CONF_VARIABLE_TRAITS_HH_

namespace sdd { namespace conf {

/*-------------------------------------------------------------------------------------------*/

/// @brief Default trait for variables.
template <typename T>
struct variable_traits
{
  static constexpr
  T
  first()
  {
    return 0;
  }

  static
  T
  next(T x)
  {
    return x + 1;
  }
};

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::conf

#endif // _SDD_CONF_VARIABLE_TRAITS_HH_
