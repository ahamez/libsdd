#pragma once

#include <vector> // TEMPORARY, WE SHOULD WRITE AN ITERATOR ON ONLY ONE ARC

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C, typename Valuation>
class fake_arc final
{
private:

  /// @brief The type of the values.
  using valuation_type = Valuation;

  valuation_type valuation_;
  SDD<C> successor_;

public:

  fake_arc(valuation_type&& val, SDD<C>&& succ)
    : valuation_(std::move(val))
    , successor_(std::move(succ))
  {}

  /// @brief Get the valuation of this arc.
  const valuation_type&
  valuation()
  const noexcept
  {
    return valuation_;
  }

  /// @brief Get the successor of this arc.
  SDD<C>
  successor()
  const noexcept
  {
    return successor_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C, typename Valuation>
class fake_node final
{
  /// @brief The type of the variable of this node.
  using variable_type = typename C::variable_type;

  /// @brief The type of the valuation of this node.
  using valuation_type = Valuation;

  /// @brief The arc type.
  using arc_type = fake_arc<C, valuation_type>;

  /// @brief A (const) iterator on the arcs of this node.
  using const_iterator = typename std::vector<arc_type>::const_iterator;

private:

  /// @brief The variable of this fake node.
  const variable_type variable_;

  /// @brief The (only one) arc of this fake node.
  std::vector<arc_type> arc_;

public:

  fake_node() = default;

  fake_node(const fake_node&) = default;
  fake_node& operator=(const fake_node&) = default;

  fake_node(fake_node&&) = default;
  fake_node& operator=(fake_node&&) = default;


  fake_node(variable_type var, valuation_type val, SDD<C> succ)
    : variable_(var)
    , arc_()
    //, arc_(std::move(val), std::move(succ))
  {
    arc_.emplace_back(std::move(val), std::move(succ));
  }

  variable_type
  variable()
  const noexcept
  {
    return variable_;
  }

  const_iterator
  begin()
  const noexcept
  {
    return arc_.cbegin();
  }

  const_iterator
  end()
  const noexcept
  {
    return arc_.cend();
  }

  std::size_t
  size()
  const noexcept
  {
    return 1;
  }

  bool
  empty()
  const noexcept
  {
    return values::empty_values(begin()->valuation()) or begin()->successor().empty();
  }

  const fake_node&
  operator*()
  const noexcept
  {
    return *this;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
using fake_flat_node = fake_node<C, typename C::Values>;

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd
