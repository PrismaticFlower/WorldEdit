#include "temp_buffer_allocator.hpp"

#include "math/align.hpp"

#include <atomic>
#include <bit>
#include <cassert>

namespace we {

temp_buffer_allocator::temp_buffer_allocator(const std::size_t start_page_size) noexcept
   : _page_size{start_page_size}
{
}

auto temp_buffer_allocator::allocate_bytes(const std::size_t count) noexcept
   -> std::byte*
{
   const std::size_t aligned_count = math::align_up(count, alignment);

retry_allocate:
   std::byte* exceeded_base_address = nullptr;

   {
      std::shared_lock lock{_mutex};

      if (_page) {
         const std::size_t allocation_offset =
            std::atomic_ref{_page->allocated}.fetch_add(aligned_count,
                                                        std::memory_order_relaxed);

         if (allocation_offset + aligned_count <= _page_size) {
            return _page->base_address + allocation_offset;
         }
         else {
            exceeded_base_address = _page->base_address;
         }
      }
   }

   {
      std::scoped_lock lock{_mutex};

      if (_page and _page->base_address != exceeded_base_address) {
         goto retry_allocate;
      }

      [[unlikely]] if (aligned_count > _page_size) {
         _page_size = std::bit_ceil(aligned_count);

         _deferred_release_pages.push_back(std::move(_pages));

         _pages.clear();
         _pages.emplace_back(_page_size);

         _page_index = 0;
      }
      else {
         _page_index += 1;

         if (_page_index >= std::ssize(_pages)) _pages.emplace_back(_page_size);
      }

      _page = &_pages[_page_index];
   }

   goto retry_allocate;
}

void temp_buffer_allocator::reset() noexcept
{
   std::scoped_lock lock{_mutex};

   if (not _pages.empty()) {
      _page_index = 0;
      _page = &_pages[0];
   }
   else {
      _page_index = -1;
   }

   for (page& page : _pages) page.allocated = 0;

   _deferred_release_pages.clear();
}

temp_buffer_allocator::page::page(std::size_t size) noexcept
{
   base_address = reinterpret_cast<std::byte*>(
      ::operator new(size, std::align_val_t{alignment}, std::nothrow));

   assert(base_address);
}

temp_buffer_allocator::page::~page()
{
   if (base_address) {
      ::operator delete[](base_address, std::align_val_t{alignment}, std::nothrow);
   }
}

temp_buffer_allocator::page::page(page&& other) noexcept
{
   this->swap(other);
}

auto temp_buffer_allocator::page::operator=(page&& other) noexcept -> page&
{
   page discard{std::move(*this)};

   this->swap(other);

   return *this;
}

void temp_buffer_allocator::page::swap(page& other)
{
   using std::swap;

   swap(this->allocated, other.allocated);
   swap(this->base_address, other.base_address);
}

}