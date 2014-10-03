#pragma once

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

template <typename C>
class homomorphism;

/*------------------------------------------------------------------------------------------------*/

namespace hom {

/*------------------------------------------------------------------------------------------------*/

template<typename C, typename T, typename... Args>
homomorphism<C>
make_variable_size(std::size_t, Args&&...);

template<typename C, typename T, typename... Args>
homomorphism<C>
make(Args&&...);

/*------------------------------------------------------------------------------------------------*/
  
}} // namespace sdd::hom
