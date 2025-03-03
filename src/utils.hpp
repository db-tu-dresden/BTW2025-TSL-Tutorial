#pragma once

struct min_size{};
struct max_size{};

template <typename... Ts>
struct min_max_type;

template <typename MinMax, typename T, typename... Ts>
struct min_max_type<MinMax, T, Ts...> {
  private:  
    using RestMinMax = typename min_max_type<MinMax, Ts...>::type;
  public:
    using type = std::conditional_t<
      std::is_same_v<MinMax, min_size>,
      std::conditional_t<(sizeof(T) < sizeof(RestMinMax)), T, RestMinMax>,
      std::conditional_t<(sizeof(T) > sizeof(RestMinMax)), T, RestMinMax>
    >;
};

template <typename MinMax, typename T>
struct min_max_type<MinMax, T> {
    using type = T;
};

template <typename MinMax, typename... Ts>
using min_max_t = typename min_max_type<MinMax, Ts...>::type;
