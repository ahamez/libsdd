#ifndef _SDD_HOM_EVALUATION_ERROR_HH_
#define _SDD_HOM_EVALUATION_ERROR_HH_

#include <exception>
#include <memory> // make_shared, shared_ptr
#include <sstream>
#include <string>
#include <vector>

#include "sdd/hom/definition_fwd.hh"

namespace sdd { namespace hom {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief A base class to wrap operations of different type.
struct operation_wrapper_base
{
  virtual
  ~operation_wrapper_base()
  {
  }

  virtual std::string print() const noexcept = 0;
};

/// @brief A new type for each different operation, but which inherits from
/// operation_wrapper_base.
///
/// It it thus possible to have a list containing different operations by having a pointer
/// to the base class operation_wrapper_base.
template <typename Operation>
struct operation_wrapper
  : public operation_wrapper_base
{
  const Operation operation_;

  /// @brief Constructor.
  ///
  /// Operations are non-copyable, but movable.
  operation_wrapper(Operation&& op)
    : operation_(std::move(op))
  {
  }

  /// @brief Return a textual description of the contained operation.
  std::string
  print()
  const noexcept final
  {
    std::stringstream ss;
    return ss.str();
  }
};

/// @endcond

/*-------------------------------------------------------------------------------------------*/

/// @exception evaluation_error
/// @brief The top terminal.
///
/// The top terminal is represented as an exception thrown when encoutering incompatible SDD.
template <typename C>
class evaluation_error
  : public std::exception
{
private:

  /// @brief The sequence, in reverse order, of operations that led to the error.
  std::vector<std::shared_ptr<operation_wrapper_base>> steps_;

  /// @brief Flag to determine if the description has been built.
  mutable bool description_built_;

  /// @Textual description of the error.
  mutable std::string description_;

public:

/// @cond INTERNAL_DOC

  evaluation_error()
    : steps_()
    , description_built_(false)
    , description_()
  {
  }

/// @endcond

  /// @brief Return the textual description of the error.
  ///
  /// All operations that led to the error are printed.
  const char*
  what()
  const noexcept
  {
    if (not description_built_)
    {
      std::stringstream ss;
      ss << "TODO";
      description_ = ss.str();
      description_built_ = true;
    }
    return description_.c_str();
  }

/// @cond INTERNAL_DOC

  /// @brief Add an operation to the sequence of operations that lead to incompatible SDD.
  ///
  /// Called by internal::mem::cache.
  template <typename Operation>
  void
  add_step(Operation&& op)
  {
    steps_.emplace_back(std::make_shared<operation_wrapper<Operation>>(std::move(op)));
  }

/// @endcond
};

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

#endif // _SDD_HOM_EVALUATION_ERROR_HH_
