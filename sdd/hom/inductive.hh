#ifndef _SDD_HOM_INDUCTIVE_HH_
#define _SDD_HOM_INDUCTIVE_HH_

#include <cassert>
#include <iosfwd>
#include <memory>   // unique_ptr
#include <typeinfo> // typeid

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation_error.hh"
#include "sdd/util/packed.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Used to wrap user's inductive homomorphisms.
template <typename C>
class inductive_base
{
public:

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
class inductive_derived
  : public inductive_base<C>
{
private:

  /// @brief The user's inductive homomorphism.
  const User h_;

public:

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief Constructor.
  inductive_derived(const User& h)
    : h_(h)
  {}

  /// @brief Constructor.
  inductive_derived(User&& h)
    : h_(std::move(h))
  {}

  /// @brief Tell if the user's inductive skip the current variable.
  bool
  skip(const order<C>& o)
  const noexcept override
  {
    return skip_impl(h_, o, 0);
  }

  /// @brief Tell if the user's inductive is a selector.
  bool
  selector()
  const noexcept override
  {
    return selector_impl(h_, 0);
  }

  /// @brief Get the next homomorphism to apply from the user.
  homomorphism<C>
  operator()(const order<C>& o, const SDD<C>& x)
  const override
  {
    return h_(o, x);
  }

  /// @brief Get the next homomorphism to apply from the user.
  homomorphism<C>
  operator()(const order<C>& o, const values_type& val)
  const override
  {
    return h_(o, val);
  }

  /// @brief Get the base case from the user.
  SDD<C>
  operator()(const one_terminal<C>&)
  const override
  {
    return h_();
  }

  /// @brief Compare inductive_derived.
  bool
  operator==(const inductive_base<C>& other)
  const noexcept override
  {
    return typeid(*this) == typeid(other)
         ? h_ == reinterpret_cast<const inductive_derived&>(other).h_
         : false;
  }

  /// @brief Get the user's inductive hash value.
  std::size_t
  hash()
  const noexcept(noexcept(std::hash<User>()(h_))) override
  {
    return std::hash<User>()(h_);
  }

  /// @brief Get the user's inductive textual representation.
  void
  print(std::ostream& os)
  const override
  {
    print_impl(os, h_, 0);
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
class _inductive
{
private:

  /// @brief Ownership of the user's inductive homomorphism.
  const std::unique_ptr<const inductive_base<C>> hom_ptr_;

  /// @brief Dispatch the inductive homomorphism evaluation.
  struct evaluation
  {
    using result_type = SDD<C>;

    context<C>& cxt_;
    const order<C>& order_;
    const SDD<C> sdd_;

    evaluation(context<C>& c, const order<C>& o, const SDD<C>& s)
    noexcept
      : cxt_(c), order_(o), sdd_(s)
    {}

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
    operator()(const Node& node, const inductive_base<C>& i)
    const
    {
      dd::sum_builder<C, SDD<C>> sum_operands;
      sum_operands.reserve(node.size());
      for (const auto& arc : node)
      {
        const homomorphism<C> next_hom = i(order_, arc.valuation());
        sum_operands.add(next_hom(cxt_, order_.next(), arc.successor()));
      }

      try
      {
        return dd::sum(cxt_.sdd_context(), std::move(sum_operands));
      }
      catch (top<C>& t)
      {
        evaluation_error<C> e(sdd_);
        e.add_top(t);
        throw e;
      }
    }
  };

public:

  /// @brief Constructor.
  _inductive(const inductive_base<C>* i_ptr)
    : hom_ptr_(i_ptr)
  {}

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& s)
  const
  {
    return visit(evaluation{cxt, o, s}, s, *hom_ptr_);
  }

  /// @brief Skip predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return hom_ptr_->skip(o);
  }

  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return hom_ptr_->selector();
  }

  /// @brief Return the user's inductive homomorphism
  const inductive_base<C>&
  hom()
  const noexcept
  {
    return *hom_ptr_;
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @related _inductive
template <typename C>
inline
bool
operator==(const _inductive<C>& lhs, const _inductive<C>& rhs)
noexcept
{
  return lhs.hom() == rhs.hom();
}

/// @internal
/// @related _inductive
template <typename C>
std::ostream&
operator<<(std::ostream& os, const _inductive<C>& i)
{
  i.hom().print(os);
  return os;
}

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the inductive homomorphism.
/// @related homomorphism
template <typename C, typename User>
homomorphism<C>
inductive(const User& u)
{
  return homomorphism<C>::create( mem::construct<hom::_inductive<C>>()
                                , new hom::inductive_derived<C, User>(u));
}

/// @brief Create the inductive homomorphism.
/// @related homomorphism
template <typename C, typename User>
homomorphism<C>
inductive(User&& u)
{
  return homomorphism<C>::create( mem::construct<hom::_inductive<C>>()
                                , new hom::inductive_derived<C, User>(std::move(u)));
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
    return i.hom().hash();
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_INDUCTIVE_HH_
