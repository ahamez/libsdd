/// @file
/// @copyright The code is licensed under the BSD License
///            <http://opensource.org/licenses/BSD-2-Clause>,
///            Copyright (c) 2012-2015 Alexandre Hamez.
/// @author Alexandre Hamez

#pragma once

#include <utility> // pair

#include "sdd/tools/arcs.hh"
#include "sdd/tools/nodes.hh"
#include "sdd/tools/size.hh"

namespace sdd { namespace tools {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
class sdd_statistics
{
private:

  std::size_t bytes_;
  arcs_frequency_type arcs_frequency_;
  std::pair<unsigned int, unsigned int> all_nodes_;
  std::pair<unsigned int, unsigned int> all_arcs_;

public:

  sdd_statistics(const SDD<C>& x)
    : bytes_(size(x))
    , arcs_frequency_(arcs(x))
    , all_nodes_(nodes(x))
    , all_arcs_(number_of_arcs(arcs_frequency_))
  {}

  std::size_t bytes()                                   const noexcept {return bytes_;}
  const arcs_frequency_type& arcs_frequency()           const noexcept {return arcs_frequency_;}
  std::pair<unsigned int, unsigned int> all_nodes()     const noexcept {return all_nodes_;}
  std::pair<unsigned int, unsigned int> all_arcs()      const noexcept {return all_arcs_;}
};

/*------------------------------------------------------------------------------------------------*/

template <typename C>
sdd_statistics<C>
statistics(const SDD<C>& s)
{
  return {s};
}

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::tools
