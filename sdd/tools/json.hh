#ifndef _SDD_TOOLS_JSON_HH_
#define _SDD_TOOLS_JSON_HH_

#include <iosfwd>
#include <tuple>

#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/utility.hpp>

#include "sdd/manager.hh"
#include "sdd/mem/unique_table.hh"
#include "sdd/tools/arcs.hh"
#include "sdd/tools/nodes.hh"
#include "sdd/tools/size.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

namespace mem { namespace {

template<class Archive>
void save(Archive & archive, const unique_table_statistics& s)
{
  archive( cereal::make_nvp("#", static_cast<unsigned int>(s.size))
         , cereal::make_nvp("# peak", static_cast<unsigned int>(s.peak))
         , cereal::make_nvp("# access", static_cast<unsigned int>(s.access))
         , cereal::make_nvp("# hits", static_cast<unsigned int>(s.hit))
         , cereal::make_nvp("# miss", static_cast<unsigned int>(s.miss))
         , cereal::make_nvp("load factor", static_cast<unsigned int>(s.load_factor))
         );
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace mem::anonymous

namespace tools { namespace /* anonymous */ {

/*------------------------------------------------------------------------------------------------*/

struct sdd_stats
{
  unsigned int flat_nodes;
  unsigned int hierarchical_nodes;
  unsigned int flat_arcs;
  unsigned int hierarchical_arcs;
  unsigned int bytes;
  arcs_frequency_type frequency;

  template<class Archive>
  void
  save(Archive& archive)
  const
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

/// @internal
template <typename C>
void
json(const manager<C>& m, std::ostream& os)
{
  cereal::JSONOutputArchive archive(os);

  archive( cereal::make_nvp("SDD unique table", m.sdd_stats())
         , cereal::make_nvp("hom unique table", m.hom_stats()));
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools

#endif // _SDD_TOOLS_JSON_HH_
