/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <cassert>
#include <iosfwd>
#include <memory>   // unique_ptr
#include <typeinfo> // typeid

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/util/packed.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Used to wrap user's inductive homomorphisms.
template <typename C>
struct inductive_base
{
  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief Required virtual destructor.
  virtual
  ~inductive_base()
  {}

  /// @brief Tell if the user's inductive skip the current variable.
  virtual
  bool
  skip(const order<C>&) const noexcept = 0;

  /// @brief Tell if the user's inductive is a selector.
  virtual
  bool
  selector() const noexcept = 0;

  /// @brief Get the next homomorphism to apply from the user.
  virtual
  homomorphism<C>
  operator()(const order<C>&, const SDD<C>&) const = 0;

  /// @brief Get the next homomorphism to apply from the user.
  virtual
  homomorphism<C>
  operator()(const order<C>&, const values_type&) const = 0;

  /// @brief Get the terminal case from the user.
  virtual
  SDD<C>
  operator()(const one_terminal<C>&) const = 0;

  /// @brief Compare inductive_base.
  virtual
  bool
  operator==(const inductive_base&) const noexcept = 0;

  /// @brief Get the user's inductive hash value.
  virtual
  std::size_t
  hash() const noexcept = 0;

  /// @brief Get the user's inductive textual representation.
  virtual
  void
  print(std::ostream&) const = 0;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Used to wrap user's inductive homomorphisms.
template <typename C, typename User>
struct inductive_derived
  : public inductive_base<C>
{
  /// @brief The user's inductive homomorphism.
  const User h;

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief Constructor.
  inductive_derived(User u)
    : h{std::move(u)}
  {}

  /// @brief Tell if the user's inductive skip the current variable.
  bool
  skip(const order<C>& o)
  const noexcept override
  {
    return skip_impl(h, o, 0);
  }

  /// @brief Tell if the user's inductive is a selector.
  bool
  selector()
  const noexcept override
  {
    return selector_impl(h, 0);
  }

  /// @brief Get the next homomorphism to apply from the user.
  homomorphism<C>
  operator()(const order<C>& o, const SDD<C>& x)
  const override
  {
    return h(o, x);
  }

  /// @brief Get the next homomorphism to apply from the user.
  homomorphism<C>
  operator()(const order<C>& o, const values_type& val)
  const override
  {
    return h(o, val);
  }

  /// @brief Get the base case from the user.
  SDD<C>
  operator()(const one_terminal<C>&)
  const override
  {
    return h();
  }

  /// @brief Compare inductive_derived.
  bool
  operator==(const inductive_base<C>& other)
  const noexcept override
  {
    return typeid(*this) == typeid(other)
           and h == static_cast<const inductive_derived&>(other).h;
  }

  /// @brief Get the user's inductive hash value.
  std::size_t
  hash()
  const noexcept(noexcept(std::hash<User>()(h))) override
  {
    return std::hash<User>()(h);
  }

  /// @brief Get the user's inductive textual representation.
  void
  print(std::ostream& os)
  const override
  {
    print_impl(os, h, 0);
  }

private:

  /// @brief Called when the user's inductive is printable.
  ///
  /// Compile-time dispatch.
  template <typename H>
  static auto
  print_impl(std::ostream& os, const H& h, int)
  -> decltype(operator<<(os, h))
  {
    return os << h;
  }

  /// @brief Called when the user's inductive is not printable.
  ///
  /// Compile-time dispatch.
  template <typename H>
  static auto
  print_impl(std::ostream& os, const H& h, long)
  -> decltype(void())
  {
    os << "inductive(" << &h << ")";
  }

  /// @brief Called when the user's inductive has skip().
  ///
  /// Compile-time dispatch.
  template <typename H>
  static auto
  skip_impl(const H& h, const order<C>& o, int)
  noexcept
  -> decltype(h.skip(o.identifier().user()))
  {
    // We can safely pass the order_identifier as a user one because only hierarchical levels
    // can be artificial.
    assert(not o.identifier().artificial());
    return h.skip(o.identifier().user());
  }

  /// @brief Called when the user's inductive doesn't have skip().
  ///
  /// Compile-time dispatch.
  template <typename H>
  static auto
  skip_impl(const H&, const order<C>&, long)
  noexcept
  -> decltype(false)
  {
    return false;
  }

  /// @brief Called when the user's inductive has selector().
  ///
  /// Compile-time dispatch.
  template <typename H>
  static auto
  selector_impl(const H& h, int)
  noexcept
  -> decltype(h.selector())
  {
    return h.selector();
  }

  /// @brief Called when the user's inductive doesn't have selector().
  ///
  /// Compile-time dispatch.
  template <typename H>
  static auto
  selector_impl(const H&, long)
  noexcept
  -> decltype(false)
  {
    return false;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief inductive homomorphism.
template <typename C>
struct _inductive
{
  /// @brief Ownership of the user's inductive homomorphism.
  const std::unique_ptr<const inductive_base<C>> hom_ptr;

  /// @brief Dispatch the inductive homomorphism evaluation.
  struct evaluation
  {
    context<C>& cxt_;
    const order<C>& order_;
    const SDD<C> sdd_;

    SDD<C>
    operator()(const zero_terminal<C>&, const inductive_base<C>&)
    const noexcept
    {
      assert(false);
      __builtin_unreachable();
    }

    SDD<C>
    operator()(const one_terminal<C>& one, const inductive_base<C>& i)
    const
    {
      return i(one);
    }

    template <typename Node>
    SDD<C>
    operator()(const Node& node, const inductive_base<C>& inductive)
    const
    {
      dd::sum_builder<C, SDD<C>> sum_operands(cxt_.sdd_context());
      sum_operands.reserve(node.size());
      for (const auto& arc : node)
      {
        const homomorphism<C> next_hom = inductive(order_, arc.valuation());
        sum_operands.add(next_hom(cxt_, order_.next(), arc.successor()));
      }
      return dd::sum(cxt_.sdd_context(), std::move(sum_operands));
    }
  };

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& s)
  const
  {
    return visit(evaluation{cxt, o, s}, s, *hom_ptr);
  }

  /// @brief Skip predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return hom_ptr->skip(o);
  }

  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return hom_ptr->selector();
  }

  friend
  bool
  operator==(const _inductive& lhs, const _inductive& rhs)
  noexcept
  {
    return *lhs.hom_ptr == *rhs.hom_ptr;
  }

  friend
  std::ostream&
  operator<<(std::ostream& os, const _inductive& i)
  {
    i.hom_ptr->print(os);
    return os;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the inductive homomorphism.
/// @related homomorphism
template <typename C, typename User>
homomorphism<C>
inductive(const User& u)
{
  return hom::make<C, hom::_inductive<C>>(std::make_unique<hom::inductive_derived<C, User>>(u));
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::_inductive.
template <typename C>
struct hash<sdd::hom::_inductive<C>>
{
  std::size_t
  operator()(const sdd::hom::_inductive<C>& i)
  const
  {
    return i.hom_ptr->hash();
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std
