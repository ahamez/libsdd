#ifndef _SDD_HOM_IF_THEN_ELSE_HH_
#define _SDD_HOM_IF_THEN_ELSE_HH_

#include <iosfwd>
#include <stdexcept> // invalid_argument

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/evaluation_error.hh"
#include "sdd/hom/identity.hh"
#include "sdd/hom/interrupt.hh"
#include "sdd/order/order.hh"

namespace sdd { namespace hom {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief if then else homomorphism.
template <typename C>
class _if_then_else
{
private:

  /// @brief The predicate (acts as filter).
  const homomorphism<C> h_if_;

  /// @brief The true branch (works on the accepted part).
  const homomorphism<C> h_then_;

  /// @brief The false branch (works on the rejected part).
  const homomorphism<C> h_else_;

public:

  /// @brief Constructor.
  _if_then_else( const homomorphism<C>& h_if, const homomorphism<C>& h_then
               , const homomorphism<C>& h_else)
    : h_if_(h_if), h_then_(h_then), h_else_(h_else)
  {}

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const order<C>& o, const SDD<C>& s)
  const
  {
    // Apply predicate.
    const auto tmp = h_if_(cxt, o, s);

    dd::sum_builder<C, SDD<C>> sum_operands(cxt.sdd_context());
    sum_operands.reserve(2);

    try
    {
      // Apply "then" on the part accepted by the predicate.
      sum_operands.add(h_then_(cxt, o, tmp));

      // Apply "else" on the part rejected by the predicate.
      sum_operands.add(h_else_(cxt, o, dd::difference(cxt.sdd_context(), s, tmp)));

      return dd::sum(cxt.sdd_context(), std::move(sum_operands));
    }
    catch (top<C>& t)
    {
      evaluation_error<C> e(s);
      e.add_top(t);
      throw e;
    }
  }

  /// @brief Skip predicate.
  bool
  skip(const order<C>& o)
  const noexcept
  {
    return h_if_.skip(o) and h_else_.skip(o) and h_then_.skip(o);
  }

  /// @brief Selector predicate
  bool
  selector()
  const noexcept
  {
    return h_if_.selector() and h_else_.selector() and h_then_.selector();
  }

  /// @brief Equality.
  friend
  bool
  operator==(const _if_then_else& lhs, const _if_then_else& rhs)
  noexcept
  {
    return lhs.h_if_ == rhs.h_if_ and lhs.h_then_ == rhs.h_then_ and lhs.h_else_ == rhs.h_else_;
  }

  /// @brief Textual output.
  friend
  std::ostream&
  operator<<(std::ostream& os, const _if_then_else<C>& ite)
  {
    return os << "ite(" << ite.h_if_ << ", " << ite.h_then_ << "," << ite.h_else_ << ")";
  }

  std::size_t
  hash()
  const noexcept
  {
    std::size_t seed = sdd::util::hash(h_if_);
    sdd::util::hash_combine(seed, h_then_);
    sdd::util::hash_combine(seed, h_else_);
    return seed;
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace hom

/*------------------------------------------------------------------------------------------------*/

/// @brief Create the "if then else" homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
if_then_else( const homomorphism<C>& h_if, const homomorphism<C>& h_then
            , const homomorphism<C>& h_else)
{
  if (not h_if.selector())
  {
    throw std::invalid_argument("Predicate for 'if then else' must be a selector.");
  }
  if (h_if == id<C>())
  {
    return h_then;
  }
  return homomorphism<C>::create( mem::construct<hom::_if_then_else<C>>()
                                , h_if, h_then, h_else);
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

namespace std {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Hash specialization for sdd::hom::_if_then_else.
template <typename C>
struct hash<sdd::hom::_if_then_else<C>>
{
  std::size_t
  operator()(const sdd::hom::_if_then_else<C>& ite)
  const
  {
    return ite.hash();
  }
};

/*------------------------------------------------------------------------------------------------*/

} // namespace std

#endif // _SDD_HOM_IF_THEN_ELSE_HH_
