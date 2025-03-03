#pragma once

#include <cstddef>
#include <memory>
#include <span>
#include <bit>

template <typename T>
class column_t {
  public:
    using data_t = std::unique_ptr<T[]>;

  private:  
    data_t _data;
    size_t _element_count;

  public:
    explicit column_t(size_t element_count)
    : _data{std::make_unique<T[]>(element_count)},
      _element_count(element_count) {}
  
  public:
  [[nodiscard]] auto data() const -> T* {
    return _data.get();
  }

  [[nodiscard]] auto data() -> T* {
    return _data.get();
  }

  [[nodiscard]] auto size() const -> size_t {
    return _element_count;
  }

  [[nodiscard]] auto span() -> std::span<T> {
    return { _data.get(), _element_count };
  }

  [[nodiscard]] auto span() const -> std::span<const T> {
    return { _data.get(), _element_count };
  }

  [[nodiscard]] auto operator[](size_t index) -> T& {
    return _data[index];
  }

  [[nodiscard]] auto operator[](size_t index) const -> const T& {
    return _data[index];
  }

};