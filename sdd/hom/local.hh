#ifndef _SDD_HOM_LOCAL_HH_
#define _SDD_HOM_LOCAL_HH_

#include <iosfwd>

#include "sdd/dd/definition.hh"
#include "sdd/hom/context_fwd.hh"
#include "sdd/hom/definition_fwd.hh"
#include "sdd/hom/identity.hh"

namespace sdd { namespace hom {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Local homomorphism.
template <typename C>
class local
{
private:

  /// @brief The variable type.
  typedef typename C::Variable variable_type;

  /// @brief The hierarchical node where the nested homomorphism will be carried to.
  const variable_type variable_;

  /// @brief The nested homomorphism to apply in a nested level.
  const homomorphism<C> h_;

public:

  /// @brief Constructor.
  local(const variable_type& var, const homomorphism<C>& h)
    : variable_(var)
    , h_(h)
  {
  }

  /// @brief Local's evaluation implementation.
  struct evaluation
  {
    typedef SDD<C> result_type;

    SDD<C>
    operator()( const hierarchical_node<C>& n
              , context<C>& cxt, const variable_type& var, const homomorphism<C>& h)
    const
    {
      sum_builder<C, SDD<C>> sum_operands(n.size());
      for (const auto& arc : n)
      {
        sum_operands.add(SDD<C>(var, h(cxt, arc.valuation()), arc.successor()));
      }
      return sdd::sum(cxt.sdd_context(), std::move(sum_operands));
    }

    template <typename T>
    SDD<C>
    operator()(const T&, context<C>&, const variable_type&, const homomorphism<C>&)
    const
    {
      throw evaluation_error<C>();
    }
  };

  /// @brief Evaluation.
  SDD<C>
  operator()(context<C>& cxt, const SDD<C>& x)
  const
  {
    return apply_visitor(evaluation(), x->data(), cxt, variable_, h_);
  }

  /// @brief Skip variable predicate.
  bool
  skip(const variable_type& v)
  const noexcept
  {
    return v != variable_;
  }

  /// @brief Return the target.
  const variable_type&
  variable()
  const noexcept
  {
    return variable_;
  }

  /// @brief Return the carried homomorphism.
  homomorphism<C>
  hom()
  const noexcept
  {
    return h_;
  }
};

/*-------------------------------------------------------------------------------------------*/

/// @brief Equality of two Local homomorphisms.
/// @related local
template <typename C>
inline
bool
operator==(const local<C>& lhs, const local<C>& rhs)
noexcept
{
  return lhs.variable() == rhs.variable() and lhs.hom() == rhs.hom();
}

/// @related local
template <typename C>
std::ostream&
operator<<(std::ostream& os, const local<C>& l)
{
  return os << "@(" << l.variable() << ", " << l.hom() << ")";
}

/// @endcond

/*-------------------------------------------------------------------------------------------*/

/// @brief Create the Local homomorphism.
/// @related homomorphism
template <typename C>
homomorphism<C>
Local(const typename C::Variable& var, const homomorphism<C>& h)
{
  if (h == Id<C>())
  {
    return h;
  }
  else
  {
    return homomorphism<C>::create(internal::mem::construct<local<C>>(), var, h);
  }
}

/*-------------------------------------------------------------------------------------------*/

}} // namespace sdd::hom

namespace std {

/*-------------------------------------------------------------------------------------------*/

/// @cond INTERNAL_DOC

/// @brief Hash specialization for sdd::hom::local.
template <typename C>
struct hash<sdd::hom::local<C>>
{
  std::size_t
  operator()(const sdd::hom::local<C>& l)
  const noexcept
  {
    std::size_t seed = 0;
    sdd::internal::util::hash_combine(seed, l.variable());
    sdd::internal::util::hash_combine(seed, l.hom());
    return seed;
  }
};

/// @endcond

/*-------------------------------------------------------------------------------------------*/

} // namespace std


#endif // _SDD_HOM_FIXPOINT_HH_
