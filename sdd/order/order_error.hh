#ifndef _SDD_ORDER_ERROR_HH_
#define _SDD_ORDER_ERROR_HH_

#include <deque>
#include <exception>
#include <memory> // make_shared, shared_ptr
#include <sstream>
#include <string>

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @exception order_error
/// @brief Raised when an identifier is not found in an order.
class order_error
  : public std::exception
{
public:

  ~order_error()
  {}

  /// @brief Return the textual description of the error.
  const char*
  what()
  const noexcept
  {
    return description().c_str();
  }

  /// @internal
  /// @brief Return a textual description.
  virtual
  std::string&
  description()
  const = 0;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @exception order_error_impl
template <typename C>
class order_error_impl final
  : public order_error
{
public:

  /// @brief The type of a user identifier.
  using identifier_type = typename C::Identifier;

private:

  /// @brief The identifier not found.
  const identifier_type identifier_;

  /// @brief Textual description of the error.
  mutable std::string description_;

public:

  /// @internal
  order_error_impl(const identifier_type& id)
    : identifier_(id)
  {}

  ~order_error_impl()
  {}

  /// @internal
  /// @brief Return a textual description.
  std::string&
  description()
  const noexcept override
  {
    if (description_.empty())
    {
      std::ostringstream ss;
      ss << "Identifier " << identifier_ << " not found in order.";
      description_ = ss.str();
    }
    return description_;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_ORDER_ERROR_HH_
