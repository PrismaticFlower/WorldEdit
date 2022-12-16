#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

namespace we {

/// @brief Provides storage for objects with hidden implementations but without the overhead of dynamic memory allocation. If you're getting strange compiler errors make sure your special member functions are setup correctly.
/// @tparam T The type to provide storage for. Only needs to be complete in implementation files
/// @tparam size The size of the type. Can be larger than the types true size.
/// @tparam alignment The alignment of the type. Can be larger than the types true alignment.
template<typename T, std::size_t size, std::size_t alignment = alignof(void*)>
struct implementation_storage {
   implementation_storage() noexcept(std::is_nothrow_constructible_v<T>)
   {
      static_assert(
         alignof(T) <= alignment,
         "implementation_storage alignment does not match alignment for T");
      static_assert(sizeof(T) <= size,
                    "implementation_storage size is too small for T");

      new (as_ptr()) T();
   }

   implementation_storage(const implementation_storage& other) noexcept(
      std::is_nothrow_copy_constructible_v<T>)
   {
      static_assert(
         alignof(T) <= alignment,
         "implementation_storage alignment does not match alignment for T");
      static_assert(sizeof(T) <= size,
                    "implementation_storage size is too small for T");

      new (as_ptr()) T(other.get());
   }

   implementation_storage(implementation_storage&& other) noexcept(
      std::is_nothrow_move_constructible_v<T>)
   {
      static_assert(
         alignof(T) <= alignment,
         "implementation_storage alignment does not match alignment for T");
      static_assert(sizeof(T) <= size,
                    "implementation_storage size is too small for T");

      new (as_ptr()) T(std::move(other.get()));
   }

   template<typename... Args>
      requires(std::is_constructible_v<T, Args...>)
   implementation_storage(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
   {
      static_assert(
         alignof(T) <= alignment,
         "implementation_storage alignment does not match alignment for T");
      static_assert(sizeof(T) <= size,
                    "implementation_storage size is too small for T");

      new (as_ptr()) T(std::forward<Args>(args)...);
   }

   ~implementation_storage()
   {
      static_assert(
         alignof(T) <= alignment,
         "implementation_storage alignment does not match alignment for T");
      static_assert(sizeof(T) <= size,
                    "implementation_storage size is too small for T");

      get().~T();
   }

   auto operator=(const implementation_storage& other) noexcept(
      std::is_nothrow_copy_assignable_v<T>) -> implementation_storage&
   {
      static_assert(
         alignof(T) <= alignment,
         "implementation_storage alignment does not match alignment for T");
      static_assert(sizeof(T) <= size,
                    "implementation_storage size is too small for T");

      [[maybe_unused]] T& self = (get() = other.get());

      return *this;
   }

   auto operator=(implementation_storage&& other) noexcept(
      std::is_nothrow_move_assignable_v<T>) -> implementation_storage&
   {
      static_assert(
         alignof(T) <= alignment,
         "implementation_storage alignment does not match alignment for T");
      static_assert(sizeof(T) <= size,
                    "implementation_storage size is too small for T");

      [[maybe_unused]] T& self = (get() = std::move(other.get()));

      return *this;
   }

   template<typename U>
      requires(std::is_assignable_v<T, U>)
   auto operator=(U&& other) noexcept(std::is_nothrow_assignable_v<T, U>)
      -> implementation_storage&
   {
      static_assert(
         alignof(T) <= alignment,
         "implementation_storage alignment does not match alignment for T");
      static_assert(sizeof(T) <= size,
                    "implementation_storage size is too small for T");

      [[maybe_unused]] auto& self = (get() = std::forward<U>(other));

      return *this;
   }

   auto get() noexcept -> T&
   {
      return *reinterpret_cast<T*>(&_storage);
   }

   auto get() const noexcept -> const T&
   {
      return *reinterpret_cast<const T*>(&_storage);
   }

   auto operator->() noexcept -> T*
   {
      return reinterpret_cast<T*>(&_storage);
   }

   auto operator->() const noexcept -> const T*
   {
      return reinterpret_cast<const T*>(&_storage);
   }

private:
   auto as_ptr() noexcept -> T*
   {
      return reinterpret_cast<T*>(&_storage);
   }

   alignas(alignment) std::byte _storage[size];
};

}
