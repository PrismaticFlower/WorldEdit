#pragma once

#include <cstddef>
#include <shared_mutex>
#include <vector>

namespace we {

/// @brief A thread safe temporary buffer allocator.
struct temp_buffer_allocator {
   constexpr static std::size_t alignment = 64;

   /// @brief Construct a temp_buffer_allocator, no memory is allocated however until the first allocate call.
   /// @param start_page_size The starting size for allocator pages.
   explicit temp_buffer_allocator(const std::size_t start_page_size) noexcept;

   /// @brief Allocate space for an array of T.
   /// @tparam T The type to allocate space for.
   /// @param object_count The size of the array.
   /// @return A pointer to the allocated bytes. Valid until reset is called.
   template<typename T>
   [[nodiscard]] auto allocate(const std::size_t object_count) noexcept -> std::byte*
   {
      static_assert(alignof(T) <= alignment, "T's alignment is too large.");
      static_assert(std::is_trivially_destructible_v<T>,
                    "T must be trivially destructible.");

      return allocate_bytes(object_count * sizeof(T));
   }

   /// @brief Allocate bytes.
   /// @param count The number of bytes to allocate.
   /// @return A pointer to the allocated bytes. Valid until reset is called.
   [[nodiscard]] auto allocate_bytes(const std::size_t count) noexcept -> std::byte*;

   /// @brief Reset the allocator. This invalidate all allocations.
   void reset() noexcept;

private:
   struct page {
      explicit page(std::size_t size) noexcept;

      ~page();

      page(page&& other) noexcept;
      auto operator=(page&& other) noexcept -> page&;

      page(const page&) = delete;
      auto operator=(const page&) -> page& = delete;

      void swap(page& other);

      std::size_t allocated = 0;
      std::byte* base_address = nullptr;
   };

   std::shared_mutex _mutex;
   page* _page = nullptr;

   std::size_t _page_size = 0;
   std::ptrdiff_t _page_index = -1;

   std::vector<page> _pages;
   std::vector<std::vector<page>> _deferred_release_pages;
};

}
