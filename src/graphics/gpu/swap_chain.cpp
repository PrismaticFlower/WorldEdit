
#include "swap_chain.hpp"
#include "hresult_error.hpp"

#include <cassert>

namespace we::graphics::gpu {
namespace {

auto create_dxgi_swapchain(const HWND window, IDXGIFactory7& factory,
                           ID3D12CommandQueue& command_queue)
   -> utility::com_ptr<IDXGISwapChain4>
{
   RECT rect{};
   GetWindowRect(window, &rect);

   const DXGI_SWAP_CHAIN_DESC1 desc{.Width = static_cast<UINT>(rect.right - rect.left),
                                    .Height =
                                       static_cast<UINT>(rect.bottom - rect.top),
                                    .Format = swap_chain::format,
                                    .SampleDesc =
                                       {
                                          .Count = 1,
                                       },
                                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                                    .BufferCount = swap_chain::frame_count,
                                    .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
                                    .Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT};

   utility::com_ptr<IDXGISwapChain1> swap_chain_1;
   utility::com_ptr<IDXGISwapChain4> swap_chain_4;

   throw_if_failed(
      factory.CreateSwapChainForHwnd(&command_queue, window, &desc, nullptr,
                                     nullptr, swap_chain_1.clear_and_assign()));

   throw_if_failed(swap_chain_1->QueryInterface(swap_chain_4.clear_and_assign()));

   // throw_if_failed(factory.MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER));

   return swap_chain_4;
}

}

swap_chain::swap_chain(const HWND window, IDXGIFactory7& factory,
                       ID3D12Device& device, ID3D12CommandQueue& command_queue,
                       descriptor_heap& descriptor_heap_rtv)
   : dxgi_swap_chain{create_dxgi_swapchain(window, factory, command_queue)},
     render_target_views{descriptor_heap_rtv.allocate_static(frame_count)},
     _device{&device},
     _waitable_ready_handle{dxgi_swap_chain->GetFrameLatencyWaitableObject()}
{
   DXGI_SWAP_CHAIN_DESC1 desc{};
   dxgi_swap_chain->GetDesc1(&desc);

   _width = static_cast<uint16>(desc.Width);
   _height = static_cast<uint16>(desc.Height);

   for (int i = 0; i < frame_count; ++i) {
      throw_if_failed(
         dxgi_swap_chain->GetBuffer(i, IID_PPV_ARGS(
                                          render_targets[i].clear_and_assign())));

      const D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{.Format = format_rtv,
                                                   .ViewDimension =
                                                      D3D12_RTV_DIMENSION_TEXTURE2D};

      _device->CreateRenderTargetView(render_targets[i].get(), &rtv_desc,
                                      render_target_views[i].cpu);
   }
}

void swap_chain::wait_for_ready()
{
   WaitForSingleObjectEx(_waitable_ready_handle, 1000, true);
}

void swap_chain::present()
{
   throw_if_failed(dxgi_swap_chain->Present(1, 0));
}

void swap_chain::resize(const uint16 width, const uint16 height)
{
   assert(dxgi_swap_chain);

   _width = width;
   _height = height;
   render_targets = {};

   throw_if_failed(
      dxgi_swap_chain->ResizeBuffers(frame_count, width, height, format,
                                     DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT));

   for (int i = 0; i < frame_count; ++i) {
      throw_if_failed(
         dxgi_swap_chain->GetBuffer(i, IID_PPV_ARGS(
                                          render_targets[i].clear_and_assign())));

      const D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{.Format = format_rtv,
                                                   .ViewDimension =
                                                      D3D12_RTV_DIMENSION_TEXTURE2D};

      _device->CreateRenderTargetView(render_targets[i].get(), &rtv_desc,
                                      render_target_views[i].cpu);
   }
}

auto swap_chain::current_back_buffer()
   -> std::pair<ID3D12Resource&, D3D12_CPU_DESCRIPTOR_HANDLE>
{
   const auto index = dxgi_swap_chain->GetCurrentBackBufferIndex();

   return {*render_targets[index], render_target_views[index].cpu};
}

}
