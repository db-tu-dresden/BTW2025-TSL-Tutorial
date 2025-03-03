#pragma once

#include <type_traits>

#include "tsl/tsl.hpp"

template <class _Target = tsl::runtime::cpu>
struct simd_helper_t {
  using executor = tsl::executor<_Target>;
  using target = _Target;
  
  template<typename T, int Par>
  using simd_by_par_t = tsl::simd<T, typename executor::template simd_ext_by_par_t<T, Par>::type>;

  template <typename T>
  static consteval auto max_par() {
    return tsl::simd<T, typename target::max_width_extension_t>::vector_element_count();
  }

  template <typename T>
  static consteval auto min_par() {
    return tsl::simd<T, typename target::min_width_extension_t>::vector_element_count();
  }

  template <typename T>
  static consteval auto available_parallelism() {
    return executor::template available_parallelism<T>();
  } 
};

using simd_helper = simd_helper_t<>; 

template <typename T>
consteval auto available_parallelism() {
  return simd_helper::template available_parallelism<T>();
}


template <int Par, typename... Ts>
constexpr bool parallelism_available_for_all() {
  return std::conjunction_v<std::bool_constant<simd_helper::executor::parallelism_available<Ts, Par>()>...>; 
}


