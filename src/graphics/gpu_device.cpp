
#include <graphics/gpu_device.hpp>

#include <hresult_error.hpp>

#include <algorithm>

#include <d3dx12.h>

namespace sk::graphics {

namespace {
auto create_factory() -> utility::com_ptr<IDXGIFactory7>
{
   UINT dxgi_flags = 0;

#ifndef NDEBUG
   dxgi_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif

   utility::com_ptr<IDXGIFactory7> factory;

   throw_if_failed(
      CreateDXGIFactory2(dxgi_flags, IID_PPV_ARGS(factory.clear_and_assign())));

   return factory;
}

auto create_adapter(IDXGIFactory7& factory) -> utility::com_ptr<IDXGIAdapter4>
{
   utility::com_ptr<IDXGIAdapter4> adapter;

   for (int i = 0; SUCCEEDED(
           factory.EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                                              IID_PPV_ARGS(adapter.clear_and_assign())));
        ++i) {
      if (SUCCEEDED(D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_11_0,
                                      __uuidof(ID3D12Device), nullptr))) {
         return adapter;
      }
   }

   factory.EnumWarpAdapter(IID_PPV_ARGS(adapter.clear_and_assign()));

   return adapter;
}

auto create_swapchain(const HWND window, IDXGIFactory7& factory,
                      ID3D12CommandQueue& command_queue)
   -> utility::com_ptr<IDXGISwapChain4>
{
   RECT rect{};
   GetWindowRect(window, &rect);

   const DXGI_SWAP_CHAIN_DESC1 desc{.Width = static_cast<UINT>(rect.right - rect.left),
                                    .Height =
                                       static_cast<UINT>(rect.bottom - rect.top),
                                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                                    .SampleDesc =
                                       {
                                          .Count = 1,
                                       },
                                    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                                    .BufferCount = gpu_device::frame_count,
                                    .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD};

   utility::com_ptr<IDXGISwapChain1> swap_chain_1;
   utility::com_ptr<IDXGISwapChain4> swap_chain_4;

   throw_if_failed(
      factory.CreateSwapChainForHwnd(&command_queue, window, &desc, nullptr,
                                     nullptr, swap_chain_1.clear_and_assign()));

   throw_if_failed(swap_chain_1->QueryInterface(swap_chain_4.clear_and_assign()));

   return swap_chain_4;
}

}

gpu_device::gpu_device(const HWND window)
{
#ifndef NDEBUG
   utility::com_ptr<ID3D12Debug> d3d_debug;
   throw_if_failed(D3D12GetDebugInterface(IID_PPV_ARGS(d3d_debug.clear_and_assign())));

   d3d_debug->EnableDebugLayer();
#endif

   factory = create_factory();
   adapter = create_adapter(*factory);

   throw_if_failed(D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_11_0,
                                     IID_PPV_ARGS(device.clear_and_assign())));
   throw_if_failed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                       IID_PPV_ARGS(fence.clear_and_assign())));

   const D3D12_COMMAND_QUEUE_DESC queue_desc{.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                                             .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE};

   fence_event = wil::unique_event{CreateEventW(nullptr, false, false, nullptr)};

   throw_if_failed(
      device->CreateCommandQueue(&queue_desc,
                                 IID_PPV_ARGS(command_queue.clear_and_assign())));

   for (auto& command_allocator : command_allocators) {
      throw_if_failed(
         device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                        IID_PPV_ARGS(
                                           command_allocator.clear_and_assign())));
   }

   throw_if_failed(
      device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                 D3D12_COMMAND_LIST_FLAG_NONE,
                                 IID_PPV_ARGS(command_list.clear_and_assign())));

   cbv_srv_uav_descriptor_size =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
   sampler_descriptor_size =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
   rtv_descriptor_size =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
   dsv_descriptor_size =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

   const D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc{.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                                                         .NumDescriptors = frame_count};

   swap_chain = create_swapchain(window, *factory, *command_queue);

   throw_if_failed(
      device->CreateDescriptorHeap(&descriptor_heap_desc,
                                   IID_PPV_ARGS(
                                      swap_chain_descriptors.clear_and_assign())));

   CD3DX12_CPU_DESCRIPTOR_HANDLE rtv{
      swap_chain_descriptors->GetCPUDescriptorHandleForHeapStart()};

   for (int i = 0; i < frame_count; ++i) {
      throw_if_failed(
         swap_chain->GetBuffer(i, IID_PPV_ARGS(
                                     swap_chain_render_targets[i].clear_and_assign())));

      const D3D12_RENDER_TARGET_VIEW_DESC rtv_desc{.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                                                   .ViewDimension =
                                                      D3D12_RTV_DIMENSION_TEXTURE2D};

      device->CreateRenderTargetView(swap_chain_render_targets[i].get(),
                                     &rtv_desc, rtv);
      rtv.Offset(1, rtv_descriptor_size);
   }

   frame_index = swap_chain->GetCurrentBackBufferIndex();

   if (utility::com_ptr<ID3D12InfoQueue> info_queue;
       SUCCEEDED(device->QueryInterface(info_queue.clear_and_assign()))) {
      info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
      info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
      info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
   }
}

gpu_device::~gpu_device()
{
   wait_for_idle();
}

void gpu_device::wait_for_idle()
{
   const UINT64 wait_value = fence_values[frame_index];
   throw_if_failed(command_queue->Signal(fence.get(), wait_value));
   ++fence_values[frame_index];

   if (fence->GetCompletedValue() < wait_value) {
      throw_if_failed(fence->SetEventOnCompletion(wait_value, fence_event.get()));
      WaitForSingleObject(fence_event.get(), INFINITE);
   }
}

void gpu_device::end_frame()
{
   frame_index = swap_chain->GetCurrentBackBufferIndex();

   wait_for_idle();

   throw_if_failed(command_allocators[frame_index]->Reset());
}

}
