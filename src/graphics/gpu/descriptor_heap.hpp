#pragma once

#include "descriptor_range.hpp"
#include "types.hpp"
#include "utility/com_ptr.hpp"

#include <atomic>
#include <mutex>
#include <stack>
#include <vector>

#include <d3d12.h>

namespace sk::graphics::gpu {

class descriptor_heap {
public:
   descriptor_heap(const D3D12_DESCRIPTOR_HEAP_TYPE type,
                   const D3D12_DESCRIPTOR_HEAP_FLAGS flags,
                   const uint32 descriptor_count, ID3D12Device6& device);

   auto get() const noexcept -> ID3D12DescriptorHeap&;

   auto allocate_static(const uint32 count) -> descriptor_range;

   void free_static(descriptor_range range);

private:
   uint32 _descriptor_size{};
   utility::com_ptr<ID3D12DescriptorHeap> _descriptor_heap;

   descriptor_handle _root_handle;
   uint32 _count = 0;
   std::atomic_uint32_t _used_count = 0;
   std::mutex _free_ranges_mutex;
   std::vector<descriptor_range> _free_ranges;
};

class descriptor_heap_cpu {
public:
   descriptor_heap_cpu(const D3D12_DESCRIPTOR_HEAP_TYPE type,
                       const uint32 descriptor_count, ID3D12Device& device);

   auto allocate() -> D3D12_CPU_DESCRIPTOR_HANDLE;

   void free(D3D12_CPU_DESCRIPTOR_HANDLE handle);

private:
   std::mutex _mutex;

   uint32 _descriptor_size{};
   utility::com_ptr<ID3D12DescriptorHeap> _descriptor_heap;

   uint32 _descriptor_count = 0;
   uint32 _used_count = 0;

   D3D12_CPU_DESCRIPTOR_HANDLE _root_handle;

   std::stack<D3D12_CPU_DESCRIPTOR_HANDLE, std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>> _freed_stack;
};

}