/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <exception>
#include <memory> // make_shared, shared_ptr
#include <sstream>
#include <string>

#include "sdd/dd/definition_fwd.hh"

namespace sdd {

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

  /// @brief Textual description of the error.
  mutable std::string description_;

public:

  /// @internal
  top(SDD<C> lhs, SDD<C> rhs)
    : lhs_{std::move(lhs)}
    , rhs_{std::move(rhs)}
    , description_{}
  {}

  /// @brief Return the textual description of the error.
  const char*
  what()
  const noexcept override
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
  /// @brief Return a textual description.
  std::string&
  description()
  const noexcept
  {
    if (description_.empty())
    {
      std::stringstream ss;
      ss << "Incompatible SDD: " << lhs_ << " and " << rhs_ << "." << std::endl;
      description_ = ss.str();
    }
    return description_;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd
