#ifndef _SDD_TOOLS_SERIALIZATION_HH_
#define _SDD_TOOLS_SERIALIZATION_HH_

#include <iosfwd>
#include <tuple>

#include <cereal/types/forward_list.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/utility.hpp>

#include "sdd/manager.hh"
#include "sdd/mem/unique_table.hh"
#include "sdd/tools/arcs.hh"
#include "sdd/tools/nodes.hh"
#include "sdd/tools/sdd_statistics.hh"
#include "sdd/tools/size.hh"

namespace sdd { namespace mem { namespace /* anonymous */ {

/*------------------------------------------------------------------------------------------------*/

template<class Archive>
void
save(Archive& archive, const unique_table_statistics& s)
{
  archive( cereal::make_nvp("#", static_cast<unsigned int>(s.size))
         , cereal::make_nvp("# peak", static_cast<unsigned int>(s.peak))
         , cereal::make_nvp("# access", static_cast<unsigned int>(s.access))
         , cereal::make_nvp("# hits", static_cast<unsigned int>(s.hits))
         , cereal::make_nvp("# misses", static_cast<unsigned int>(s.misses))
         , cereal::make_nvp("load factor", static_cast<unsigned int>(s.load_factor))
         );
}

/*------------------------------------------------------------------------------------------------*/

template<class Archive>
void
save(Archive& archive, const cache_statistics::round& r)
{
  archive( cereal::make_nvp("# hits", static_cast<unsigned int>(r.hits))
         , cereal::make_nvp("# misses", static_cast<unsigned int>(r.misses))
         , cereal::make_nvp("# filtered", static_cast<unsigned int>(r.filtered))
         );
}

/*------------------------------------------------------------------------------------------------*/

template<class Archive>
void
save(Archive& archive, const cache_statistics& s)
{
  archive( cereal::make_nvp("# cleanup", static_cast<unsigned int>(s.cleanups()))
         , cereal::make_nvp("total", s.total())
         , cereal::make_nvp("rounds", s.rounds)
         );
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace mem::anonymous

/*------------------------------------------------------------------------------------------------*/

template<typename C, typename Archive>
void
save(Archive& archive, const manager<C>& m)
{
  archive( cereal::make_nvp("SDD unique table", m.sdd_stats())
         , cereal::make_nvp("SDD differences cache", m.sdd_difference_cache_stats())
         , cereal::make_nvp("SDD intersections cache", m.sdd_intersection_cache_stats())
         , cereal::make_nvp("SDD sums cache", m.sdd_sum_cache_stats())
         , cereal::make_nvp("hom unique table", m.hom_stats())
         , cereal::make_nvp("hom cache", m.hom_cache_stats())
         );
}

/*------------------------------------------------------------------------------------------------*/

namespace tools {


template<typename C, typename Archive>
void
save(Archive& archive, const sdd_statistics<C>& stats)
{
  archive( cereal::make_nvp("bytes", static_cast<unsigned int>(stats.bytes))
         , cereal::make_nvp("flat nodes", stats.all_nodes.first)
         , cereal::make_nvp("hierarchical nodes", stats.all_nodes.second)
         , cereal::make_nvp("flat arcs", stats.all_arcs.first)
         , cereal::make_nvp("hierarchical arcs", stats.all_arcs.second)
         , cereal::make_nvp("arcs frequency", stats.frequency)
         );
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools

#endif // _SDD_TOOLS_SERIALIZATION_HH_
