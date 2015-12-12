/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <iosfwd>
#include <tuple>

#include <cereal/types/map.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/utility.hpp>

#include "sdd/manager.hh"
#include "sdd/mem/unique_table.hh"
#include "sdd/tools/arcs.hh"
#include "sdd/tools/nodes.hh"
#include "sdd/tools/sdd_statistics.hh"
#include "sdd/tools/size.hh"

namespace sdd { namespace mem {

/*------------------------------------------------------------------------------------------------*/

template<class Archive>
void
save(Archive& archive, const unique_table_statistics& s)
{
  archive( cereal::make_nvp("#", s.size)
         , cereal::make_nvp("# peak", s.peak)
         , cereal::make_nvp("# accesses", s.access)
         , cereal::make_nvp("# hits", s.hits)
         , cereal::make_nvp("# misses", s.misses)
         , cereal::make_nvp("# rehash", s.rehash)
         , cereal::make_nvp("# collisions", s.collisions)
         , cereal::make_nvp("# alone", s.alone)
         , cereal::make_nvp("# empty", s.empty)
         , cereal::make_nvp("# buckets", s.buckets)
         , cereal::make_nvp("load factor", s.load_factor));
}

/*------------------------------------------------------------------------------------------------*/

template<class Archive>
void
save(Archive& archive, const cache_statistics& c)
{
  archive( cereal::make_nvp("#", c.size)
         , cereal::make_nvp("# hits", c.hits)
         , cereal::make_nvp("# misses", c.misses)
         , cereal::make_nvp("# filtered", c.filtered)
         , cereal::make_nvp("# discarded", c.discarded)
         , cereal::make_nvp("# collisions", c.collisions)
         , cereal::make_nvp("# alone", c.alone)
         , cereal::make_nvp("# empty", c.empty)
         , cereal::make_nvp("# buckets", c.buckets)
         , cereal::make_nvp("load factor", c.load_factor));
}

/*------------------------------------------------------------------------------------------------*/

} // namespace mem

namespace tools {

/*------------------------------------------------------------------------------------------------*/

template<typename C, typename Archive>
void
save(Archive& archive, const manager_statistics<C>& m)
{
  archive( cereal::make_nvp("SDD unique table", m.sdd_ut())
         , cereal::make_nvp("SDD differences cache", m.diff_cache())
         , cereal::make_nvp("SDD intersections cache", m.inter_cache())
         , cereal::make_nvp("SDD sums cache", m.sum_cache())
         , cereal::make_nvp("hom unique table", m.hom_ut())
         , cereal::make_nvp("hom cache", m.hom_cache())
         , cereal::make_nvp("values", m.values_ut()));
}

/*------------------------------------------------------------------------------------------------*/

template<typename C, typename Archive>
void
save(Archive& archive, const sdd_statistics<C>& stats)
{
  archive( cereal::make_nvp("bytes", stats.bytes())
         , cereal::make_nvp("flat nodes", stats.all_nodes().first)
         , cereal::make_nvp("hierarchical nodes", stats.all_nodes().second)
         , cereal::make_nvp("flat arcs", stats.all_arcs().first)
         , cereal::make_nvp("hierarchical arcs", stats.all_arcs().second)
         , cereal::make_nvp("arcs frequency", stats.arcs_frequency())
         );
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools
