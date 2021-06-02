#pragma once

#include "utility/make_range.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <compare>
#include <cstddef>
#include <initializer_list>
#include <limits>
#include <memory>
#include <new>
#include <span>
#include <stdexcept>
#include <type_traits>

namespace we::container {

template<typename Type>
class row_iterator {
public:
   using difference_type = std::ptrdiff_t;
   using value_type = std::span<Type>;
   using reference = std::span<Type>;
   using iterator_category = std::random_access_iterator_tag;

   row_iterator() = default;

   row_iterator(std::span<Type> data, std::ptrdiff_t width, std::ptrdiff_t height) noexcept
   {
      _data = data.data();
      _width = width;
      _height = height;
   }

   auto operator++() noexcept -> row_iterator&
   {
      ++_y;

      bounds_check();

      return *this;
   }

   auto operator++(int) const noexcept -> row_iterator
   {
      auto other = *this;

      ++other;

      return other;
   }

   auto operator--() noexcept -> row_iterator&
   {
      --_y;

      bounds_check();

      return *this;
   }

   auto operator--(int) const noexcept -> row_iterator
   {
      auto other = *this;

      --other;

      return other;
   }

   auto operator+=(const std::ptrdiff_t offset) noexcept -> row_iterator&
   {
      _y += offset;

      bounds_check();

      return *this;
   }

   auto operator+(const std::ptrdiff_t offset) const noexcept -> row_iterator
   {
      auto other = *this;

      other += offset;

      return other;
   }

   auto operator-=(const std::ptrdiff_t offset) noexcept -> row_iterator&
   {
      _y -= offset;

      bounds_check();

      return *this;
   }

   auto operator-(const std::ptrdiff_t offset) const noexcept -> row_iterator
   {
      auto other = *this;

      other -= offset;

      return other;
   }

   auto operator*() const noexcept -> value_type
   {
      assert(_y >= 0 and _y < _height);

      return value_type{_data + (_y * _width), static_cast<std::size_t>(_width)};
   }

   auto operator[](const std::ptrdiff_t offset) const noexcept -> value_type
   {
      auto other = *this;

      other += offset;

      return *other;
   }

   auto operator<=>(const row_iterator&) const noexcept = default;

private:
   void bounds_check() const noexcept
   {
      assert(_y >= 0 and _y <= _height);
   }

   Type* _data = nullptr;
   std::ptrdiff_t _width = -1;
   std::ptrdiff_t _height = -1;
   std::ptrdiff_t _y = 0;
};

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

   template<typename Int, typename = std::enable_if_t<std::is_integral_v<Int>>>
   dynamic_array_2d(Int width, Int height) noexcept
   {
      _width = static_cast<std::ptrdiff_t>(width);
      _height = static_cast<std::ptrdiff_t>(height);
      _memory = std::make_unique<Type[]>(_width * _height);
   }

   template<std::size_t width>
   dynamic_array_2d(std::initializer_list<std::array<Type, width>> initializer) noexcept
      : dynamic_array_2d{width, initializer.size()}
   {
      const auto src_rows = initializer.begin();
      const auto dest_rows = rows_begin();

      for (std::ptrdiff_t y = 0; y < _height; ++y) {
         std::copy_n(src_rows[y].cbegin(), _width, dest_rows[y].begin());
      }
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
      this->_width = other._width;
      this->_height = other._height;
      this->_memory = std::make_unique<Type[]>(_width * _height);

      std::copy(other.cbegin(), other.cend(), this->begin());

      return *this;
   }

   auto operator=(dynamic_array_2d&& other) noexcept -> dynamic_array_2d&
   {
      this->_width = std::exchange(other._width, 0);
      this->_height = std::exchange(other._height, 0);
      this->_memory = std::move(other._memory);

      return *this;
   }

   ~dynamic_array_2d() = default;

   auto begin() noexcept -> iterator
   {
      return begin_impl(*this);
   }

   auto end() noexcept -> iterator
   {
      return end_impl(*this);
   }

   auto begin() const noexcept -> const_iterator
   {
      return begin_impl(*this);
   }

   auto end() const noexcept -> const_iterator
   {
      return end_impl(*this);
   }

   auto cbegin() const noexcept -> const_iterator
   {
      return begin_impl(*this);
   }

   auto cend() const noexcept -> const_iterator
   {
      return end_impl(*this);
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
      return static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max());
   }

   bool empty() const noexcept
   {
      return _memory == nullptr;
   }

   auto data() noexcept -> Type*
   {
      return _memory.get();
   }

   auto data() const noexcept -> const Type*
   {
      return _memory.get();
   }

   auto shape() const noexcept -> std::array<std::size_t, 2>
   {
      return {static_cast<std::size_t>(_width), static_cast<std::size_t>(_height)};
   }

   auto sshape() const noexcept -> std::array<std::ptrdiff_t, 2>
   {
      return {_width, _height};
   }

   auto operator[](const index index) noexcept -> reference
   {
      return at_impl<std::nothrow_t>(*this, index.x, index.y);
   }

   auto operator[](const index index) const noexcept -> const_reference
   {
      return at_impl<std::nothrow_t>(*this, index.x, index.y);
   }

   auto at(const index index) -> reference
   {
      return at_impl(*this, index.x, index.y);
   }

   auto at(const index index) const -> const_reference
   {
      return at_impl(*this, index.x, index.y);
   }

   auto rows_begin() noexcept -> row_iterator<Type>
   {
      return rows_begin_impl(*this);
   }

   auto rows_end() noexcept -> row_iterator<Type>
   {
      return rows_begin_impl(*this) + _height;
   }

   auto rows_begin() const noexcept -> row_iterator<const Type>
   {
      return rows_begin_impl(*this);
   }

   auto rows_end() const noexcept -> row_iterator<const Type>
   {
      return rows_begin_impl(*this) + _height;
   }

   auto rows_cbegin() const noexcept -> row_iterator<const Type>
   {
      return rows_begin_impl(*this);
   }

   auto rows_cend() const noexcept -> row_iterator<const Type>
   {
      return rows_begin_impl(*this) + _height;
   }

   auto rows() noexcept
   {
      return utility::make_range(rows_begin(), rows_end());
   }

   auto rows() const noexcept
   {
      return utility::make_range(rows_cbegin(), rows_cend());
   }

   template<typename Type>
   friend bool operator==(const dynamic_array_2d<Type>& l,
                          const dynamic_array_2d<Type>& r) noexcept;

private:
   template<typename Throwing = void, typename Self>
   static auto at_impl(Self& self, const std::ptrdiff_t x,
                       const std::ptrdiff_t y) noexcept(std::is_same_v<Throwing, std::nothrow_t>)
      -> std::conditional_t<std::is_const_v<Self>, const_reference, reference>
   {
      if (x >= self._width or y >= self._height) {
         if constexpr (not std::is_same_v<Throwing, std::nothrow_t>) {
            throw std::out_of_range{"index for 2d array out of range"};
         }
      }

      return self._memory[(y * self._width) + x];
   }

   template<typename Self>
   static auto begin_impl(Self& self) noexcept
      -> std::conditional_t<std::is_const_v<Self>, const_iterator, iterator>
   {
      return self._memory.get();
   }

   template<typename Self>
   static auto end_impl(Self& self) noexcept
      -> std::conditional_t<std::is_const_v<Self>, const_iterator, iterator>
   {
      return self._memory.get() + (self._width * self._height);
   }

   template<typename Self>
   static auto rows_begin_impl(Self& self) noexcept
      -> std::conditional_t<std::is_const_v<Self>, row_iterator<const Type>, row_iterator<Type>>
   {
      return row_iterator{std::span(self.data(), self.size()), self._width, self._height};
   }

   std::ptrdiff_t _width = 0;
   std::ptrdiff_t _height = 0;

   std::unique_ptr<Type[]> _memory;
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
   if (l.sshape() != r.sshape()) return false;

   return std::equal(l.cbegin(), l.cend(), r.cbegin());
}
}
