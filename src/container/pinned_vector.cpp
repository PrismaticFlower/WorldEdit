#include "pinned_vector.hpp"

#include <algorithm>
#include <exception>
#include <stdexcept>

#include <Windows.h>

#include <fmt/core.h>

namespace we::container::detail {

namespace {

struct system_page_info {
   DWORD size = 0;
   DWORD allocation_granularity = 0;
};

auto get_system_page_info() noexcept -> system_page_info
{
   SYSTEM_INFO system_info{};

   GetSystemInfo(&system_info);

   return {.size = system_info.dwPageSize,
           .allocation_granularity = system_info.dwAllocationGranularity};
}

}

[[noreturn]] void throw_std_out_of_range(std::size_t i, std::size_t size)
{
   throw std::out_of_range{
      fmt::format("Index '{}' is out of range. Range is [0, {}).", i, size)};
}

[[noreturn]] void terminate_out_of_reserved_memory()
{
   std::terminate(); // Out of reserved virtual memory, pinned_vector can not grow anymore.
}

auto virtual_reserve(const std::size_t count, const std::size_t item_size,
                     std::size_t& allocated_count) noexcept -> void*
{
   const system_page_info page_info = get_system_page_info();

   const std::size_t needed_bytes = count * item_size;
   const std::size_t aligned_needed_bytes =
      ((needed_bytes + (page_info.size - 1)) / page_info.size) * page_info.size;

   const std::size_t overflow_padded_bytes = aligned_needed_bytes + page_info.size;

   const std::size_t allocation_needed_bytes =
      ((overflow_padded_bytes + (page_info.allocation_granularity - 1)) /
       page_info.allocation_granularity) *
      page_info.allocation_granularity;

   allocated_count = (allocation_needed_bytes - page_info.size) / item_size;

   void* memory =
      VirtualAlloc(nullptr, allocation_needed_bytes, MEM_RESERVE, PAGE_READWRITE);

   if (not memory) std::terminate();

   return memory;
}

auto virtual_commit(void* begin, const std::size_t count,
                    const std::size_t item_size) noexcept -> std::size_t
{
   const system_page_info page_info = get_system_page_info();

   const std::size_t committed_count = std::max(page_info.size / item_size, count);

   if (not VirtualAlloc(begin, committed_count * item_size, MEM_COMMIT, PAGE_READWRITE)) {
      std::terminate();
   }

   return committed_count;
}

void virtual_free(void* memory) noexcept
{
   if (not VirtualFree(memory, 0, MEM_RELEASE)) std::terminate();
}

}