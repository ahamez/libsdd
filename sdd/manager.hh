#ifndef _SDD_MANAGER_HH_
#define _SDD_MANAGER_HH_

#include <stdexcept> // runtime_error

#include "sdd/internal_manager.hh"
#include "sdd/values_manager.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

// Forward declaration.
template <typename C>
class manager_impl;

// Forward declaration.
template <typename C>
class manager;

/*------------------------------------------------------------------------------------------------*/

/// @brief Initialize the library for a specific configuration.
/// @tparam C The configuration type.
/// @param configuration An instance of the configuration.
/// @throw std::runtime_error if the library was already initialized.
///
/// It must be the first function called before any other call to the library.
template <typename C>
manager<C>
init(const C& configuration = C())
{
  using values_type = typename C::Values;
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

    return manager<C>(std::make_shared<manager_impl<C>>(configuration, v, g));
  }
  else
  {
    throw std::runtime_error("Library already initialized.");
  }
}

/*------------------------------------------------------------------------------------------------*/

template <typename C>
class manager
{
private:

  friend manager<C> init<C>(const C&);

  /// @brief The real manager.
  std::shared_ptr<manager_impl<C>> ptr_;

  /// @internal
  manager(std::shared_ptr<manager_impl<C>>&& ptr)
    : ptr_(ptr)
  {}

public:

  /// @brief Reset homomorphisms evaluation cache.
  void
  reset_hom_cache()
  {
    ptr_->reset_hom_cache();
  }

  /// @internal
  /// @brief Get the statistics for SDDs.
  const mem::unique_table_statistics&
  sdd_stats()
  const noexcept
  {
    return ptr_->sdd_stats();
  }

  /// @internal
  /// @brief Get the statistics for SDD difference operations.
  const mem::cache_statistics&
  sdd_difference_cache_stats()
  const noexcept
  {
    return ptr_->sdd_difference_cache_stats();
  }

  /// @internal
  /// @brief Get the statistics for SDD intersection operations.
  const mem::cache_statistics&
  sdd_intersection_cache_stats()
  const noexcept
  {
    return ptr_->sdd_intersection_cache_stats();
  }

  /// @internal
  /// @brief Get the statistics for SDD sum operations.
  const mem::cache_statistics&
  sdd_sum_cache_stats()
  const noexcept
  {
    return ptr_->sdd_sum_cache_stats();
  }

  /// @internal
  /// @brief Get the statistics for homomorphisms.
  const mem::unique_table_statistics&
  hom_stats()
  const noexcept
  {
    return ptr_->hom_stats();
  }

  /// @internal
  /// @brief Get the statistics for SDD sum operations.
  const mem::cache_statistics&
  hom_cache_stats()
  const noexcept
  {
    return ptr_->hom_cache_stats();
  }

  /// @internal
  auto
  values_stats()
  const noexcept
  -> decltype(ptr_->values_stats())
  {
    return ptr_->values_stats();
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
class manager_impl
{
  // Can't copy a manager_impl.
  manager_impl(const manager_impl&) = delete;
  manager_impl& operator=(const manager_impl&) = delete;

  // Can't move a manager_impl.
  manager_impl(manager_impl&&) = delete;
  manager_impl& operator=(manager_impl&&) = delete;

private:

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief Keep the configuration.
  const C conf_;

  /// @brief The manager of Values.
  values_manager<values_type>* values_;

  /// @brief The manager of SDD and homomorphisms.
  internal_manager<C>* m_;

public:

  /// @brief Construct a manager with internal managers.
  manager_impl(const C& conf, values_manager<values_type>* v, internal_manager<C>* m)
    : conf_(conf), values_(v), m_(m)
  {}

  /// @brief Destructor.
  ~manager_impl()
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

  /// @internal
  auto
  values_stats()
  const noexcept
  -> decltype(values_->statistics())
  {
    return values_->statistics();
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_MANAGER_HH_
