/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <memory> // make_shared, shared_ptr

#include "sdd/dd/context.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation.hh"
#include "sdd/hom/rewrite.hh"
#include "sdd/mem/cache.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief The evaluation context of homomorphisms.
///
/// Its purpose is to be able to create local caches at different points of the evaluation.
/// The cache is wrapped in a std::shared_ptr to enable cheap copy.
template <typename C>
class context
{
public:

  /// @brief Homomorphism evaluation cache type.
  using cache_type = mem::cache< context, cached_homomorphism<C>, should_cache<C>>;

  /// @brief SDD operation context type.
  using sdd_context_type = sdd::dd::context<C>;

private:

  /// @brief Cache homomorphisms evaluation.
  std::shared_ptr<cache_type> cache_;

  /// @brief Context of SDD operations.
  ///
  /// It already implements cheap-copy, we don't need to use a shared_ptr.
  sdd_context_type sdd_context_;

public:

  /// @brief Construct a new context.
  context(std::size_t size, sdd_context_type& sdd_cxt)
   	: cache_(std::make_shared<cache_type>(*this, size))
    , sdd_context_(sdd_cxt)
  {}

  /// @brief Copy constructor.
  context(const context&) = default;

  /// @brief Return the cache of homomorphism evaluation.
  cache_type&
  cache()
  noexcept
  {
    return *cache_;
  }

  /// @brief Return the context of SDD operations.
  sdd_context_type&
  sdd_context()
  noexcept
  {
    return sdd_context_;
  }

  /// @brief Remove all cache entries of this context.
  void
  clear()
  noexcept
  {
    cache_->clear();
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom
