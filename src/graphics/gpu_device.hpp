#pragma once

#include "command_allocator_pool.hpp"
#include "descriptor_heap.hpp"
#include "swap_chain.hpp"
#include "utility/com_ptr.hpp"

#include <array>
#include <memory>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wil/resource.h>

namespace sk::graphics {

struct gpu_device {
   explicit gpu_device(const HWND window);

   gpu_device(const gpu_device&) = delete;
   gpu_device operator=(const gpu_device&) = delete;

   gpu_device(gpu_device&&) = delete;
   gpu_device operator=(gpu_device&&) = delete;

   ~gpu_device();

   void wait_for_idle();

   void end_frame();

   constexpr static int rtv_descriptor_heap_size = 128;

   utility::com_ptr<IDXGIFactory7> factory;
   utility::com_ptr<IDXGIAdapter4> adapter;
   utility::com_ptr<ID3D12Device6> device;
   utility::com_ptr<ID3D12Fence> fence;
   UINT64 fence_value = 1;
   UINT64 previous_frame_fence_value = 0;
   wil::unique_event fence_event{CreateEventW(nullptr, false, false, nullptr)};
   utility::com_ptr<ID3D12CommandQueue> command_queue;
   utility::com_ptr<ID3D12GraphicsCommandList5> command_list;

   descriptor_heap_cpu rtv_descriptor_heap{D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                                           rtv_descriptor_heap_size, *device};
   command_allocator_pool direct_command_allocator_pool{D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                        *device};

   swap_chain swap_chain;

   auto aquire_direct_command_allocator()
   {
      const auto free = [this](ID3D12CommandAllocator* allocator) {
         direct_command_allocator_pool.free(*allocator, fence_value);
      };

      return std::unique_ptr<ID3D12CommandAllocator, decltype(free)>{
         &direct_command_allocator_pool.aquire(fence->GetCompletedValue()), free};
   }
};

}