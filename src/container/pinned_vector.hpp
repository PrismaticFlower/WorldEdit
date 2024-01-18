#pragma once

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace we::container {

namespace detail {

[[noreturn]] void throw_std_out_of_range(std::size_t i, std::size_t size);

[[noreturn]] void terminate_out_of_reserved_memory();

[[nodiscard]] auto virtual_reserve(const std::size_t count, const std::size_t item_size,
                                   std::size_t& allocated_count) noexcept -> void*;

[[nodiscard]] auto virtual_commit(void* begin, const std::size_t count,
                                  const std::size_t item_size) noexcept -> std::size_t;

[[nodiscard]] void virtual_free(void* memory) noexcept;

}

/// @brief Initialization parameters for a pinned_vector.
struct pinned_vector_init {
   /// @brief The max number of items the vector will be able to hold.
   std::size_t max_size = 0;

   /// @brief The initial capacity of the vector.
   std::size_t initial_capacity = 0;
};

template<typename T>
struct pinned_vector {
   using value_type = T;
   using pointer = T*;
   using const_pointer = const T*;
   using reference = value_type&;
   using const_reference = const value_type&;
   using size_type = std::size_t;
   using difference_type = std::ptrdiff_t;
   using iterator = T*;
   using const_iterator = const T*;

   pinned_vector(const pinned_vector_init& init) noexcept
   {
      if (init.max_size == 0) return;

      std::size_t allocated_count = 0;
      void* memory =
         detail::virtual_reserve(init.max_size, sizeof(T), allocated_count);

      _begin = static_cast<T*>(memory);
      _end = _begin;
      _commited_end = _begin;
      _reserved_end = _begin + allocated_count;

      if (init.initial_capacity > 0) {
         const std::size_t committed_count =
            detail::virtual_commit(_begin, init.initial_capacity, sizeof(T));

         _commited_end = _begin + committed_count;
      }
   }

   template<typename Range>
   pinned_vector(const pinned_vector_init& init, const Range& range) noexcept
      : pinned_vector{pinned_vector_init{.max_size = init.max_size,
                                         .initial_capacity =
                                            init.initial_capacity > range.size()
                                               ? init.initial_capacity
                                               : range.size()}}
   {
      append_range(range);
   }

   pinned_vector(const pinned_vector& other) noexcept
      : pinned_vector{pinned_vector_init{.max_size = other.max_size(),
                                         .initial_capacity = other.size()}}
   {
      append_range(other);
   }

   pinned_vector(pinned_vector&& other) noexcept
   {
      this->swap(other);
   }

   ~pinned_vector()
   {
      if (not _begin) return;

      clear();

      detail::virtual_free(_begin);
   }

   auto operator=(const pinned_vector& other) noexcept -> pinned_vector&
   {
      pinned_vector discard{other};

      this->swap(discard);

      return *this;
   }

   auto operator=(pinned_vector&& other) noexcept -> pinned_vector&
   {
      pinned_vector discard{pinned_vector_init{.max_size = 0}};

      discard.swap(other);
      this->swap(discard);

      return *this;
   }

   auto operator=(std::initializer_list<T> initializer_list) noexcept -> pinned_vector&
   {
      assign_range(initializer_list);

      return *this;
   }

   auto operator=(const pinned_vector_init& init) noexcept -> pinned_vector&
   {
      pinned_vector discard{init};

      this->swap(discard);

      return *this;
   }

   void assign(size_type count, const T& value) noexcept
   {
      clear();

      insert(end(), count, value);
   }

   void assign(std::initializer_list<T> initializer_list) noexcept
   {
      assign_range(initializer_list);
   }

   [[nodiscard]] auto begin() noexcept -> iterator
   {
      return _begin;
   }

   [[nodiscard]] auto begin() const noexcept -> const_iterator
   {
      return _begin;
   }

   [[nodiscard]] auto end() noexcept -> iterator
   {
      return _end;
   }

   [[nodiscard]] auto end() const noexcept -> const_iterator
   {
      return _end;
   }

   [[nodiscard]] auto cbegin() const noexcept -> const_iterator
   {
      return _begin;
   }

   [[nodiscard]] auto cend() const noexcept -> const_iterator
   {
      return _end;
   }

   [[nodiscard]] bool empty() const noexcept
   {
      return _begin == _end;
   }

   [[nodiscard]] auto size() const noexcept -> size_type
   {
      return static_cast<size_type>(_end - _begin);
   }

   [[nodiscard]] auto max_size() const noexcept -> size_type
   {
      return static_cast<size_type>(_reserved_end - _begin);
   }

   [[nodiscard]] auto capacity() const noexcept -> size_type
   {
      return static_cast<size_type>(_commited_end - _begin);
   }

   void resize(size_type size) noexcept(std::is_nothrow_default_constructible_v<T>)
   {
      const std::size_t current_size = static_cast<std::size_t>(_end - _begin);

      if (size > current_size) {
         ensure_space(size - current_size);

         T* new_end = _begin + size;

         for (T* ptr = _end; ptr != new_end; ++ptr) {
            if constexpr (std::is_nothrow_default_constructible_v<T>) {
               new (ptr) T();
            }
            else {
               try {
                  new (ptr) T();
               }
               catch (...) {
                  for (T* undo_ptr = _end; undo_ptr != ptr; ++undo_ptr) {
                     undo_ptr->~T();
                  }

                  throw;
               }
            }
         }

         _end = new_end;
      }
      else if (size < current_size) {
         for (T* ptr = _begin + size; ptr != _end; ++ptr) ptr->~T();

         _end = _begin + size;
      }
   }

   void resize(size_type size,
               const T& fill_value) noexcept(std::is_nothrow_copy_constructible_v<T>)
   {
      const std::size_t current_size = static_cast<std::size_t>(_end - _begin);

      if (size > current_size) {
         ensure_space(size - current_size);

         T* new_end = _begin + size;

         for (T* ptr = _end; ptr != new_end; ++ptr) {
            if constexpr (std::is_nothrow_copy_constructible_v<T>) {
               new (ptr) T(fill_value);
            }
            else {
               try {
                  new (ptr) T(fill_value);
               }
               catch (...) {
                  for (T* undo_ptr = _end; undo_ptr != ptr; ++undo_ptr) {
                     undo_ptr->~T();
                  }

                  throw;
               }
            }
         }

         _end = new_end;
      }
      else if (size < current_size) {
         for (T* ptr = _begin + size; ptr != _end; ++ptr) ptr->~T();

         _end = _begin + size;
      }
   }

   void reserve(size_type size) noexcept
   {
      const std::size_t current_reservation =
         static_cast<std::size_t>(_commited_end - _begin);

      if (current_reservation >= size) return;

      const std::size_t committed_count =
         detail::virtual_commit(_commited_end, size - current_reservation, sizeof(T));

      _commited_end += committed_count;
   }

   void shrink_to_fit() noexcept
   {
      return;
   }

   [[nodiscard]] auto operator[](size_type index) noexcept -> T&
   {
      assert(_begin + index < _end);

      return _begin[index];
   }

   [[nodiscard]] auto operator[](size_type index) const noexcept -> const T&
   {
      assert(_begin + index < _end);

      return _begin[index];
   }

   [[nodiscard]] auto at(size_type index) const -> const T&
   {
      if (_begin + index >= _end) detail::throw_std_out_of_range(index, size());

      return _begin[index];
   }

   [[nodiscard]] auto at(size_type index) -> T&
   {
      if (_begin + index >= _end) detail::throw_std_out_of_range(index, size());

      return _begin[index];
   }

   [[nodiscard]] auto front() -> T&
   {
      assert(_begin != _end);

      return *_begin;
   }

   [[nodiscard]] auto front() const -> T&
   {
      assert(_begin != _end);

      return *_begin;
   }

   [[nodiscard]] auto back() -> T&
   {
      assert(_begin != _end);

      return *(_end - 1);
   }

   [[nodiscard]] auto back() const -> T&
   {
      assert(_begin != _end);

      return *(_end - 1);
   }

   [[nodiscard]] auto data() noexcept -> T*
   {
      return _begin;
   }

   [[nodiscard]] auto data() const noexcept -> const T*
   {
      return _begin;
   }

   template<typename... Args>
   auto emplace_back(Args&&... args) -> T&
   {
      ensure_space(1);

      T* new_item = _end;

      new (new_item) T{std::forward<Args>(args)...};

      ++_end;

      return *new_item;
   }

   void push_back(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
   {
      ensure_space(1);

      T* new_item = _end;

      new (new_item) T{value};

      ++_end;
   }

   void push_back(T&& value) noexcept
   {
      ensure_space(1);

      T* new_item = _end;

      new (new_item) T{std::move(value)};

      ++_end;
   }

   void pop_back() noexcept
   {
      assert(_begin != _end);

      (_end - 1)->~T();

      --_end;
   }

   template<typename... Args>
   auto emplace(const_iterator position,
                Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
      -> iterator
   {
      T* item = _begin + (position - _begin);

      insert_shift_forward(item, 1);

      if constexpr (std::is_nothrow_constructible_v<T, Args...>) {
         new (item) T{std::forward<Args>(args)...};
      }
      else {
         try {
            new (item) T{std::forward<Args>(args)...};
         }
         catch (...) {
            insert_shift_back(item + 1, 1);

            throw;
         }
      }

      _end += 1;

      return item;
   }

   auto insert(const_iterator position,
               const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) -> iterator
   {
      return emplace(position, value);
   }

   auto insert(const_iterator position, T&& value) noexcept -> iterator
   {
      return emplace(position, std::move(value));
   }

   auto insert(const_iterator position, size_type count,
               const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) -> iterator
   {
      T* const inserted_begin = _begin + (position - _begin);
      T* const inserted_end = inserted_begin + count;

      insert_shift_forward(inserted_begin, count);

      for (T* item = inserted_begin; item != inserted_end; ++item) {
         if constexpr (std::is_nothrow_copy_constructible_v<T>) {
            new (item) T{value};
         }
         else {
            try {
               new (item) T{value};
            }
            catch (...) {
               for (T* destroy_item = inserted_begin; destroy_item != item;
                    ++destroy_item) {
                  destroy_item->~T();
               }

               insert_shift_back(inserted_end, count);

               throw;
            }
         }
      }

      _end += count;

      return inserted_begin;
   }

   auto insert(const_iterator position, std::initializer_list<T> initializer_list) noexcept(
      std::is_nothrow_copy_constructible_v<T>) -> iterator
   {
      return insert_range(position, initializer_list);
   }

   auto erase(const_iterator position) noexcept -> iterator
   {
      return erase(position, position + 1);
   }

   auto erase(const_iterator first, const_iterator last) noexcept -> iterator
   {
      assert(first <= last);
      assert(first >= _begin and last <= _end);

      T* const erase_begin = _begin + (first - _begin);
      T* const erase_end = _begin + (last - _begin);

      if (first == last) return erase_end;

      for (T* item = erase_begin; item != erase_end; ++item) {
         item->~T();
      }

      const std::ptrdiff_t erase_count = erase_end - erase_begin;

      for (T* item = erase_begin + erase_count; item != _end; ++item) {
         new (item - erase_count) T{std::move(*item)};

         item->~T();
      }

      _end -= (erase_end - erase_begin);

      return erase_begin;
   }

   template<typename Range>
   void assign_range(const Range& range) noexcept(
      std::is_nothrow_constructible_v<T, typename Range::value_type>)
   {
      clear();
      ensure_space(range.size());

      T* item = _begin;

      for (auto& v : range) {
         if constexpr (std::is_nothrow_constructible_v<T, typename Range::value_type>) {
            new (item) T{v};
         }
         else {
            try {
               new (item) T{v};
            }
            catch (...) {
               for (T* destroy_item = _begin; destroy_item != item; ++destroy_item) {
                  destroy_item->~T();
               }

               throw;
            }
         }

         item += 1;
      }

      _end = item;
   }

   template<typename Range>
   void append_range(const Range& range) noexcept(
      std::is_nothrow_constructible_v<T, typename Range::value_type>)
   {
      ensure_space(range.size());

      T* item = _end;

      for (auto& v : range) {
         if constexpr (std::is_nothrow_constructible_v<T, typename Range::value_type>) {
            new (item) T{v};
         }
         else {
            try {
               new (item) T{v};
            }
            catch (...) {
               for (T* destroy_item = _end; destroy_item != item; ++destroy_item) {
                  destroy_item->~T();
               }

               throw;
            }
         }

         item += 1;
      }

      _end = item;
   }

   template<typename Range>
   [[nodiscard]] auto insert_range(const_iterator position, const Range& range) noexcept(
      std::is_nothrow_constructible_v<T, typename Range::value_type>) -> iterator
   {
      const std::ptrdiff_t count = static_cast<std::ptrdiff_t>(range.size());

      T* inserted_begin = _begin + (position - _begin);
      T* inserted_end = inserted_begin + count;

      insert_shift_forward(inserted_begin, count);

      T* item = inserted_begin;

      for (auto& v : range) {
         if constexpr (std::is_nothrow_constructible_v<T, typename Range::value_type>) {
            new (item) T{v};
         }
         else {
            try {
               new (item) T{v};
            }
            catch (...) {
               for (T* destroy_item = inserted_begin; destroy_item != item;
                    ++destroy_item) {
                  destroy_item->~T();
               }

               insert_shift_back(inserted_end, count);

               throw;
            }
         }

         item += 1;
      }

      _end += count;

      return inserted_begin;
   }

   void swap(pinned_vector& other) noexcept
   {
      T* const temp_begin = other._begin;
      T* const temp_end = other._end;
      T* const temp_commited_end = other._commited_end;
      T* const temp_reserved_end = other._reserved_end;

      other._begin = this->_begin;
      other._end = this->_end;
      other._commited_end = this->_commited_end;
      other._reserved_end = this->_reserved_end;

      this->_begin = temp_begin;
      this->_end = temp_end;
      this->_commited_end = temp_commited_end;
      this->_reserved_end = temp_reserved_end;
   }

   void clear() noexcept
   {
      for (T* ptr = _begin; ptr != _end; ++ptr) ptr->~T();

      _end = _begin;
   }

private:
   void ensure_space(std::size_t count) noexcept
   {
      assert(_end != _reserved_end);

      const std::size_t current_space = static_cast<std::size_t>(_commited_end - _end);

      if (current_space >= count) return;

      [[unlikely]] if (_commited_end == _reserved_end) {
         detail::terminate_out_of_reserved_memory();
      }

      const std::size_t committed_count =
         detail::virtual_commit(_commited_end, count, sizeof(T));

      _commited_end = _end + committed_count;
   }

   void insert_shift_forward(T* from, std::ptrdiff_t count) noexcept
   {
      if (count == 0) return;

      ensure_space(count);

      const std::ptrdiff_t items_to_shift = _end - from;

      for (std::ptrdiff_t i = items_to_shift - 1; i >= 0; --i) {
         new (from + i + count) T{std::move(from[i])};

         from[i].~T();
      }
   }

   void insert_shift_back(T* from, std::ptrdiff_t count) noexcept
   {
      if (count == 0) return;

      const std::ptrdiff_t items_to_shift = _end - from;

      for (std::ptrdiff_t i = 0; i < items_to_shift; ++i) {
         new (from - i - count) T{std::move(from[i])};

         from[i].~T();
      }
   }

   T* _begin = nullptr;
   T* _end = nullptr;
   T* _commited_end = nullptr;
   T* _reserved_end = nullptr;
};

}