#include <iostream>
#include <numeric>
#include <algorithm>
#include <stdfloat>

#include "column.hpp"

#ifdef LIVE_SESSION
#include "filter_aggregate_live.hpp"
#else
#include "filter_aggregate.hpp"
#endif


template <typename FilterColumnT, typename AggregateColumnT = FilterColumnT>
auto filter_aggregate_ref(std::span<const AggregateColumnT> to_agg, std::span<const FilterColumnT> to_filter, FilterColumnT const filter_value) -> AggregateColumnT {
  AggregateColumnT result = 0;
  for (auto i = 0; i < to_filter.size(); ++i) {
    if (to_filter[i] >= filter_value) {
      result += to_agg[i];
    }
  }
  return result;
}

int main(void) {
  using FilterColumnT = int32_t;
  using AggregateColumnT = int32_t;
  auto const N = 129;
  
  column_t<FilterColumnT>   to_filter(N);
  column_t<AggregateColumnT> to_aggregate(N);

  /* Initialize filter data with a sequence [0, N-1] */
  std::iota(to_filter.span().begin(), to_filter.span().end(), 0);
  /* Initialize aggregation data with a constant value 4 */
  std::fill(to_aggregate.span().begin(), to_aggregate.span().end(), 4);

  /* Accumulate all values from to_aggregate, if the corresponding element in to_filter is greater or equal to 50. */
  auto result = filter_aggregate_simd<FilterColumnT,AggregateColumnT>(to_aggregate.span(), to_filter.span(), (FilterColumnT)50);

  /* Run the reference implementation for sanity checking */
  AggregateColumnT reference_result = filter_aggregate_ref<FilterColumnT, AggregateColumnT>(to_aggregate.span(), to_filter.span(), (FilterColumnT) 50);
  
  std::cout << "Result: " << +result << ". (should be: " << +reference_result << ") => ";
  std::cout << ((result == reference_result) ? "Fine." : "FAIL") << std::endl;
  return 0;
}