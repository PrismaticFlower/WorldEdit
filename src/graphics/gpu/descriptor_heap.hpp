#pragma once

#include "types.hpp"
#include "utility/com_ptr.hpp"

#include <mutex>
#include <stack>
#include <vector>

#include <atomic>
#include <d3d12.h>

namespace sk::graphics::gpu {

struct descriptor_handle {
   D3D12_CPU_DESCRIPTOR_HANDLE cpu{};
   D3D12_GPU_DESCRIPTOR_HANDLE gpu{};
};

class descriptor_range {
public:
   descriptor_range() = default;

   descriptor_range(const descriptor_handle handle, const uint32 count) noexcept
      : _handle{handle}, _count{count} {};

   auto start() const noexcept -> descriptor_handle
   {
      return _handle;
   }

   auto size() const noexcept -> uint32
   {
      return _count;
   }

private:
   descriptor_handle _handle{};
   uint32 _count = 0;
};

class descriptor_heap {
public:
   descriptor_heap(const D3D12_DESCRIPTOR_HEAP_TYPE type,
                   const uint32 static_descriptor_count,
                   const uint32 dynamic_descriptor_count, ID3D12Device6& device);

   auto get() const noexcept -> ID3D12DescriptorHeap&;

   auto allocate_static(const uint32 count) -> descriptor_range;

   void free_static(descriptor_range range);

   auto allocate_dynamic(const uint32 count) -> descriptor_range;

   void reset_dynamic(const std::size_t new_frame_index) noexcept;

private:
   uint32 _descriptor_size{};
   utility::com_ptr<ID3D12DescriptorHeap> _descriptor_heap;

   descriptor_handle _root_static_handle;
   uint32 _static_count = 0;
   std::atomic_uint32_t _static_used_count = 0;
   std::mutex _static_free_list_mutex;
   std::vector<descriptor_range> _static_free_list;

   std::size_t _frame_index = 0;
   std::array<descriptor_handle, 2> _root_dynamic_handles;
   uint32 _dynamic_count = 0;
   std::atomic_uint32_t _dynamic_used_count = 0;
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