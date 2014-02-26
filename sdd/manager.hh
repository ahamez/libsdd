#ifndef _SDD_MANAGER_HH_
#define _SDD_MANAGER_HH_

#include <stdexcept> // runtime_error

#include "sdd/internal_manager.hh"
#include "sdd/values_manager.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

// Foward declaration
template <typename C>
class manager;

/// @brief Show some statistics of the library
/// @related manager
template <typename C>
std::ostream&
operator<<(std::ostream& os, const manager<C>& m)
{
  const auto sdd_stats = m.m_->sdd_unique_table.stats();
  const auto hom_stats = m.m_->hom_unique_table.stats();

  os << "SDD" << std::endl;
  os << "size        : " << sdd_stats.size << std::endl;
  os << "load_factor : " << sdd_stats.load_factor << std::endl;
  os << "access      : " << sdd_stats.access << std::endl;
  os << "hit         : " << sdd_stats.hit << std::endl;
  os << "miss        : " << sdd_stats.miss << std::endl;
  os << std::endl;

  os << "Hom" << std::endl;
  os << "size        : " << hom_stats.size << std::endl;
  os << "load_factor : " << hom_stats.load_factor << std::endl;
  os << "access      : " << hom_stats.access << std::endl;
  os << "hit         : " << hom_stats.hit << std::endl;
  os << "miss        : " << hom_stats.miss << std::endl;

  return os;
}

/*------------------------------------------------------------------------------------------------*/

/// @brief This class represents the global context of the library.
///
/// It can only be created by the init() function. It is safe to use the library as long as the
/// instance returned by manager::init() exists.
template <typename C>
class manager final
{
  // Can't copy a manager.
  manager(const manager&) = delete;
  manager& operator=(const manager&) = delete;

private:

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief Keep the configuration.
  const C& conf_;

  /// @brief The manager of Values.
  values_manager<values_type>* values_;

  /// @brief The manager of SDD and homomorphisms.
  internal_manager<C>* m_;

  /// @brief Construct a manager from an internal manager.
  manager(const C& conf, values_manager<values_type>* v, internal_manager<C>* m)
    : conf_(conf), values_(v), m_(m)
  {}

  friend
  std::ostream&
  operator<< <C>(std::ostream&, const manager&);

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

  /// @brief Reset homomorphisms evaluation cache.
  void
  reset_hom_cache()
  {
    m_->hom_context.clear();
  }

  /// @brief Default move constructor.
  manager(manager&&) = default;

  /// @brief Default move operator.
  manager& operator=(manager&&) = default;
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_MANAGER_HH_
