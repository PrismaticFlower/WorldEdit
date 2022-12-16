#pragma once

#include "descriptor_allocator.hpp"
#include "error.hpp"
#include "utility/com_ptr.hpp"

#include <d3d12.h>

namespace we::graphics::gpu::detail {

struct descriptor_heap {
   descriptor_heap(ID3D12Device& device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
      : allocator{desc.NumDescriptors}
   {
      throw_if_fail(
         device.CreateDescriptorHeap(&desc, IID_PPV_ARGS(heap.clear_and_assign())));

      descriptor_size = device.GetDescriptorHandleIncrementSize(desc.Type);
   }

   auto index_cpu(uint32 index) const noexcept -> D3D12_CPU_DESCRIPTOR_HANDLE
   {
      return {heap->GetCPUDescriptorHandleForHeapStart().ptr + (index * descriptor_size)};
   }

   auto index_gpu(uint32 index) const noexcept -> D3D12_GPU_DESCRIPTOR_HANDLE
   {
      return {heap->GetGPUDescriptorHandleForHeapStart().ptr + (index * descriptor_size)};
   }

   auto to_index(D3D12_CPU_DESCRIPTOR_HANDLE descriptor) const noexcept -> uint32
   {
      return static_cast<uint32>(
         (descriptor.ptr - heap->GetCPUDescriptorHandleForHeapStart().ptr) /
         descriptor_size);
   }

   auto to_index(D3D12_GPU_DESCRIPTOR_HANDLE descriptor) const noexcept -> uint32
   {
      return static_cast<uint32>(
         (descriptor.ptr - heap->GetGPUDescriptorHandleForHeapStart().ptr) /
         descriptor_size);
   }

   descriptor_allocator allocator;
   utility::com_ptr<ID3D12DescriptorHeap> heap;
   uint32 descriptor_size = 0;
};

}