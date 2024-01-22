#pragma once

#include <cstddef>

namespace we {

struct string {
   using value_type = char;
   using size_type = ::std::size_t;
   using difference_type = ::std::ptrdiff_t;
   using pointer = char*;
   using const_pointer = const char*;
   using reference = char&;
   using const_reference = const char&;
   using iterator = char*;
   using const_iterator = const char*;

   string() noexcept = default;

   string(const string& other) noexcept;

   string(string&& other) noexcept;

   string(const char* string_null_terminated) noexcept;
   string(const char* string, size_type size) noexcept;

   string(::std::nullptr_t) noexcept = delete;

   ~string();

   auto operator=(const string& other) noexcept -> string&;
   auto operator=(string&& other) noexcept -> string&;

   auto operator=(const char* string_null_terminated) noexcept -> string&;

   auto operator=(::std::nullptr_t) noexcept -> string& = delete;

   [[nodiscard]] auto begin() noexcept -> char*;
   [[nodiscard]] auto begin() const noexcept -> const char*;
   [[nodiscard]] auto end() noexcept -> char*;
   [[nodiscard]] auto end() const noexcept -> const char*;

   [[nodiscard]] auto cbegin() const noexcept -> const char*;
   [[nodiscard]] auto cend() const noexcept -> const char*;

   [[nodiscard]] auto size() const noexcept -> size_type;

   [[nodiscard]] auto max_size() const noexcept -> size_type;

   void resize(size_type new_size);

   [[nodiscard]] auto capacity() const noexcept -> size_type;

   void reserve(size_type new_min_capacity);

   void shrink_to_fit();

   void clear() noexcept;

   [[nodiscard]] bool empty() const noexcept;

   [[nodiscard]] auto operator[](size_type pos) const noexcept -> const char&;

   [[nodiscard]] auto operator[](size_type pos) noexcept -> char&;

   [[nodiscard]] auto at(size_type n) const -> const char&;

   [[nodiscard]] auto at(size_type n) -> char&;

   auto c_str() const noexcept -> const char*;

   auto data() const noexcept -> const char*;

   auto data() noexcept -> char*;

   void swap(string& other) noexcept;

private:
   char* _data = &_local_buffer[0];
   size_type _size = 0;

   union {
      char _local_buffer[16] = {'\0'};
      size_type _allocated_capacity;
   };

   bool local_buffer_active() const noexcept;
};

bool operator==(const string& left, const string& right) noexcept;

bool operator==(const string& left, const char* right) noexcept;

}
