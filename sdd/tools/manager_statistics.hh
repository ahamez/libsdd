/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include "sdd/tools/arcs.hh"
#include "sdd/tools/nodes.hh"
#include "sdd/tools/size.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
class manager_statistics
{
private:

  mem::unique_table_statistics sdd_ut_;
  mem::cache_statistics diff_cache_;
  mem::cache_statistics inter_cache_;
  mem::cache_statistics sum_cache_;
  mem::unique_table_statistics hom_ut_;
  mem::cache_statistics hom_cache_;
  mem::unique_table_statistics values_ut_;

public:

  manager_statistics(const manager<C>& m)
    : sdd_ut_(m.sdd_stats())
    , diff_cache_(m.sdd_difference_cache_stats())
    , inter_cache_(m.sdd_intersection_cache_stats())
    , sum_cache_(m.sdd_sum_cache_stats())
    , hom_ut_(m.hom_stats())
    , hom_cache_(m.hom_cache_stats())
    , values_ut_(m.values_stats())
  {}

  const mem::unique_table_statistics& sdd_ut()    const noexcept {return sdd_ut_;}
  const mem::cache_statistics& diff_cache()       const noexcept {return diff_cache_;}
  const mem::cache_statistics& inter_cache()      const noexcept {return inter_cache_;}
  const mem::cache_statistics& sum_cache()        const noexcept {return sum_cache_;}
  const mem::unique_table_statistics& hom_ut()    const noexcept {return hom_ut_;}
  const mem::cache_statistics& hom_cache()        const noexcept {return hom_cache_;}
  const mem::unique_table_statistics& values_ut() const noexcept {return values_ut_;}
};

/*------------------------------------------------------------------------------------------------*/

template <typename C>
manager_statistics<C>
statistics(const manager<C>& m)
{
  return {m};
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools
