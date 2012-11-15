#include <chrono>
#include <iostream>
#include <set>

#include "sdd/sdd.hh"

/*------------------------------------------------------------------------------------------------*/

typedef sdd::conf::conf1 conf;
typedef sdd::SDD<conf> SDD;
typedef sdd::homomorphism<conf> hom;
typedef sdd::conf::conf1::Values Values;

SDD one = sdd::one<conf>();
SDD zero = sdd::zero<conf>();

using sdd::Cons;
using sdd::Constant;
using sdd::Fixpoint;
using sdd::Inductive;
using sdd::Sum;

/*------------------------------------------------------------------------------------------------*/

struct no_ring_above
{
  const unsigned int i;
  const unsigned int j;

  no_ring_above(int _i, int _j)
    : i(_i<_j?_i:_j)
    , j(_i<_j?_j:_i)
  {}

  bool
  skip(unsigned char var)
  const noexcept
  {
    return false;
  }

  hom
  operator()(unsigned int, const SDD&)
  const noexcept
  {
    // no hierarchy
    assert(false);
    __builtin_unreachable();
  }

  hom
  operator()(unsigned int var, const Values& val)
  const
  {
    Values v(val);
    v.erase(i);
    v.erase(j);
    return Cons<conf>(var, std::move(v), Inductive<conf>(*this));
  }

  SDD
  operator()()
  const noexcept
  {
    return one;
  }

  bool
  operator==(const no_ring_above& other)
  const noexcept
  {
    return i == other.i and j == other.j;
  }
};

std::ostream&
operator<<(std::ostream& os, const no_ring_above& x)
{
  return os << "nra(" << x.i << "," << x.j << ")";
}

/*------------------------------------------------------------------------------------------------*/

struct swap_pole
{

  const unsigned int ring;
  const unsigned int source;
  const unsigned int destination;

  swap_pole(int ring, int source, int destination)
    : ring(ring)
    , source(source)
    , destination(destination)
  {}

  bool
  skip(unsigned int var)
  const noexcept
  {
    return var != ring;
  }

  hom
  operator()(unsigned int, const SDD&)
  const noexcept
  {
    // no hierarchy
    assert(false);
    __builtin_unreachable();
  }

  hom
  operator()(unsigned int, const Values& val)
  const
  {
    if (val.find(source) == val.cend())
    {
      return Constant<conf>(zero);
    }
    else
    {
      return Cons<conf>( ring
                       , Values {destination}
                       , Inductive<conf>(no_ring_above(source,destination)));
    }
  }

  SDD
  operator()()
  const noexcept
  {
    return one;
  }

  bool
  operator==(const swap_pole& other)
  const noexcept
  {
    return ring == other.ring and source == other.source and destination == other.destination;
  }
};

std::ostream&
operator<<(std::ostream& os, const swap_pole& x)
{
  return os << "swap_pole(" << x.ring << "," << x.source << "," << x.destination << ")";
}

/*------------------------------------------------------------------------------------------------*/

namespace std {

template <>
struct hash<no_ring_above>
{

  std::size_t
  operator()(const no_ring_above& x)
  const noexcept
  {
    std::size_t seed = 0;
    sdd::util::hash_combine(seed, x.i);
    sdd::util::hash_combine(seed, x.j);
    return seed;
  }
};

template <>
struct hash<swap_pole>
{

  std::size_t  
  operator()(const swap_pole& x)
  const noexcept
  {
    std::size_t seed = 0;
    sdd::util::hash_combine(seed, x.ring);
    sdd::util::hash_combine(seed, x.source);
    sdd::util::hash_combine(seed, x.destination);
    return seed;
  }
};

}

/*------------------------------------------------------------------------------------------------*/

int
main(int argc, char** argv)
{
  // The default number of rings
  unsigned int nb_rings = 5;
  if (argc >= 2)
  {
    nb_rings = atoi(argv[1]);
  }

  // The default number of poles
  unsigned int nb_poles = 3;
  if (argc >= 3)
  {
    nb_poles = atoi(argv[2]);
  }

  /// Initial state
  SDD m0 = one;
  for (unsigned int i = 0; i < nb_rings; ++i)
  {
    m0 = SDD(i, {0}, m0);
  }

  /// Events
  std::set<hom> union_swap_pole;
  for (unsigned int i = 0; i < nb_rings; ++i)
  {
    for (unsigned int source = 0; source < nb_poles; ++source)
    {
      for (unsigned int destination = 0; destination < nb_poles; ++destination)
      {
        if (source != destination)
        {
          union_swap_pole.insert(Inductive<conf>(swap_pole(i, source, destination)));
        }
      }
    }
  }
  union_swap_pole.insert(sdd::Id<conf>());
  hom events = Fixpoint(Sum<conf>(union_swap_pole.begin(), union_swap_pole.end()));

  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  /// Apply events
  SDD final = events(m0);
  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  const std::size_t elapsed = std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
  std::cout << "Time: " << elapsed << "s" << std::endl;

  /// Number of distinct paths
  std::cout << "Number of states : " << sdd::count_paths(final) << std::endl;

  return 0;
}
