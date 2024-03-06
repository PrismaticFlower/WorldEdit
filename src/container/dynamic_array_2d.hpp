#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <stdexcept>

namespace we::container {

template<typename Type>
class dynamic_array_2d {
public:
   using value_type = Type;
   using reference = Type&;
   using const_reference = const Type&;
   using iterator = Type*;
   using const_iterator = const Type*;
   using difference_type = std::ptrdiff_t;
   using size_type = std::size_t;

   struct index {
      std::ptrdiff_t x = 0;
      std::ptrdiff_t y = 0;
   };

   dynamic_array_2d() = default;

   dynamic_array_2d(std::integral auto width, std::integral auto height) noexcept
   {
      _width = static_cast<std::ptrdiff_t>(width);
      _height = static_cast<std::ptrdiff_t>(height);
      _memory = new Type[_width * _height]();
   }

   dynamic_array_2d(const dynamic_array_2d& other) noexcept
   {
      *this = other;
   }

   dynamic_array_2d(dynamic_array_2d&& other) noexcept
   {
      *this = std::move(other);
   }

   auto operator=(const dynamic_array_2d& other) noexcept -> dynamic_array_2d&
   {
      if (this->_memory) delete[] this->_memory;

      this->_width = other._width;
      this->_height = other._height;
      this->_memory = new Type[_width * _height];

      for (std::ptrdiff_t y = 0; y < _height; ++y) {
         for (std::ptrdiff_t x = 0; x < _width; ++x) {
            (*this)[{x, y}] = other[{x, y}];
         }
      }

      return *this;
   }

   auto operator=(dynamic_array_2d&& other) noexcept -> dynamic_array_2d&
   {
      dynamic_array_2d discard;

      discard.swap(*this);
      this->swap(other);

      return *this;
   }

   ~dynamic_array_2d()
   {
      if (_memory) delete[] _memory;
   }

   auto begin() noexcept -> iterator
   {
      return _memory;
   }

   auto end() noexcept -> iterator
   {
      return _memory + (_width * _height);
   }

   auto begin() const noexcept -> const_iterator
   {
      return _memory;
   }

   auto end() const noexcept -> const_iterator
   {
      return _memory + (_width * _height);
   }

   auto cbegin() const noexcept -> const_iterator
   {
      return begin();
   }

   auto cend() const noexcept -> const_iterator
   {
      return end();
   }

   void swap(dynamic_array_2d& other) noexcept
   {
      using std::swap;

      swap(this->_width, other._width);
      swap(this->_height, other._height);
      swap(this->_memory, other._memory);
   }

   auto size() const noexcept -> std::size_t
   {
      return static_cast<std::size_t>(_width * _height);
   }

   auto ssize() const noexcept -> std::ptrdiff_t
   {
      return _width * _height;
   }

   auto max_size() const noexcept -> std::size_t
   {
      return static_cast<std::size_t>(PTRDIFF_MAX);
   }

   bool empty() const noexcept
   {
      return _memory == nullptr;
   }

   auto data() noexcept -> Type*
   {
      return _memory;
   }

   auto data() const noexcept -> const Type*
   {
      return _memory;
   }

   auto width() const noexcept -> std::size_t
   {
      return _width;
   }

   auto height() const noexcept -> std::size_t
   {
      return _height;
   }

   auto s_width() const noexcept -> std::ptrdiff_t
   {
      return _width;
   }

   auto s_height() const noexcept -> std::ptrdiff_t
   {
      return _height;
   }

   auto operator[](const index index) noexcept -> reference
   {
      assert(index.x < _width and index.y < _height);

      return _memory[(index.y * _width) + index.x];
   }

   auto operator[](const index index) const noexcept -> const_reference
   {
      assert(index.x < _width and index.y < _height);

      return _memory[(index.y * _width) + index.x];
   }

   auto at(const index index) -> reference
   {
      if (index.x >= _width or index.y >= _height) {
         throw std::out_of_range{"index for 2d array out of range"};
      }

      return _memory[(index.y * _width) + index.x];
   }

   auto at(const index index) const -> const_reference
   {
      if (index.x >= _width or index.y >= _height) {
         throw std::out_of_range{"index for 2d array out of range"};
      }

      return _memory[(index.y * _width) + index.x];
   }

   template<typename Type>
   friend bool operator==(const dynamic_array_2d<Type>& l,
                          const dynamic_array_2d<Type>& r) noexcept;

private:
   std::ptrdiff_t _width = 0;
   std::ptrdiff_t _height = 0;

   Type* _memory = nullptr;
};

template<typename Type>
inline void swap(dynamic_array_2d<Type>& l, dynamic_array_2d<Type>& r) noexcept
{
   l.swap(r);
}

template<typename Type>
inline bool operator==(const dynamic_array_2d<Type>& l,
                       const dynamic_array_2d<Type>& r) noexcept
{
   if (l.width() != r.width() or l.height() != r.height()) return false;

   for (std::ptrdiff_t y = 0; y < l.s_height(); ++y) {
      for (std::ptrdiff_t x = 0; x < l.s_width(); ++x) {
         if (l[{x, y}] != r[{x, y}]) return false;
      }
   }

   return true;
}

}
