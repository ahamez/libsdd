#ifndef _SDD_HOM_CONTEXT_HH_
#define _SDD_HOM_CONTEXT_HH_

/// @cond INTERNAL_DOC

#include <memory> // make_shared, shared_ptr

#include "sdd/dd/context.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation.hh"
#include "sdd/hom/evaluation_error.hh"
#include "sdd/internal/mem/cache.hh"

namespace sdd { namespace hom {

/*-------------------------------------------------------------------------------------------*/

/// @brief The evaluation context of homomorphisms.
///
/// Its purpose is to be able to create local caches at different points of the evaluation.
/// The cache is wrapped in a std::shared_ptr to enable cheap copy.
template <typename C>
class context
{
public:

  /// @brief Cache type.
  typedef internal::mem::cache<cached_homomorphism<C>, evaluation_error<C>, should_cache<C>>
          cache_type;

  /// @brief SDD operation context type.
  typedef sdd::context<C> sdd_context_type;

private:

  /// @brief Cache of union homomorphisms.
  std::shared_ptr<cache_type> cache_;

  /// @brief Context of SDD operations.
  ///
  /// It already implements cheap-copy, we don't need to use a shared_ptr.
  sdd_context_type sdd_context_;

  /// @brief The context's cache size.
  const std::size_t size_;

public:

  /// @brief Construct a new context.
  context(std::size_t size, sdd_context_type& sdd_cxt)
   	: cache_(std::make_shared<cache_type>("homomorphism_cache", size))
    , sdd_context_(sdd_cxt)
    , size_(size)
  {
  }

  /// @brief Default constructor.
  context()
	  : context(10000, sdd::initial_context<C>())
  {
  }

  /// @brief Construct a context with an existing SDD context.
  context(sdd_context_type& sdd_cxt)
    : context(10000, sdd_cxt)
  {
  }

  /// @brief Copy constructor.
  context(const context& other) = default;

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

  /// @brief Return this context's cache size.
  std::size_t
  size()
  const noexcept
  {
    return size_;
  }
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Return the context that serves as an entry point for the evaluation of
/// homomorphisms.
/// @related context
template <typename C>
context<C>&
initial_context()
noexcept
{
  static context<C> initial_context;
  return initial_context;
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

/// @endcond

#endif // _SDD_HOM_CONTEXT_HH_
