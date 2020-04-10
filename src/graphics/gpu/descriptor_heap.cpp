
#include "descriptor_heap.hpp"
#include "common.hpp"
#include "hresult_error.hpp"

#include <cassert>
#include <stdexcept>

#include <range/v3/algorithm/find_if.hpp>

namespace sk::graphics::gpu {

namespace {

template<typename Type>
auto index_descriptor_ptr(const Type ptr, const std::type_identity_t<Type> index,
                          const std::type_identity_t<Type> size)
{
   return ptr + (index * size);
}

template<typename Type>
auto index_descriptor_handle(const descriptor_handle handle, Type index,
                             const std::type_identity_t<Type> size) -> descriptor_handle
{
   const Type offset = (index * size);

   return {.cpu = {handle.cpu.ptr + offset}, .gpu = {handle.gpu.ptr + offset}};
}

}

descriptor_heap::descriptor_heap(const D3D12_DESCRIPTOR_HEAP_TYPE type,
                                 const uint32 static_descriptor_count,
                                 const uint32 dynamic_descriptor_count,
                                 ID3D12Device6& device)
   : _descriptor_size{device.GetDescriptorHandleIncrementSize(type)},
     _static_count{static_descriptor_count},
     _dynamic_count{dynamic_descriptor_count}
{
   const uint32 descriptor_count = _static_count + (_dynamic_count * render_latency);

   const D3D12_DESCRIPTOR_HEAP_DESC desc{.Type = type,
                                         .NumDescriptors = descriptor_count,
                                         .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE};

   throw_if_failed(
      device.CreateDescriptorHeap(&desc,
                                  IID_PPV_ARGS(_descriptor_heap.clear_and_assign())));

   const descriptor_handle
      root_handle{.cpu = _descriptor_heap->GetCPUDescriptorHandleForHeapStart(),
                  .gpu = _descriptor_heap->GetGPUDescriptorHandleForHeapStart()};

   _root_static_handle = root_handle;
   _root_dynamic_handles = {
      index_descriptor_handle(root_handle, _static_count, _descriptor_size),
      index_descriptor_handle(root_handle, _static_count + _dynamic_count, _descriptor_size),
   };
}

auto descriptor_heap::get() const noexcept -> ID3D12DescriptorHeap&
{
   return *_descriptor_heap;
}

auto descriptor_heap::allocate_static(const uint32 count) -> descriptor_range
{
   // check if an exact size match exists in the free list
   if (std::lock_guard lock{_static_free_list_mutex}; not _static_free_list.empty()) {
      if (auto match = ranges::find_if(_static_free_list,
                                       [count](const auto& range) {
                                          return range.size() == count;
                                       });
          match != _static_free_list.end()) {
         const descriptor_range range = *match;

         _static_free_list.erase(match);

         return range;
      }
   }

   // check if we can allocate a new range
   const uint32 range_offset =
      _static_used_count.fetch_add(count, std::memory_order_relaxed);

   if ((range_offset + count) <= _static_count) {
      return {index_descriptor_handle(_root_static_handle, range_offset, _descriptor_size),
              count};
   }

   // check if a partial size match exists in the free list
   if (std::lock_guard lock{_static_free_list_mutex}; not _static_free_list.empty()) {
      if (auto match = ranges::find_if(_static_free_list,
                                       [count](const auto& range) {
                                          return range.size() >= count;
                                       });
          match != _static_free_list.end()) {
         descriptor_range range = *match;

         if (range.size() > count) {
            *match = {index_descriptor_handle(range.start(), count, _descriptor_size),
                      range.size() - count};
         }

         return {range.start(), count};
      }
   }

   throw std::runtime_error{"out of space in static descriptor heap!"};
}

void descriptor_heap::free_static(descriptor_range range)
{
   std::lock_guard lock{_static_free_list_mutex};
   _static_free_list.push_back(range);
}

auto descriptor_heap::allocate_dynamic(const uint32 count) -> descriptor_range
{
   const uint32 range_offset =
      _dynamic_used_count.fetch_add(count, std::memory_order_relaxed);

   if ((range_offset + count) > _dynamic_count) {
      throw std::runtime_error{"out of space in dynamic descriptor heap!"};
   }

   return {index_descriptor_handle(_root_dynamic_handles[_frame_index],
                                   range_offset, _descriptor_size),
           count};
}

void descriptor_heap::reset_dynamic(const std::size_t new_frame_index) noexcept
{
   _frame_index = new_frame_index;
   _dynamic_used_count.store(0, std::memory_order_relaxed);
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