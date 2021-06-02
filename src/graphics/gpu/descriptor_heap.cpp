
#include "descriptor_heap.hpp"
#include "common.hpp"
#include "hresult_error.hpp"

#include <cassert>
#include <stdexcept>

#include <range/v3/algorithm/find_if.hpp>

namespace we::graphics::gpu {

namespace {

template<typename Type>
auto index_descriptor_ptr(const Type ptr, const std::type_identity_t<Type> index,
                          const std::type_identity_t<Type> size)
{
   return ptr + (index * size);
}

}

descriptor_heap::descriptor_heap(const D3D12_DESCRIPTOR_HEAP_TYPE type,
                                 const D3D12_DESCRIPTOR_HEAP_FLAGS flags,
                                 const uint32 descriptor_count, ID3D12Device6& device)
   : _descriptor_size{device.GetDescriptorHandleIncrementSize(type)}, _count{descriptor_count}
{
   const D3D12_DESCRIPTOR_HEAP_DESC desc{.Type = type,
                                         .NumDescriptors = _count,
                                         .Flags = flags};

   throw_if_failed(
      device.CreateDescriptorHeap(&desc,
                                  IID_PPV_ARGS(_descriptor_heap.clear_and_assign())));

   const descriptor_handle
      root_handle{.cpu = _descriptor_heap->GetCPUDescriptorHandleForHeapStart(),
                  .gpu = _descriptor_heap->GetGPUDescriptorHandleForHeapStart()};

   _root_handle = root_handle;
}

auto descriptor_heap::get() const noexcept -> ID3D12DescriptorHeap&
{
   return *_descriptor_heap;
}

auto descriptor_heap::allocate_static(const uint32 count) -> descriptor_range
{
   // check if an exact size match exists in the free list
   if (std::lock_guard lock{_free_ranges_mutex}; not _free_ranges.empty()) {
      if (auto match = ranges::find_if(_free_ranges,
                                       [count](const auto& range) {
                                          return range.size() == count;
                                       });
          match != _free_ranges.end()) {
         const descriptor_range range = *match;

         _free_ranges.erase(match);

         return range;
      }
   }

   // check if we can allocate a new range
   const uint32 range_offset = _used_count.fetch_add(count, std::memory_order_relaxed);

   if ((range_offset + count) <= _count) {
      return {index_descriptor_handle(_root_handle, range_offset, _descriptor_size),
              count, _descriptor_size};
   }

   // check if a partial size match exists in the free list
   if (std::lock_guard lock{_free_ranges_mutex}; not _free_ranges.empty()) {
      if (auto match = ranges::find_if(_free_ranges,
                                       [count](const auto& range) {
                                          return range.size() >= count;
                                       });
          match != _free_ranges.end()) {
         descriptor_range range = *match;

         if (range.size() > count) {
            *match = {index_descriptor_handle(range.start(), count, _descriptor_size),
                      range.size() - count, _descriptor_size};
         }

         return {range.start(), count, _descriptor_size};
      }
   }

   throw std::runtime_error{"out of space in static descriptor heap!"};
}

void descriptor_heap::free_static(descriptor_range range)
{
   std::lock_guard lock{_free_ranges_mutex};
   _free_ranges.push_back(range);
}

descriptor_heap_cpu::descriptor_heap_cpu(const D3D12_DESCRIPTOR_HEAP_TYPE type,
                                         const uint32 descriptor_count,
                                         ID3D12Device& device)

   : _descriptor_size{device.GetDescriptorHandleIncrementSize(type)}, _descriptor_count{descriptor_count}
{
   const D3D12_DESCRIPTOR_HEAP_DESC desc{.Type = type,
                                         .NumDescriptors = descriptor_count};

   throw_if_failed(
      device.CreateDescriptorHeap(&desc,
                                  IID_PPV_ARGS(_descriptor_heap.clear_and_assign())));

   _root_handle = _descriptor_heap->GetCPUDescriptorHandleForHeapStart();
}

auto descriptor_heap_cpu::allocate() -> D3D12_CPU_DESCRIPTOR_HANDLE
{
   std::lock_guard lock{_mutex};

   if (not _freed_stack.empty()) {
      const auto handle = _freed_stack.top();

      _freed_stack.pop();

      return handle;
   }

   if (_used_count == _descriptor_count) {
      throw std::runtime_error{"out of space in descriptor heap!"};
   }

   const UINT index = _used_count;
   ++_used_count;

   return {.ptr = index_descriptor_ptr(_root_handle.ptr, index, _descriptor_size)};
}

void descriptor_heap_cpu::free(D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
   std::lock_guard lock{_mutex};

   assert(handle.ptr >= _root_handle.ptr and
          handle.ptr < index_descriptor_ptr(_root_handle.ptr, _descriptor_count,
                                            _descriptor_size));

   _freed_stack.push(handle);
}

}
