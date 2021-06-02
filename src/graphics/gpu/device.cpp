
#include "device.hpp"
#include "hresult_error.hpp"

#include <d3dx12.h>

namespace we::graphics::gpu {

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

device::device(const HWND window)
   : factory{create_factory()}, adapter{create_adapter(*factory)}, device_d3d{create_device(*adapter)}
{
   throw_if_failed(device_d3d->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                           IID_PPV_ARGS(fence.clear_and_assign())));

   D3D12_COMMAND_QUEUE_DESC queue_desc{.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                                       .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE};

   fence_event = wil::unique_event{CreateEventW(nullptr, false, false, nullptr)};

   throw_if_failed(
      device_d3d->CreateCommandQueue(&queue_desc,
                                     IID_PPV_ARGS(command_queue.clear_and_assign())));

   swap_chain = {window, *factory, *device_d3d, *command_queue, descriptor_heap_rtv};

   if (utility::com_ptr<ID3D12InfoQueue> info_queue;
       SUCCEEDED(device_d3d->QueryInterface(info_queue.clear_and_assign()))) {
      info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
      info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
      info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
   }
}

device::~device()
{
   wait_for_idle();
}

void device::wait_for_idle()
{
   const UINT64 wait_value = fence_value++;
   throw_if_failed(command_queue->Signal(fence.get(), wait_value));
   completed_fence_value = fence->GetCompletedValue();

   if (completed_fence_value < wait_value) {
      throw_if_failed(fence->SetEventOnCompletion(wait_value, fence_event.get()));
      WaitForSingleObject(fence_event.get(), INFINITE);
      completed_fence_value = fence->GetCompletedValue();
   }
}

void device::end_frame()
{
   copy_manager.update_completed();

   const UINT64 wait_value = previous_frame_fence_value;
   throw_if_failed(command_queue->Signal(fence.get(), wait_value));
   previous_frame_fence_value = fence_value++;
   frame_index = fence_value % render_latency;
   completed_fence_value = fence->GetCompletedValue();

   process_deferred_resource_destructions();

   if (completed_fence_value < wait_value) {
      throw_if_failed(fence->SetEventOnCompletion(wait_value, fence_event.get()));
      WaitForSingleObject(fence_event.get(), INFINITE);
      completed_fence_value = fence->GetCompletedValue();
   }
}

auto device::create_command_allocators(const D3D12_COMMAND_LIST_TYPE type) -> command_allocators
{
   command_allocators allocators;

   for (auto& allocator : allocators) {
      throw_if_failed(
         device_d3d->CreateCommandAllocator(type, IID_PPV_ARGS(
                                                     allocator.clear_and_assign())));
   }

   return allocators;
}

auto device::create_command_list(const D3D12_COMMAND_LIST_TYPE type)
   -> utility::com_ptr<ID3D12GraphicsCommandList5>
{
   utility::com_ptr<ID3D12GraphicsCommandList5> command_list;

   throw_if_failed(
      device_d3d->CreateCommandList1(0, type, D3D12_COMMAND_LIST_FLAG_NONE,
                                     IID_PPV_ARGS(command_list.clear_and_assign())));

   return command_list;
}

void device::process_deferred_resource_destructions()
{
   std::scoped_lock lock{_deferred_destruction_mutex};

   std::erase_if(_deferred_destructions, [=](const deferred_destruction& resource) {
      return resource.last_used_frame <= completed_fence_value;
   });
}

}
