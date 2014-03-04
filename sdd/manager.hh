#ifndef _SDD_MANAGER_HH_
#define _SDD_MANAGER_HH_

#include <stdexcept> // runtime_error

#include "sdd/internal_manager.hh"
#include "sdd/values_manager.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @brief This class represents the global context of the library.
///
/// It can only be created by the init() function. It is safe to use the library as long as the
/// instance returned by manager::init() exists.
template <typename C>
class manager
{
  // Can't copy a manager.
  manager(const manager&) = delete;
  manager& operator=(const manager&) = delete;

private:

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief Keep the configuration.
  const C conf_;

  /// @brief The manager of Values.
  values_manager<values_type>* values_;

  /// @brief The manager of SDD and homomorphisms.
  internal_manager<C>* m_;

  /// @brief Construct a manager from an internal manager.
  manager(const C& conf, values_manager<values_type>* v, internal_manager<C>* m)
    : conf_(conf), values_(v), m_(m)
  {}

public:

  /// @brief Initialize the library for a specific configuration.
  /// @tparam C The configuration type.
  /// @param configuration An instance of the configuration with some parameters set.
  /// @throw std::runtime_error if the library was already configured.
  ///
  /// It must be the first function called before any other call to the library.
  static
  manager<C>
  init(const C& configuration = C())
  {
    if (*global_ptr<C>() == nullptr and *global_ptr<values_type>() == nullptr)
    {
      values_manager<values_type>* v = new values_manager<values_type>(configuration);
      if (not v)
      {
        throw std::runtime_error("Can't allocate values manager.");
      }

      *global_values_ptr<values_type>() = v;

      internal_manager<C>* g = new internal_manager<C>(configuration);
      if (not g)
      {
        delete v;
        throw std::runtime_error("Can't allocate internal manager.");
      }
      *global_ptr<C>() = g;

      return manager<C>(configuration, v, g);
    }
    else
    {
      throw std::runtime_error("Library already initialized.");
    }
  }

  /// @brief Destructor.
  ~manager()
  {
    if (m_)
    {
      *global_ptr<C>() = nullptr;
      if (conf_.final_cleanup)
      {
        delete m_;
      }
    }
    if (values_)
    {
      *global_values_ptr<values_type>() = nullptr;
      if (conf_.final_cleanup)
      {
        delete values_;
      }
    }
  }

  /// @brief Default move constructor.
  manager(manager&&) = default;

  /// @brief Default move operator.
  manager& operator=(manager&&) = default;

  /// @brief Reset homomorphisms evaluation cache.
  void
  reset_hom_cache()
  {
    m_->hom_context.clear();
  }

  /// @internal
  /// @brief Get the statistics for SDDs.
  const mem::unique_table_statistics&
  sdd_stats()
  const noexcept
  {
    return m_->sdd_unique_table.stats();
  }

  /// @internal
  /// @brief Get the statistics for SDD difference operations.
  const mem::cache_statistics&
  sdd_difference_cache_stats()
  const noexcept
  {
    return m_->sdd_context.difference_cache().statistics();
  }

  /// @internal
  /// @brief Get the statistics for SDD intersection operations.
  const mem::cache_statistics&
  sdd_intersection_cache_stats()
  const noexcept
  {
    return m_->sdd_context.intersection_cache().statistics();
  }

  /// @internal
  /// @brief Get the statistics for SDD sum operations.
  const mem::cache_statistics&
  sdd_sum_cache_stats()
  const noexcept
  {
    return m_->sdd_context.sum_cache().statistics();
  }

  /// @internal
  /// @brief Get the statistics for homomorphisms.
  const mem::unique_table_statistics&
  hom_stats()
  const noexcept
  {
    return m_->hom_unique_table.stats();
  }

  /// @internal
  /// @brief Get the statistics for SDD sum operations.
  const mem::cache_statistics&
  hom_cache_stats()
  const noexcept
  {
    return m_->hom_context.cache().statistics();
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_MANAGER_HH_
