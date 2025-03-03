#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <bit>
#include <utility>
#include "tsl/tsl.hpp"

#include "column.hpp"
#include "simd_utils.hpp"
#include "utils.hpp"

template <unsigned SimdPar, typename FilterColumnT, typename AggregateColumnT = FilterColumnT>
auto filter_aggregate_simd_impl(std::span<const AggregateColumnT> to_agg, std::span<const FilterColumnT> to_filter, FilterColumnT const filter_value) -> AggregateColumnT {

  using FilterProcStyle = simd_helper::simd_by_par_t<FilterColumnT, SimdPar>;
  using AggProcStyle = simd_helper::simd_by_par_t<AggregateColumnT, SimdPar>;
  std::cout << "\tSIMD parallelism: " << SimdPar << std::endl;
  std::cout << "\tFilter uses     : " << tsl::type_name<FilterProcStyle>() << std::endl;
  std::cout << "\tAggregation uses: " << tsl::type_name<AggProcStyle>() << std::endl;
  
  /* initializing */
  
  for (auto i = 0; i < to_filter.size(); i += SimdPar) {
    /* load filter data */
    
    /* compare filter data */
    
    /* load aggregation data */

    /* add valid values to result.
       As AVX512 returns a mask (integral value) and AVX2/SSE/NEON return a register when comparing registers, 
       we need to check the backend support. 
       As long as we use the same SIMD extension for filtering and aggregation, everything is fine. 
       However, as we will demonstrate during the tutorial, if the data type sizes of the underlying data varies,
       the degree of data-level parallelism will diverge. On AVX512-VL we can use the resulting mask type directly,
       as AVX512-VL supports masked operations on 128/256, and 512-bit registers. On AVX2/SSE/NEON, we need to convert
       the mask to an integral value to use it as a mask for the aggregation operation.
       We can check this by using the TSL_BACKEND_SUPPORTS_BY_VALUE macro, which checks if the backend supports the
      by value operation. If not, we can use the to_integral function to convert the mask to an integral value.
      */
    
  }
  /* sum up all partial results */
  return -1;
}

/*
 * This function is used to demonstrate the case where the SIMD parallelism of the filter and aggregation data types differ.
 * To cope with this, we need to use a different SIMD parallelism for the aggregation data type.
 */
template <unsigned SimdParFilter, typename FilterColumnT, unsigned SimdParAggregate, typename AggregateColumnT>
auto filter_aggregate_simd_differing_parallelism_impl(std::span<const AggregateColumnT> to_agg, std::span<const FilterColumnT> to_filter, FilterColumnT const filter_value) -> AggregateColumnT {
  static_assert((SimdParAggregate & SimdParFilter)==0, "SimdParAggregate must be a multiple of SimdParFilter");

  using FilterProcStyle = simd_helper::simd_by_par_t<FilterColumnT, SimdParFilter>;
  using AggProcStyle = simd_helper::simd_by_par_t<AggregateColumnT, SimdParAggregate>;
  static_assert(TSL_BACKEND_SUPPORTS_BY_TYPE(tsl::add<AggProcStyle>, typename AggProcStyle::imask_type, typename AggProcStyle::register_type, typename AggProcStyle::register_type), "Backend does not support by type add");

  std::cout << "\tFilter SIMD parallelism     : " << SimdParFilter << "(uses: " << tsl::type_name<FilterProcStyle>() << ")" << std::endl;
  std::cout << "\tAggregation SIMD parallelism: " << SimdParAggregate << "(uses: " << tsl::type_name<AggProcStyle>() << ")" << std::endl;
  

  /* initializing */
  auto result = tsl::set_zero<AggProcStyle>();
  auto filter_val = tsl::set1<FilterProcStyle>(filter_value);
  
  for (auto i = 0; i < to_agg.size(); i += SimdParAggregate) {
    /* load aggregation data */
    auto const aggregate = tsl::loadu<AggProcStyle>(&to_agg[i]);
    typename AggProcStyle::imask_type valid = 0;

    for (auto j = 0; j < (SimdParAggregate / SimdParFilter); ++j) {
      /* load filter data */
      auto const filter_data = tsl::loadu<FilterProcStyle>(&to_filter[i + j * SimdParFilter]);

      /* compare filter data */
      auto const part_valid = tsl::to_integral<FilterProcStyle>(tsl::greater_than_or_equal<FilterProcStyle>(filter_data, filter_val));
      
      /* merge partial results */
      valid |= (part_valid << (j * SimdParFilter));
    }

    /* add valid values to result */
    result = tsl::add<AggProcStyle>(valid, result, aggregate);
  }
  /* sum up all partial results */
  return tsl::hadd<AggProcStyle>(result);
  
}

template <typename FilterColumnT, typename AggregateColumnT, unsigned SimdPar = simd_helper::max_par<FilterColumnT>()>
auto filter_aggregate_simd(std::span<const AggregateColumnT> to_agg, std::span<const FilterColumnT> to_filter, FilterColumnT const filter_value) -> AggregateColumnT {

  static_assert(std::is_arithmetic_v<FilterColumnT>, "FilterColumnT must be an arithmetic type");
  static_assert(std::is_arithmetic_v<AggregateColumnT>, "AggregateColumnT must be an arithmetic type");
  static_assert(SimdPar > 0, "SimdPar must be greater than 0");
  static_assert(sizeof(FilterColumnT) >= sizeof(AggregateColumnT), "FilterColumnT must be larger or equal in size to AggregateColumnT");

  if (to_agg.size() != to_filter.size()) {
    throw std::runtime_error("Size mismatch");
  }
  /* calculate the number of elements that can be processed in parallel */
  auto to_filter_simd_count = (to_filter.size() / SimdPar) * SimdPar;
  auto to_agg_simd_count = (to_agg.size() / SimdPar) * SimdPar;

  /* create std::spans for simd and scalar processing */
  auto to_filter_simd = to_filter.subspan(0, to_filter_simd_count);
  auto to_agg_simd = to_agg.subspan(0, to_agg_simd_count);

  auto to_filter_scalar = to_filter.subspan(to_filter_simd_count);
  auto to_agg_scalar = to_agg.subspan(to_agg_simd_count);

  AggregateColumnT result = 0;
  /* execute filter aggregation */
  if constexpr (parallelism_available_for_all<SimdPar, FilterColumnT, AggregateColumnT>()) {
    std::cout << "Executing filter_aggregate_impl on [0:" << to_filter_simd_count <<")" << std::endl;
    result = filter_aggregate_simd_impl<SimdPar, FilterColumnT, AggregateColumnT>(to_agg_simd, to_filter_simd, filter_value);
  } else {
    std::cout << "Executing filter_aggregate_differing_parallelism_impl on [0:" << to_filter_simd_count <<")" << std::endl;
    result = filter_aggregate_simd_differing_parallelism_impl<SimdPar, FilterColumnT, simd_helper::max_par<AggregateColumnT>(), AggregateColumnT>(to_agg_simd, to_filter_simd, filter_value);
  }
  std::cout << "Executing filter_aggregate_impl on [" << to_filter_simd_count << ":" << to_filter.size() <<")" << std::endl;
  result += filter_aggregate_simd_impl<1, FilterColumnT, AggregateColumnT>(to_agg_scalar, to_filter_scalar, filter_value);
  return result;
}