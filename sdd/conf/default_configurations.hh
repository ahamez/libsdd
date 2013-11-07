#ifndef _SDD_CONF_DEFAULT_CONFIGURATIONS_HH_
#define _SDD_CONF_DEFAULT_CONFIGURATIONS_HH_

#include <cstdint> // uint16_t, uint32_t
#include <string>

#include "sdd/values/bitset.hh"
#include "sdd/values/flat_set.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief The default base configuration.
///
/// It doesn't include the configuration of variable, identifier and values type. These informations
/// should be given by derived configurations.
struct default_configuration
{
  /// @brief The type to store the number of elements in an alpha successor function.
  typedef std::uint16_t alpha_size_type;

  /// @brief The type to store the number of elements in an operation.
  typedef std::uint32_t operands_size_type;

  /// @brief The initial size of the hash table that stores SDD.
  std::size_t sdd_unique_table_size;

  /// @brief The size of the cache of SDD difference operations.
  std::size_t sdd_difference_cache_size;

  /// @brief The size of the cache of SDD intersection operations.
  std::size_t sdd_intersection_cache_size;

  /// @brief The size of the cache of SDD sum (union) operations.
  std::size_t sdd_sum_cache_size;

  /// @brief The initial size of the hash table that stores homomorphisms.
  std::size_t hom_unique_table_size;

  /// @brief The size of the cache of homomorphism applications.
  std::size_t hom_cache_size;

  /// @brief Tell if FPU registers shoud be preserved when using Expressions.
  static constexpr bool expression_preserve_fpu_registers = false;

  /// @brief Default constructor.
  ///
  /// Initialize all parameters to their default values.
  default_configuration()
    : sdd_unique_table_size(1000000)
    , sdd_difference_cache_size(500000)
    , sdd_intersection_cache_size(500000)
    , sdd_sum_cache_size(1000000)
    , hom_unique_table_size(1000000)
    , hom_cache_size(1000000)
  {}
};

/*------------------------------------------------------------------------------------------------*/

struct conf0
  : public default_configuration
{
  typedef std::string        Identifier;
  typedef unsigned char      Variable;
  typedef values::bitset<64> Values;
};

/*------------------------------------------------------------------------------------------------*/

struct conf1
  : public default_configuration
{
  typedef std::string                    Identifier;
  typedef unsigned int                   Variable;
  typedef values::flat_set<unsigned int> Values;

  std::size_t flat_set_unique_table_size;

  conf1()
    : default_configuration()
    , flat_set_unique_table_size(1000)
  {}
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_CONF_DEFAULT_CONFIGURATIONS_HH_
