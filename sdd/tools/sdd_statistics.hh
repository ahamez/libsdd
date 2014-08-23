#pragma once

#include <utility> // pair

#include "sdd/tools/arcs.hh"
#include "sdd/tools/nodes.hh"
#include "sdd/tools/sequences.hh"
#include "sdd/tools/size.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
struct sdd_statistics
{
  const std::size_t bytes;
  const arcs_frequency_type arcs_frequency;
  const std::pair<unsigned int, unsigned int> all_nodes;
  const std::pair<unsigned int, unsigned int> all_arcs;
  const sequences_frequency_type sequences_frequency;

  sdd_statistics(const SDD<C>& x)
    : bytes(size(x))
    , arcs_frequency(arcs(x))
    , all_nodes(nodes(x))
    , all_arcs(number_of_arcs(arcs_frequency))
    , sequences_frequency(sequences(x))
  {}
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools
