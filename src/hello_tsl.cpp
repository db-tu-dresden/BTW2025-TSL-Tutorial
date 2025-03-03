#include <iostream>
#include <ranges>
#include <cstdint>
#include <tuple>
#include <iomanip>

#include "tsl/tsl.hpp"
#include "simd_utils.hpp"

template <typename... Args>
void print(Args... args) {
  std::cout << "[";
  ((std::cout << args << (&args != &std::get<sizeof...(args) - 1>(std::tie(args...)) ? ", " : "")), ...);
  std::cout << "]\n";
}

template <typename T>
void print_availabel_degree_of_parallelism() {
  std::cout << std::left << std::setw(10) << ( tsl::type_name<T>() + ": ");
  std::apply([](auto... args) { print(args...); }, available_parallelism<T>());
}


int main() {
  std::cout << "Maximum degree of parallelism for "
            << tsl::type_name<unsigned long long int>() << ": "
            << simd_helper::max_par<unsigned long long int>() << " "
            << "(" << tsl::type_name<simd_helper::target::max_width_extension_t>() << ")" << std::endl;
  std::cout << "Minimum degree of parallelism for "
            << tsl::type_name<unsigned long long int>() << ": "
            << simd_helper::min_par<unsigned long long int>() << " "
            << "(" << tsl::type_name<simd_helper::target::min_width_extension_t>() << ")" << std::endl;
            
  print_availabel_degree_of_parallelism<int8_t>();
  print_availabel_degree_of_parallelism<int16_t>();
  print_availabel_degree_of_parallelism<int32_t>();
  print_availabel_degree_of_parallelism<int64_t>();
}