#ifndef _SDD_TOOLS_JSON_HH_
#define _SDD_TOOLS_JSON_HH_

#include <iosfwd>
#include <tuple>

#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/utility.hpp>

#include "sdd/manager.hh"
#include "sdd/tools/arcs.hh"
#include "sdd/tools/nodes.hh"
#include "sdd/tools/size.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

namespace /* anonymous */ {

struct sdd_stats
{
  unsigned int flat_nodes;
  unsigned int hierarchical_nodes;
  unsigned int flat_arcs;
  unsigned int hierarchical_arcs;
  unsigned int bytes;
  arcs_frequency_type frequency;

  template<class Archive>
  void serialize(Archive& archive)
  {
    archive( cereal::make_nvp("bytes", bytes)
           , cereal::make_nvp("flat nodes", flat_nodes)
           , cereal::make_nvp("hierarchical nodes", hierarchical_nodes)
           , cereal::make_nvp("flat arcs", flat_arcs)
           , cereal::make_nvp("hierarchical arcs", hierarchical_nodes)
           , cereal::make_nvp("arcs frequency", frequency)
           );
  }
};

} // namespace anonymous

/*------------------------------------------------------------------------------------------------*/

/// @internal
template <typename C>
void
json(const SDD<C>& x, std::ostream& os)
{
  cereal::JSONOutputArchive archive(os);
  sdd_stats stats;

  std::tie(stats.flat_nodes, stats.hierarchical_nodes) = nodes(x);
  stats.bytes = static_cast<unsigned int>(size(x));
  stats.frequency = arcs(x);
  std::tie(stats.flat_arcs, stats.hierarchical_arcs) = number_of_arcs(stats.frequency);

  archive(cereal::make_nvp("sdd", stats));
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools

#endif // _SDD_TOOLS_JSON_HH_
