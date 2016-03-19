#include <chrono>
#include <iostream>
#include <set>

#include "sdd/sdd.hh"

/*------------------------------------------------------------------------------------------------*/

using conf   = sdd::conf2;
using SDD    = sdd::SDD<conf>;
using hom    = sdd::homomorphism<conf>;
using Values = conf::Values;

using sdd::cons;
using sdd::constant;
using sdd::fixpoint;
using sdd::id;
using sdd::inductive;
using sdd::sum;

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
  skip(unsigned char)
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
    return cons<conf>(o, std::move(v), inductive<conf>(*this));
  }

  SDD
  operator()()
  const noexcept
  {
    return sdd::one<conf>();
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
      return constant<conf>(sdd::zero<conf>());
    }
    else
    {
      return cons<conf>( o
                       , Values {destination}
                       , inductive<conf>(no_ring_above(source,destination)));
    }
  }

  SDD
  operator()()
  const noexcept
  {
    return sdd::one<conf>();
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
    using namespace sdd::hash;
    return seed() (val(x.i)) (val(x.j));
  }
};

template <>
struct hash<swap_pole>
{
  std::size_t
  operator()(const swap_pole& x)
  const noexcept
  {
    using namespace sdd::hash;
    return seed() (val(x.ring)) (val(x.source)) (val(x.destination));
  }
};

} // namespace std

/*------------------------------------------------------------------------------------------------*/

int
main(int argc, char** argv)
{
  auto manager = sdd::init<conf>();

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
    ob.push(i);
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
          union_swap_pole.insert(inductive<conf>(swap_pole(i, source, destination)));
        }
      }
    }
  }

  union_swap_pole.insert(id<conf>());
  hom events = fixpoint(sum(o, union_swap_pole.begin(), union_swap_pole.end()));
  events = sdd::rewrite(o, events);

  std::chrono::time_point<std::chrono::system_clock> start, end;
  std::size_t elapsed;

  // Apply saturated events
  start = std::chrono::system_clock::now();
  SDD sat_final = events(o, m0);
  end = std::chrono::system_clock::now();
  elapsed = std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
  std::cout << "Time: " << elapsed << "s" << std::endl;
  // Number of distinct paths
  std::cout << "Number of states : " << sat_final.size() << std::endl;

  return 0;
}
