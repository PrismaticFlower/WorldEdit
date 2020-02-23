
#include "device.hpp"
#include "hresult_error.hpp"

#include <algorithm>

#include <d3dx12.h>

namespace sk::graphics::gpu {

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

   throw_if_failed(
      device_d3d->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                              IID_PPV_ARGS(copy_fence.clear_and_assign())));

   queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;

   throw_if_failed(
      device_d3d->CreateCommandQueue(&queue_desc,
                                     IID_PPV_ARGS(copy_command_queue.clear_and_assign())));

   swap_chain = {window, *factory, *device_d3d, *command_queue, rtv_descriptor_heap};

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
   const UINT64 wait_value = previous_frame_fence_value;
   throw_if_failed(command_queue->Signal(fence.get(), wait_value));
   previous_frame_fence_value = fence_value++;
   completed_fence_value = fence->GetCompletedValue();
   copy_completed_fence_value = copy_fence->GetCompletedValue();

   if (completed_fence_value < wait_value) {
      throw_if_failed(fence->SetEventOnCompletion(wait_value, fence_event.get()));
      WaitForSingleObject(fence_event.get(), INFINITE);
      completed_fence_value = fence->GetCompletedValue();
   }
}

auto device::create_buffer(const UINT size, const D3D12_HEAP_TYPE heap_type,
                           const D3D12_RESOURCE_STATES initial_resource_state) -> buffer
{

   const D3D12_HEAP_PROPERTIES heap_properties{.Type = heap_type,
                                               .CPUPageProperty =
                                                  D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                                               .MemoryPoolPreference =
                                                  D3D12_MEMORY_POOL_UNKNOWN};

   const auto buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(size);

   utility::com_ptr<ID3D12Resource> buffer_resource;

   throw_if_failed(device_d3d->CreateCommittedResource(
      &heap_properties, D3D12_HEAP_FLAG_NONE, &buffer_desc, initial_resource_state,
      nullptr, IID_PPV_ARGS(buffer_resource.clear_and_assign())));

   buffer buffer;

   buffer.parent_device = this;
   buffer.resource = buffer_resource.release();
   buffer.resource_state = initial_resource_state;
   buffer.size = size;

   return buffer;
}

void device::deferred_destroy_resource(ID3D12Resource& resource)
{
   std::lock_guard lock{_deferred_destruction_mutex};

   _deferred_resource_destructions.emplace_back(&resource);
}

void device::process_deferred_resource_destructions()
{
   std::lock_guard lock{_deferred_destruction_mutex};

   if (_deferred_resource_destructions.empty()) return;

   // Simply waiting for the GPU to go idle isn't very sophisticated but resource destruction
   // should be rare enough that it doesn't matter. If the stalls introduced are actually proven
   // to be a problem then this can be extended to work
   // asynchronously off fence values. But for now this will do.
   wait_for_idle();

   _deferred_resource_destructions.clear();
}

}
