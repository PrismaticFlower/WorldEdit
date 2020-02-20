#pragma once

#include "descriptor_heap.hpp"
#include "utility/com_ptr.hpp"

#include <array>
#include <utility>

#include <d3d12.h>
#include <dxgi1_6.h>

namespace sk::graphics {

struct swap_chain {
   swap_chain() = default;

   swap_chain(const HWND window, IDXGIFactory7& factory, ID3D12Device& device,
              ID3D12CommandQueue& command_queue,
              descriptor_heap_cpu& rtv_descriptor_heap);

   void wait_for_ready();

   void present();

   void resize(const UINT width, const UINT height);

   auto current_back_buffer()
      -> std::pair<ID3D12Resource&, D3D12_CPU_DESCRIPTOR_HANDLE>;

   auto width() -> UINT
   {
      return _width;
   }

   auto height() -> UINT
   {
      return _height;
   }

   constexpr static int frame_count = 2;

   utility::com_ptr<IDXGISwapChain4> dxgi_swap_chain;
   std::array<utility::com_ptr<ID3D12Resource>, frame_count> render_targets;
   std::array<D3D12_CPU_DESCRIPTOR_HANDLE, frame_count> render_target_views;

private:
   ID3D12Device* _device;
   HANDLE _waitable_ready_handle = INVALID_HANDLE_VALUE;
   UINT _width = 0;
   UINT _height = 0;
};

}