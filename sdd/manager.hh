/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <memory>

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
    auto vm_ptr = std::make_unique<values_manager<values_type>>(configuration);
    *global_values_ptr<values_type>() = vm_ptr.get();

    auto im_ptr = std::make_unique<internal_manager<C>>(configuration);
    *global_ptr<C>() = im_ptr.get();

    return std::make_shared<manager_impl<C>>(std::move(vm_ptr), std::move(im_ptr));
  }
  else
  {
    throw std::runtime_error("SDD library already initialized.");
  }
}

/*------------------------------------------------------------------------------------------------*/

/// @brief Global context of the libsdd.
///
/// As long as a copy of this manager (returned by init()) exists, then it's safe to use the
/// library.
template <typename C>
class manager final
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

  /// @brief The manager of Values.
  ///
  /// It must be before m_ as this one can reference data in values_.
  std::unique_ptr<values_manager<values_type>> values_;

  /// @brief The manager of SDD and homomorphisms.
  std::unique_ptr<internal_manager<C>> m_;

public:

  manager_impl( std::unique_ptr<values_manager<values_type>>&& vm_ptr
              , std::unique_ptr<internal_manager<C>>&& im_ptr)
    : values_(std::move(vm_ptr)), m_(std::move(im_ptr))
  {}

  ~manager_impl()
  {
    *global_ptr<C>() = nullptr;
    *global_values_ptr<values_type>() = nullptr;
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
  {
    return values_->statistics();
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd
