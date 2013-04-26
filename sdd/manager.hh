#ifndef _SDD_MANAGER_HH_
#define _SDD_MANAGER_HH_

#include <cassert>
#include <memory>    // unique_ptr
#include <stdexcept> // runtime_error

#include <boost/container/flat_set.hpp>

#include "sdd/manager_fwd.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"
#include "sdd/hom/identity.hh"
#include "sdd/mem/unique_table.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Contains all global unique tables and caches.
template <typename C>
struct internal_manager
{
  // Can't copy an internal_manager.
  internal_manager(const internal_manager&) = delete;
  internal_manager& operator=(const internal_manager&) = delete;

  // Can't move an internal_manager.
  internal_manager(const internal_manager&&) = delete;
  internal_manager& operator=(const internal_manager&&) = delete;

  /// @brief The type of a unified SDD.
  typedef typename SDD<C>::unique_type sdd_unique_type;

  /// @brief The type of a smart pointer to a unified SDD.
  typedef typename SDD<C>::ptr_type sdd_ptr_type;

  /// @brief The type of a unified homomorphism.
  typedef typename homomorphism<C>::unique_type hom_unique_type;

  /// @brief The type of a smart pointer to a unified homomorphism.
  typedef typename homomorphism<C>::ptr_type hom_ptr_type;

  /// @brief Manage the handlers needed by ptr when a unified data is no longer referenced.
  struct ptr_handlers
  {
    ptr_handlers( mem::unique_table<sdd_unique_type>& sdd_ut
                , mem::unique_table<hom_unique_type>& hom_ut)
    {
      mem::set_deletion_handler<sdd_unique_type>([&](const sdd_unique_type& u){sdd_ut.erase(u);});
      mem::set_deletion_handler<hom_unique_type>([&](const hom_unique_type& u){hom_ut.erase(u);});
    }

    ~ptr_handlers()
    {
      mem::reset_deletion_handler<sdd_unique_type>();
      mem::reset_deletion_handler<hom_unique_type>();
    }
  } handlers;

  /// @brief The set of a unified SDD.
  mem::unique_table<sdd_unique_type> sdd_unique_table;

  /// @brief The SDD operations evaluation context.
  dd::context<C> sdd_context;

  /// @brief The set of unified homomorphisms.
  mem::unique_table<hom_unique_type> hom_unique_table;

  /// @brief The homomorphisms evaluation context.
  hom::context<C> hom_context;

  /// @brief The cached |0| terminal.
  const sdd_ptr_type zero;

  /// @brief The cached |1| terminal.
  const sdd_ptr_type one;

  /// @brief The cached Id homomorphism.
  const hom_ptr_type id;

  /// @brief Used to avoid frequent useless reallocations in SaturationFixpoint().
  boost::container::flat_set<homomorphism<C>> saturation_fixpoint_data;

  /// @brief Default constructor.
  internal_manager(const C& configuration)
    : handlers(sdd_unique_table, hom_unique_table)
    , sdd_unique_table(configuration.sdd_unique_table_size)
    , sdd_context( configuration.sdd_difference_cache_size
                 , configuration.sdd_intersection_cache_size
                 , configuration.sdd_sum_cache_size)
    , hom_unique_table(configuration.hom_unique_table_size)
    , hom_context(configuration.hom_cache_size, sdd_context)
    , zero(mk_terminal<zero_terminal<C>>())
    , one(mk_terminal<one_terminal<C>>())
    , id(mk_id())
    , saturation_fixpoint_data()
  {
  }

private:

  /// brief Helper to construct terminals.
  template <typename T>
  sdd_ptr_type
  mk_terminal()
  {
    char* addr = sdd_unique_table.allocate(0 /*extra bytes*/);
    sdd_unique_type* u = new (addr) sdd_unique_type(mem::construct<T>());
    return sdd_ptr_type(sdd_unique_table(u));
  }

  /// @brief Helper to construct Id.
  hom_ptr_type
  mk_id()
  {
    char* addr = hom_unique_table.allocate(0 /*extra bytes*/);
    hom_unique_type* u = new (addr) hom_unique_type(mem::construct<hom::identity<C>>());
    return hom_ptr_type(hom_unique_table(u));
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Contains the global (static) internal manager.
/// @related internal_manager
template <typename C>
inline
internal_manager<C>**
global_ptr()
noexcept
{
  static internal_manager<C>* m = nullptr;
  return &m;
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Get the global context as a reference.
/// @related internal_manager
///
/// Make internal calls to the global internal_manager easier.
template <typename C>
inline
internal_manager<C>&
global()
noexcept
{
  assert(*global_ptr<C>() != nullptr && "Uninitialized global internal_manager");
  return **global_ptr<C>();
}

/*------------------------------------------------------------------------------------------------*/

/// @brief This class represents the global context of the library.
///
/// It can only be created by the init() function. It is safe to use the library as long as an
/// instance returned by init() exists.
template <typename C>
class manager final
{
  // Can't copy a manager.
  manager(const manager&) = delete;
  manager& operator=(const manager&) = delete;

private:

  /// @brief The real global context.
  std::unique_ptr<internal_manager<C>> m_;

  /// @brief Construct a manager from an internal manager.
  manager(std::unique_ptr<internal_manager<C>>&& m)
    : m_(std::move(m))
  {
  }

public:

  /// @brief Initialize the library for a specific configuration.
  /// @tparam C The configuration type.
  /// @param configuration An instance of the configuration with some parameters set.
  /// @throw std::runtime_error if the library was already configured.
  ///
  /// It's must be the first function called before any other call to the library.
  static
  manager<C>
  init(const C& configuration = C())
  {
    if (*global_ptr<C>() == nullptr)
    {
      std::unique_ptr<internal_manager<C>> g(new internal_manager<C>(configuration));
      *global_ptr<C>() = g.get();
      return manager<C>(std::move(g));
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
    }
  }

  /// @brief Default move constructor.
  manager(manager&&) = default;

  /// @brief Default move operator.
  manager& operator=(manager&&) = default;
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_MANAGER_HH_
