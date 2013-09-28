#ifndef _SDD_DD_TOP_HH_
#define _SDD_DD_TOP_HH_

#include <exception>
#include <memory> // make_shared, shared_ptr
#include <sstream>
#include <string>
#include <vector>

#include "sdd/dd/definition_fwd.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief A base class to wrap operations of different type.
struct operation_wrapper_base
{
  virtual
  ~operation_wrapper_base()
  {}

  virtual std::string print() const noexcept = 0;
};

/// @internal
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
  {}

  /// @brief Return a textual description of the contained operation.
  std::string
  print()
  const noexcept
  {
    std::stringstream ss;
    ss << operation_;
    return ss.str();
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @exception top
/// @brief The top terminal.
///
/// The top terminal is represented with an exception thrown when encoutering incompatible SDD.
template <typename C>
class top final
  : public std::exception
{
private:

  /// @brief The left incompatible operand.
  const SDD<C> lhs_;

  /// @brief The right incompatible operand.
  const SDD<C> rhs_;

  /// @brief The sequence, in reverse order, of operations that led to the error.
  std::vector<std::shared_ptr<operation_wrapper_base>> steps_;

  /// @brief Textual description of the error.
  mutable std::string description_;

public:

  /// @internal
  top(const SDD<C>& lhs, const SDD<C>& rhs)
    : lhs_(lhs), rhs_(rhs), steps_(), description_()
  {}

  ~top()
  noexcept
  {}

  /// @brief Return the textual description of the error.
  ///
  /// All operations that led to the error are printed.
  const char*
  what()
  const noexcept
  {
    return description().c_str();
  }

  /// @brief Get the left incompatible operand.
  ///
  /// Note that 'left' and 'right' are arbitrary.
  SDD<C>
  lhs()
  const noexcept
  {
    return lhs_;
  }

  /// @brief Get the right incompatible operand.
  ///
  /// Note that 'left' and 'right' are arbitrary.
  SDD<C>
  rhs()
  const noexcept
  {
    return rhs_;
  }

  /// @internal
  /// @brief Add an operation to the sequence of operations that lead to incompatible SDD.
  ///
  /// Called by mem::cache.
  template <typename Operation>
  void
  add_step(Operation&& op)
  {
    steps_.emplace_back(std::make_shared<operation_wrapper<Operation>>(std::move(op)));
  }

  /// @internal
  /// @brief Return a textual description.
  std::string&
  description()
  const noexcept
  {
    if (description_.empty())
    {
      std::stringstream ss;
      ss << "Incompatible SDD: " << lhs_ << " and " << rhs_ << "."
         << std::endl
         << "The following operations led to this error (first to last): "
         << std::endl;
      std::size_t i = 1;
      for (auto rcit = steps_.crbegin(); rcit != steps_.crend(); ++rcit, ++i)
      {
        ss << i << " : " << (*rcit)->print() << std::endl;
      }
      description_ = ss.str();
    }
    return description_;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_DD_TOP_HH_
