/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <memory> // make_shared, shared_ptr

#include "sdd/dd/context_fwd.hh"
#include "sdd/dd/definition_fwd.hh"
#include "sdd/dd/difference.hh"
#include "sdd/dd/intersection.hh"
#include "sdd/dd/sum.hh"
#include "sdd/mem/cache.hh"
#include "sdd/mem/linear_alloc.hh"

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The evaluation context of operations on SDD (union, intersection, difference).
///
/// Its purpose is to be able to create local caches at different points of the evaluation.
/// There is a cache per operation type, each of them being wrapped in a std::shared_ptr
/// to enable cheap copy if we want to transmit caches from context to context.
template <typename C>
class context
{
public:

  /// @brief Cache parameterized by the difference operation.
  using difference_cache_type = mem::cache<context, difference_op<C>>;

  /// @brief Cache parameterized by the intersection operation.
  using intersection_cache_type = mem::cache<context, intersection_op<C>>;

  /// @brief Cache parameterized by the sum operation.
  using sum_cache_type = mem::cache<context, sum_op<C>>;

private:

  /// @brief Cache of SDD difference.
  std::shared_ptr<difference_cache_type> difference_cache_;

  /// @brief Cache of SDD intersection.
  std::shared_ptr<intersection_cache_type> intersection_cache_;

  /// @brief Cache of SDD union.
  std::shared_ptr<sum_cache_type> sum_cache_;

  /// @brief Buffer for temporary containers allocation.
  std::shared_ptr<mem::arena> arena_;

public:

  /// @brief Create a new empty context.
  context( std::size_t difference_size, std::size_t intersection_size, std::size_t sum_size
         , std::size_t arena_size)
    : difference_cache_{std::make_shared<difference_cache_type>(*this, difference_size)}
    , intersection_cache_{std::make_shared<intersection_cache_type>( *this, intersection_size)}
    , sum_cache_{std::make_shared<sum_cache_type>(*this, sum_size)}
    , arena_{std::make_shared<mem::arena>(arena_size)}
  {}

  /// @brief Copy constructor.
  context(const context&) = default;

  /// @brief Get the difference cache.
  difference_cache_type&
  difference_cache()
  noexcept
  {
    return *difference_cache_;
  }

  /// @brief Get the intersection cache.
  intersection_cache_type&
  intersection_cache()
  noexcept
  {
    return *intersection_cache_;
  }

  /// @brief Get the sum cache.
  sum_cache_type&
  sum_cache()
  noexcept
  {
    return *sum_cache_;
  }

  /// @brief Get the memory buffer.
  mem::arena&
  arena()
  noexcept
  {
    return *arena_;
  }

  /// @brief Remove all entries from all this context's caches.
  void
  clear()
  noexcept
  {
    difference_cache_->clear();
    intersection_cache_->clear();
    sum_cache_->clear();
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd
