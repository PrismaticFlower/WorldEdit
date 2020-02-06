
#include "gpu_device.hpp"
#include "hresult_error.hpp"

#include <algorithm>

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

auto create_device(IDXGIAdapter4& adapter) -> utility::com_ptr<ID3D12Device6>
{
#ifndef NDEBUG
   utility::com_ptr<ID3D12Debug> d3d_debug;
   throw_if_failed(D3D12GetDebugInterface(IID_PPV_ARGS(d3d_debug.clear_and_assign())));

   d3d_debug->EnableDebugLayer();
#endif

   utility::com_ptr<ID3D12Device6> device;

   throw_if_failed(D3D12CreateDevice(&adapter, D3D_FEATURE_LEVEL_11_0,
                                     IID_PPV_ARGS(device.clear_and_assign())));

   return device;
}

}

gpu_device::gpu_device(const HWND window)
   : factory{create_factory()}, adapter{create_adapter(*factory)}, device{create_device(*adapter)}
{
   throw_if_failed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                       IID_PPV_ARGS(fence.clear_and_assign())));

   const D3D12_COMMAND_QUEUE_DESC queue_desc{.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                                             .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE};

   fence_event = wil::unique_event{CreateEventW(nullptr, false, false, nullptr)};

   throw_if_failed(
      device->CreateCommandQueue(&queue_desc,
                                 IID_PPV_ARGS(command_queue.clear_and_assign())));
   throw_if_failed(
      device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                 D3D12_COMMAND_LIST_FLAG_NONE,
                                 IID_PPV_ARGS(command_list.clear_and_assign())));

   swap_chain = {window, *factory, *device, *command_queue, rtv_descriptor_heap};

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
   const UINT64 wait_value = fence_value;
   throw_if_failed(command_queue->Signal(fence.get(), wait_value));
   ++fence_value;

   if (fence->GetCompletedValue() < wait_value) {
      throw_if_failed(fence->SetEventOnCompletion(wait_value, fence_event.get()));
      WaitForSingleObject(fence_event.get(), INFINITE);
   }
}

void gpu_device::end_frame()
{
   const UINT64 wait_value = previous_frame_fence_value;
   throw_if_failed(command_queue->Signal(fence.get(), wait_value));
   previous_frame_fence_value = fence_value;
   ++fence_value;

   if (fence->GetCompletedValue() < wait_value) {
      throw_if_failed(fence->SetEventOnCompletion(wait_value, fence_event.get()));
      WaitForSingleObject(fence_event.get(), INFINITE);
   }
}

}
