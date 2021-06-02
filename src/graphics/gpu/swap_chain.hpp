#pragma once

#include "descriptor_heap.hpp"
#include "types.hpp"
#include "utility/com_ptr.hpp"

#include <array>
#include <utility>

#include <d3d12.h>
#include <dxgi1_6.h>

namespace we::graphics::gpu {

struct swap_chain {
   swap_chain() = default;

   swap_chain(const HWND window, IDXGIFactory7& factory, ID3D12Device& device,
              ID3D12CommandQueue& command_queue, descriptor_heap& descriptor_heap_rtv);

   void wait_for_ready();

   void present();

   void resize(const uint16 width, const uint16 height);

   auto current_back_buffer()
      -> std::pair<ID3D12Resource&, D3D12_CPU_DESCRIPTOR_HANDLE>;

   auto width() -> uint16
   {
      return _width;
   }

   auto height() -> uint16
   {
      return _height;
   }

   constexpr static int frame_count = 2;
   constexpr static DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
   constexpr static DXGI_FORMAT format_rtv = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

   utility::com_ptr<IDXGISwapChain4> dxgi_swap_chain;
   std::array<utility::com_ptr<ID3D12Resource>, frame_count> render_targets;
   descriptor_range render_target_views;

private:
   ID3D12Device* _device;
   HANDLE _waitable_ready_handle = INVALID_HANDLE_VALUE;
   uint16 _width = 0;
   uint16 _height = 0;
};

}
