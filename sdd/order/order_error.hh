/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

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

  /// @brief Return the textual description of the error.
  const char*
  what()
  const noexcept override
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
/// @exception identifier_not_found_error
template <typename C>
class identifier_not_found_error final
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
  identifier_not_found_error(identifier_type id)
    : identifier_{std::move(id)}
  {}

  /// @internal
  /// @brief Return a textual description.
  std::string&
  description()
  const override
  {
    if (description_.empty())
    {
      std::ostringstream ss;
      ss << "Identifier \"" << identifier_ << "\" not found in order.";
      description_ = ss.str();
    }
    return description_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @exception duplicate_identifier_error
template <typename C>
class duplicate_identifier_error final
  : public order_error
{
public:

  /// @brief The type of a user identifier.
  using identifier_type = typename C::Identifier;

private:

  /// @brief The duplicate identifier.
  const identifier_type identifier_;

  /// @brief Textual description of the error.
  mutable std::string description_;

public:

  /// @internal
  duplicate_identifier_error(identifier_type id)
    : identifier_{std::move(id)}
  {}

  /// @internal
  /// @brief Return a textual description.
  std::string&
  description()
  const override
  {
    if (description_.empty())
    {
      std::ostringstream ss;
      ss << "Duplicate identifier \"" << identifier_ << "\".";
      description_ = ss.str();
    }
    return description_;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd
