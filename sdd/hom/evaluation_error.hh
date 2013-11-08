#ifndef _SDD_HOM_EVALUATION_ERROR_HH_
#define _SDD_HOM_EVALUATION_ERROR_HH_

#include <deque>
#include <exception>
#include <memory> // make_shared, shared_ptr
#include <sstream>
#include <string>

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

namespace hom {

/// @internal
/// @brief A base class to wrap operations of different type.
struct operation_wrapper_base
{
  virtual
  ~operation_wrapper_base()
  {}

  virtual std::string print() const = 0;
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
  const final
  {
    std::ostringstream ss;
    ss << operation_;
    return ss.str();
  }
};

/// @internal
/// @brief Specialization to contain a Top exception.
template <typename C>
struct operation_wrapper<top<C>>
  : public operation_wrapper_base
{
  const top<C> top_;

  /// @brief Constructor.
  operation_wrapper(const top<C>& t)
    : top_(t)
  {}

  /// @brief Return a textual description of the contained Top.
  std::string
  print()
  const
  {
    return top_.description();
  }
};

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @exception evaluation_error
/// @brief Raised when an error is encountered by an evaluated homomorphism.
template <typename C>
class evaluation_error final
  : public std::exception
{
private:

  /// @brief The SDD operand.
  const SDD<C> sdd_;

  /// @brief The sequence, in reverse order, of operations that led to the error.
  std::deque<std::shared_ptr<hom::operation_wrapper_base>> steps_;

  /// @brief Textual description of the error.
  mutable std::string description_;

public:

  /// @internal
  evaluation_error(const SDD<C>& s)
    : sdd_(s)
    , steps_()
    , description_()
  {}

  ~evaluation_error()
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

  /// @internal
  /// @brief Add an operation to the sequence of operations that lead to an evaluation error.
  template <typename Operation>
  void
  add_step(Operation&& op)
  {
    steps_.emplace_back(std::make_shared<hom::operation_wrapper<Operation>>(std::move(op)));
  }

  /// @internal
  void
  add_top(const top<C>& t)
  {
    steps_.emplace_back(std::make_shared<hom::operation_wrapper<top<C>>>(t));
  }

  /// @internal
  /// @brief Return a textual description.
  std::string&
  description()
  const noexcept
  {
    if (description_.empty())
    {
      std::ostringstream ss;
      ss << "Application failed on " << sdd_ << std::endl;
      ss << "The following operations led to this error:" << std::endl;
      std::size_t i = 1;
      for (auto rcit = steps_.crbegin(); rcit != steps_.crend(); ++rcit, ++i)
      {
        ss << i << " : " << (*rcit)->print() << std::endl;
      }
      description_ = ss.str();
    }
    return description_;
  }

  /// @brief Get the operand that caused this error.
  SDD<C>
  operand()
  const noexcept
  {
    return sdd_;
  }

};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_HOM_EVALUATION_ERROR_HH_
