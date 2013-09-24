#ifndef _SDD_HOM_EXPRESSION_EVALUATOR_HH_
#define _SDD_HOM_EXPRESSION_EVALUATOR_HH_

#include <iosfwd>
#include <typeinfo>  // typeid

namespace sdd { namespace hom { namespace expr {

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
class evaluator_base
{
public:

  /// @brief The type of a variable.
  using identifier_type = typename C::Identifier;

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  virtual
  ~evaluator_base()
  {}

  /// @brief Update an identifier with a set of values when it is encountered when walking the SDD.
  virtual
  void
  update(const identifier_type&, const values_type&) = 0;

  /// @brief Called when all identifiers have been updated, thus when the expression can be
  /// evaluated on a path of the SDD.
  virtual
  values_type
  evaluate() = 0;

  /// @brief Compare evaluator_base.
  virtual
  bool
  operator==(const evaluator_base&) const noexcept = 0;

  /// @brief Get the user's evaluator hash value.
  virtual
  std::size_t
  hash() const noexcept = 0;

  /// @brief Get the user's evaluator textual representation.
  virtual
  void
  print(std::ostream&) const = 0;
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Used to wrap user's evaluator.
template <typename C, typename Evaluator>
class evaluator_derived
  : public evaluator_base<C>
{
private:

  /// @brief The user's evaluator.
  Evaluator eval_;

public:

  /// @brief The type of a variable.
  using identifier_type = typename C::Identifier;

  /// @brief The type of a set of values.
  using values_type = typename C::Values;

  /// @brief Constructor.
  evaluator_derived(const Evaluator& eval)
    : eval_(eval)
  {}

  /// @brief Update an identifier with a set of values when it is encountered when walking the SDD.
  void
  update(const identifier_type& id, const values_type& values)
  override
  {
    eval_.update(id, values);
  }

  /// @brief Called when all identifiers have been updated, thus when the expression can be
  /// evaluated on a path of the SDD.
  values_type
  evaluate()
  override
  {
    return eval_.evaluate();
  }

  /// @brief Compare evaluator_derived.
  bool
  operator==(const evaluator_base<C>& other)
  const noexcept override
  {
    return typeid(*this) == typeid(other)
         ? eval_ == reinterpret_cast<const evaluator_derived&>(other).eval_
         : false;
  }

  /// @brief Get the user's evaluator hash value.
  std::size_t
  hash()
  const noexcept override
  {
    return std::hash<Evaluator>()(eval_);
  }

  /// @brief Get the user's evaluator textual representation.
  void
  print(std::ostream& os)
  const override
  {
    print_impl(os, eval_, 0);
  }

private:

  /// @brief Called when the user's evaluator is printable.
  ///
  /// Compile-time dispatch.
  template <typename E>
  static auto
  print_impl(std::ostream& os, const E& e, int)
  -> decltype(operator<<(os, e))
  {
    return os << e;
  }

  /// @brief Called when the user's evaluator is not printable.
  ///
  /// Compile-time dispatch.
  template <typename E>
  static auto
  print_impl(std::ostream& os, const E& e, long)
  -> decltype(void())
  {
    os << "UserEvaluator(" << &e << ")";
  }
};

/*------------------------------------------------------------------------------------------------*/

}}} // namespace sdd::hom::expr

#endif // _SDD_HOM_EXPRESSION_EVALUATOR_HH_
