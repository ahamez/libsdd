#include <chrono>
#include <iostream>
#include <set>

#include "sdd/sdd.hh"

/*------------------------------------------------------------------------------------------------*/

struct conf
  : public sdd::conf1
{
  typedef unsigned int Identifier;
};
typedef sdd::SDD<conf> SDD;
typedef sdd::homomorphism<conf> hom;
typedef sdd::conf1::Values Values;

SDD one = sdd::one<conf>();
SDD zero = sdd::zero<conf>();

using sdd::Cons;
using sdd::Constant;
using sdd::Fixpoint;
using sdd::Id;
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
  operator()(const sdd::order<conf>&, const SDD&)
  const noexcept
  {
    // no hierarchy
    assert(false);
    __builtin_unreachable();
  }

  hom
  operator()(const sdd::order<conf>& o, const Values& val)
  const
  {
    Values v(val);
    v.erase(i);
    v.erase(j);
    return Cons<conf>(o, std::move(v), Inductive<conf>(*this));
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
  operator()(const sdd::order<conf>&, const SDD&)
  const noexcept
  {
    // no hierarchy
    assert(false);
    __builtin_unreachable();
  }

  hom
  operator()(const sdd::order<conf>& o, const Values& val)
  const
  {
    if (val.find(source) == val.cend())
    {
      return Constant<conf>(zero);
    }
    else
    {
      return Cons<conf>( o
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

  /// Order
  sdd::order_builder<conf> ob;
  for (unsigned int i = 0; i < nb_rings; ++i)
  {
    ob.add(i);
  }
  sdd::order<conf> o(ob);

  /// Initial state
  SDD m0(o, [](unsigned int){return Values {0};});

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

  union_swap_pole.insert(Id<conf>());
  hom events = Fixpoint(Sum<conf>(o, union_swap_pole.begin(), union_swap_pole.end()));
  hom sat_events = sdd::rewrite(events, o);

  std::chrono::time_point<std::chrono::system_clock> start, end;
  std::size_t elapsed;

  // Apply events
  start = std::chrono::system_clock::now();
  SDD final = events(o, m0);
  end = std::chrono::system_clock::now();
  elapsed = std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
  std::cout << "Time: " << elapsed << "s" << std::endl;
  // Number of distinct paths
  std::cout << "Number of states : " << sdd::count_paths(final) << std::endl;

  // Apply saturated events
  start = std::chrono::system_clock::now();
  SDD sat_final = sat_events(o, m0);
  end = std::chrono::system_clock::now();
  elapsed = std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
  std::cout << "Time: " << elapsed << "s" << std::endl;
  // Number of distinct paths
  std::cout << "Number of states : " << sdd::count_paths(sat_final) << std::endl;

  return 0;
}
