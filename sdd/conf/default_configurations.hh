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
/// It doesn't include the configuration of identifier and values type. These informations
/// should be given by derived configurations.
struct default_configuration
{
  /// @brief The type of an SDD variable.
  using variable_type = unsigned int;

  /// @brief The type to store the number of elements in an alpha successor function.
  using alpha_size_type = std::uint16_t;

  /// @brief The type to store the number of elements in an operation.
  using operands_size_type = std::uint32_t;

  /// @brief The initial size of the hash table that stores SDD.
  std::size_t sdd_unique_table_size;

  /// @brief The size of the cache of SDD difference operations.
  std::size_t sdd_difference_cache_size;

  /// @brief The size of the cache of SDD intersection operations.
  std::size_t sdd_intersection_cache_size;

  /// @brief The size of the cache of SDD sum (union) operations.
  std::size_t sdd_sum_cache_size;

  /// @brief The size, in bytes, of the buffer for temporary containers allocation.
  std::size_t sdd_arena_size;

  /// @brief The initial size of the hash table that stores homomorphisms.
  std::size_t hom_unique_table_size;

  /// @brief The size of the cache of homomorphism applications.
  std::size_t hom_cache_size;

  /// @brief Tell if FPU registers shoud be preserved when using Expressions.
  static constexpr bool expression_preserve_fpu_registers = false;

  /// @brief Tell if the library should clean all memory when the manager is destroyed.
  ///
  /// Useful if you have only one instance of the library and that you don't want to wait for its
  /// cleanup at your program's exit.
  bool final_cleanup;

  /// @brief Default constructor.
  ///
  /// Initialize all parameters to their default values.
  default_configuration()
    : sdd_unique_table_size(10000000)
    , sdd_difference_cache_size(500000)
    , sdd_intersection_cache_size(500000)
    , sdd_sum_cache_size(1000000)
    , sdd_arena_size(1024*1024*16)
    , hom_unique_table_size(1000000)
    , hom_cache_size(1000000)
    , final_cleanup(true)
  {}
};

/*------------------------------------------------------------------------------------------------*/

struct flat_set_default_configuration
  : public default_configuration
{
  /// @brief The size of the hash table that stores flat_set<>.
  std::size_t flat_set_unique_table_size;

  flat_set_default_configuration()
    : default_configuration()
    , flat_set_unique_table_size(5000)
  {}
};

/*------------------------------------------------------------------------------------------------*/

struct conf0
  : public default_configuration
{
  using Identifier = std::string;
  using Values     = values::bitset<64>;
};

/*------------------------------------------------------------------------------------------------*/

struct conf1
  : public flat_set_default_configuration
{
  using Identifier = std::string;
  using Values     = values::flat_set<unsigned int>;
};

/*------------------------------------------------------------------------------------------------*/

struct conf2
  : public flat_set_default_configuration
{
  using Identifier = unsigned int;
  using Values     = values::flat_set<unsigned int>;
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_CONF_DEFAULT_CONFIGURATIONS_HH_
