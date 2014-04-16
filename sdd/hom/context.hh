#ifndef _SDD_HOM_CONTEXT_HH_
#define _SDD_HOM_CONTEXT_HH_

#include <memory> // make_shared, shared_ptr
#include <vector>

#include "sdd/dd/context.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation.hh"
#include "sdd/hom/evaluation_error.hh"
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
  using cache_type = mem::cache< context, cached_homomorphism<C>, evaluation_error<C>
                               , should_cache<C>>;

  /// @brief Associate a variable to a cache.
  using caches_type = std::vector<std::shared_ptr<cache_type>>;

  /// @brief SDD operation context type.
  using sdd_context_type = sdd::dd::context<C>;

private:

  /// @brief Cache of union homomorphisms.
  caches_type caches_;

  /// @brief Context of SDD operations.
  ///
  /// It already implements cheap-copy, we don't need to use a shared_ptr.
  sdd_context_type sdd_context_;

public:

  /// @brief Construct a new context.
  context(std::size_t /*size*/, sdd_context_type& sdd_cxt)
    : caches_(8192, nullptr)
    , sdd_context_(sdd_cxt)
  {}

  /// @brief Copy constructor.
  context(const context&) = default;

  /// @brief Return the cache of homomorphism evaluation.
  cache_type&
  cache(order_position_type pos)
  noexcept
  {
    if (__builtin_expect(pos >= caches_.size(), false))
    {
      caches_.resize(pos + 32, nullptr);
    }
    if (__builtin_expect(caches_[pos] == nullptr, false))
    {
      caches_[pos] = std::make_shared<cache_type>(*this, "homomorphism_cache", 100000);
    }
    return *caches_[pos];
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
    for (auto& cache_ptr : caches_)
    {
      cache_ptr->clear();
    }
  }
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

#endif // _SDD_HOM_CONTEXT_HH_
