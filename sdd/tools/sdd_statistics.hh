#pragma once

#include <utility> // pair

#include "sdd/tools/arcs.hh"
#include "sdd/tools/nodes.hh"
#include "sdd/tools/size.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct sdd_statistics
{
  const std::size_t bytes;
  const arcs_frequency_type frequency;
  const std::pair<unsigned int, unsigned int> all_nodes;
  const std::pair<unsigned int, unsigned int> all_arcs;

  sdd_statistics(const SDD<C>& x)
    : bytes(size(x))
    , frequency(arcs(x))
    , all_nodes(nodes(x))
    , all_arcs(number_of_arcs(frequency))
  {}
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools
