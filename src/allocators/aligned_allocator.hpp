#pragma once

#include <cstddef>
#include <new>
#include <type_traits>

namespace we {

template<typename T, typename A>
struct basic_aligned_allocator {
   using value_type = T;
   using is_always_equal = std::true_type;

   constexpr static std::size_t alignment = A::value;

   constexpr basic_aligned_allocator() = default;

   template<typename U, typename UA>
   constexpr basic_aligned_allocator(const basic_aligned_allocator<U, UA>&) noexcept
   {
      static_assert(A::value == UA::value);
   }

   [[nodiscard]] constexpr auto allocate(const std::size_t count) const noexcept -> T*
   {
      return static_cast<T*>(
         ::operator new(sizeof(T) * count, static_cast<std::align_val_t>(alignment)));
   }

   constexpr void deallocate(T* const memory, const std::size_t count) const noexcept
   {
      ::operator delete(memory, count * sizeof(T),
                        static_cast<std::align_val_t>(alignment));
   }

   [[nodiscard]] constexpr bool operator==(const basic_aligned_allocator&) const noexcept
   {
      return true;
   }

   template<typename U, typename UA>
   [[nodiscard]] constexpr bool operator==(const basic_aligned_allocator<U, UA>&) const noexcept
   {
      static_assert(A::value == UA::value);

      return true;
   }
};

template<typename T, std::size_t alignment>
using aligned_allocator =
   basic_aligned_allocator<T, std::integral_constant<std::size_t, alignment>>;

}
