#pragma once

#include <array>
#include <cassert>

namespace we::container {

template<typename T, std::size_t max_size>
struct ring_set {
   auto insert(const T& value) noexcept -> const T&
   {
      for (std::size_t i = 0; i < _size; ++i) {
         if (_items[i] == value) return _items[i];
      }

      if (_size != max_size) ++_size;

      const std::size_t index = (_next_index++) % max_size;

      _items[index] = value;

      return _items[index];
   }

   auto size() const noexcept -> std::size_t
   {
      return _size;
   }

   bool empty() const noexcept
   {
      return _size == 0;
   }

   auto operator[](const std::size_t i) const noexcept -> const T&
   {
      assert(i < _size);

      return _items[i];
   }

private:
   std::size_t _size = 0;
   std::size_t _next_index = 0;
   std::array<T, max_size> _items;
};

}