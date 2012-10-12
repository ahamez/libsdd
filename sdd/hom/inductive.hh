#ifndef _SDD_HOM_INDUCTIVE_HH_
#define _SDD_HOM_INDUCTIVE_HH_

#include <iosfwd>
#include <memory> // unique_ptr

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"

namespace sdd { namespace hom {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Used to wrap user's inductive homomorphisms.
template <typename C>
class inductive_base
{
public:

  typedef typename C::Variable  variable_type;
  typedef typename C::Values    values_type;

  virtual
  ~inductive_base()
  {
  }

  /// @brief Tell if the user's inductive skip the current variable.
  virtual
  bool
  skip(const variable_type&) const noexcept = 0;

  /// @brief Tell if the user's inductive is a selector.
  virtual
  bool
  selector() const noexcept = 0;

  /// @brief Get the next homomorphism to apply from the user.
  virtual
  homomorphism<C>
  operator()(const variable_type&, const SDD<C>&) const = 0;

  /// @brief Get the next homomorphism to apply from the user.
  virtual
  homomorphism<C>
  operator()(const variable_type&, const values_type&) const = 0;

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

/*-------------------------------------------------------------------------------------------*/

/// @brief Used to wrap user's inductive homomorphisms.
template <typename C, typename User>
class inductive_derived
  : public inductive_base<C>
{
private:

  /// @brief The user's inductive homomorphism.
  const User h_;

public:

  /// @brief The type of a variable.
  typedef typename C::Variable  variable_type;

  /// @brief The type of a set of values.
  typedef typename C::Values    values_type;

  /// @brief Constructor.
  inductive_derived(const User& h)
    : h_(h)
  {
  }

  /// @brief Tell if the user's inductive skip the current variable.
  bool
  skip(const variable_type& var)
  const noexcept
  {
    return skip_impl(h_, var, 0);
  }

  /// @brief Tell if the user's inductive is a selector.
  bool
  selector()
  const noexcept
  {
    return selector_impl(h_, 0);
  }

  /// @brief Get the next homomorphism to apply from the user.
  homomorphism<C>
  operator()(const variable_type& var, const SDD<C>& x)
  const
  {
    return h_(var, x);
  }

  /// @brief Get the next homomorphism to apply from the user.
  homomorphism<C>
  operator()(const variable_type& var, const values_type& val)
  const
  {
    return h_(var, val);
  }

  /// @brief Get the terminal case from the user.
  SDD<C>
  operator()(const one_terminal<C>&)
  const
  {
    return h_();
  }

  /// @brief Compare inductive_derived.
  bool
  operator==(const inductive_base<C>& other)
  const noexcept
  {
    try
    {
      return h_ == dynamic_cast<const inductive_derived&>(other).h_;
    }
    catch(std::bad_cast)
    {
      return false;
    }
  }

  /// @brief Get the user's inductive hash value.
  std::size_t
  hash()
  const noexcept
  {
    return std::hash<User>()(h_);
  }

  /// @brief Get the user's inductive textual representation.
  void
  print(std::ostream& os)
  const
  {
    os << h_;
  }

private:

  /// @brief Called when the user's inductive has skip().
  ///
  /// Compile-time dispatch.
  template <typename T>
  static auto
  skip_impl(const T& x, const variable_type& v, int)
  noexcept
  -> decltype(x.skip(v))
  {
    return x.skip(v);
  }

  /// @brief Called when the user's inductive doesn't have skip().
  ///
  /// Compile-time dispatch.
  template <typename T>
  static auto
  skip_impl(const T&, const variable_type&, long)
  noexcept
  -> decltype(false)
  {
    return false;
  }

  /// @brief Called when the user's inductive has selector().
  ///
  /// Compile-time dispatch.
  template <typename T>
  static auto
  selector_impl(const T& x, int)
  noexcept
  -> decltype(x.selector())
  {
    return x.selector();
  }

  /// @brief Called when the user's inductive doesn't have selector().
  ///
  /// Compile-time dispatch.
  template <typename T>
  static auto
  selector_impl(const T&, long)
  noexcept
  -> decltype(false)
  {
    return false;
  }
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Inductive homomorphism.
template <typename C>
class inductive
{
private:

  /// @brief Ownership of the user's inductive homomorphism.
  const std::unique_ptr<const inductive_base<C>> hom_ptr_;

  /// @brief Dispatch the inductive homomorphism evaluation.
  struct helper
  {
    typedef SDD<C> result_type;

    SDD<C>
    operator()(const zero_terminal<C>&, const inductive_base<C>&, context<C>&)
    const noexcept
    {
      assert(false);
      __builtin_unreachable();
    }

    SDD<C>
    operator()(const one_terminal<C>& one, const inductive_base<C>& i, context<C>&)
    const
    {
      return i(one);
    }

    template <typename Node>
    SDD<C>
    operator()(const Node& node, const inductive_base<C>& i, context<C>& cxt)
    const
    {
      sum_builder<C, SDD<C>> sum_operands(node.size());
      for (const auto& arc : node)
      {
        const homomorphism<C> next_hom = i(node.variable(), arc.valuation());
        sum_operands.add(next_hom(arc.successor()));
      }
      return sdd::sum(cxt.sdd_context(), std::move(sum_operands));
    }
  };

public:

  /// @brief Constructor.
  inductive(const inductive_base<C>* i_ptr)
    : hom_ptr_(i_ptr)
  {
  }

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const SDD<C>& x)
  const
  {
    return apply_visitor(helper(), x->data(), *hom_ptr_, cxt);
  }

  /// @brief Skip variable predicate.
  bool
  skip(const typename C::Variable& var)
  const noexcept
  {
    return hom_ptr_->skip(var);
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

/*-------------------------------------------------------------------------------------------*/

/// @brief Equality of two inductive homomorphisms.
/// @related inductive
template <typename C>
inline
bool
operator==(const inductive<C>& lhs, const inductive<C>& rhs)
noexcept
{
  return lhs.hom() == rhs.hom();
}

/// @related inductive
template <typename C>
std::ostream&
operator<<(std::ostream& os, const inductive<C>& i)
{
  i.hom().print(os);
  return os;
}

/// @endcond

/*-------------------------------------------------------------------------------------------*/

/// @brief Create the Inductive homomorphism.
/// @related homomorphism
template <typename C, typename User>
homomorphism<C>
Inductive(const User& u)
{
  return homomorphism<C>::create( internal::mem::construct<inductive<C>>()
                                , new inductive_derived<C, User>(u));
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

namespace std {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Hash specialization for sdd::hom::inductive.
template <typename C>
struct hash<sdd::hom::inductive<C>>
{
  std::size_t
  operator()(const sdd::hom::inductive<C>& i)
  const noexcept
  {
    return i.hom().hash();
  }
};

/// @endcond

/*-------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_INDUCTIVE_HH_
