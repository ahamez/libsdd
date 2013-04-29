#ifndef _SDD_MANAGER_HH_
#define _SDD_MANAGER_HH_

#include <memory>    // unique_ptr
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
class manager final
{
  // Can't copy a manager.
  manager(const manager&) = delete;
  manager& operator=(const manager&) = delete;

private:

  /// @brief The type of a set of values.
  typedef typename C::Values values_type;

  /// @brief The manager of Values.
  std::unique_ptr<values_manager<values_type>> values_;

  /// @brief The manager of SDD and homomorphisms.
  std::unique_ptr<internal_manager<C>> m_;

  /// @brief Construct a manager from an internal manager.
  manager(std::unique_ptr<values_manager<values_type>> v, std::unique_ptr<internal_manager<C>>&& m)
    : values_(std::move(v))
    , m_(std::move(m))
  {}

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
    if (*global_ptr<C>() == nullptr and *global_ptr<values_type>() == nullptr)
    {
      std::unique_ptr<values_manager<values_type>>
        v(new values_manager<values_type>(configuration));
      *global_values_ptr<values_type>() = v.get();

      std::unique_ptr<internal_manager<C>> g(new internal_manager<C>(configuration));
      *global_ptr<C>() = g.get();

      return manager<C>(std::move(v), std::move(g));
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
    if (values_)
    {
      *global_values_ptr<values_type>() = nullptr;
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
