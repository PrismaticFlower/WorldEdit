#pragma once

#include "utility/com_ptr.hpp"

#include <array>

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

   constexpr static int frame_count = 2;
   int frame_index = 0;

   utility::com_ptr<IDXGIFactory7> factory;
   utility::com_ptr<IDXGIAdapter4> adapter;
   utility::com_ptr<ID3D12Device6> device;
   utility::com_ptr<ID3D12Fence> fence;
   wil::unique_event fence_event{CreateEventW(nullptr, false, false, nullptr)};
   utility::com_ptr<ID3D12CommandQueue> command_queue;
   utility::com_ptr<ID3D12GraphicsCommandList5> command_list;

   std::array<utility::com_ptr<ID3D12CommandAllocator>, frame_count> command_allocators;
   std::array<UINT64, frame_count> fence_values{1, 1};

   utility::com_ptr<IDXGISwapChain4> swap_chain;
   std::array<utility::com_ptr<ID3D12Resource>, frame_count> swap_chain_render_targets;
   utility::com_ptr<ID3D12DescriptorHeap> swap_chain_descriptors;

   UINT cbv_srv_uav_descriptor_size{};
   UINT sampler_descriptor_size{};
   UINT rtv_descriptor_size{};
   UINT dsv_descriptor_size{};
};

}