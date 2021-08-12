
#include "device.hpp"
#include "hresult_error.hpp"

#include <ranges>

#include <d3dx12.h>

namespace we::graphics::gpu {

namespace {

constexpr bool use_debug_layer = true;

auto create_factory() -> utility::com_ptr<IDXGIFactory7>
{
   UINT dxgi_flags = 0;

   if (use_debug_layer) dxgi_flags = DXGI_CREATE_FACTORY_DEBUG;

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
   if (use_debug_layer) {
      utility::com_ptr<ID3D12Debug> d3d_debug;
      throw_if_failed(
         D3D12GetDebugInterface(IID_PPV_ARGS(d3d_debug.clear_and_assign())));

      d3d_debug->EnableDebugLayer();
   }

   utility::com_ptr<ID3D12Device6> device;

   throw_if_failed(D3D12CreateDevice(&adapter, D3D_FEATURE_LEVEL_11_0,
                                     IID_PPV_ARGS(device.clear_and_assign())));

   return device;
}

auto create_root_signature(ID3D12Device& device,
                           const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& desc,
                           const std::string_view name)
   -> utility::com_ptr<ID3D12RootSignature>
{
   utility::com_ptr<ID3DBlob> root_signature_blob;

   if (utility::com_ptr<ID3DBlob> root_signature_error_blob;
       FAILED(D3D12SerializeVersionedRootSignature(
          &desc, root_signature_blob.clear_and_assign(),
          root_signature_error_blob.clear_and_assign()))) {
      std::string message{static_cast<const char*>(
                             root_signature_error_blob->GetBufferPointer()),
                          root_signature_error_blob->GetBufferSize()};

      throw std::runtime_error{std::move(message)};
   }

   utility::com_ptr<ID3D12RootSignature> root_sig;

   throw_if_failed(
      device.CreateRootSignature(0, root_signature_blob->GetBufferPointer(),
                                 root_signature_blob->GetBufferSize(),
                                 IID_PPV_ARGS(root_sig.clear_and_assign())));

   if (not name.empty()) {
      root_sig->SetPrivateData(WKPDID_D3DDebugObjectName,
                               to_uint32(name.size()), name.data());
   }

   return root_sig;
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

   const D3D12MA::ALLOCATOR_DESC alloc_desc{.pDevice = device_d3d.get(),
                                            .pAdapter = adapter.get()};

   throw_if_failed(
      D3D12MA::CreateAllocator(&alloc_desc, _allocator.clear_and_assign()));
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

auto device::create_root_signature(const root_signature_desc& desc)
   -> utility::com_ptr<ID3D12RootSignature>
{
   boost::container::static_vector<D3D12_DESCRIPTOR_RANGE1, 256> descriptor_ranges_stack;
   boost::container::small_vector<D3D12_ROOT_PARAMETER1, 16> parameters;
   boost::container::small_vector<D3D12_STATIC_SAMPLER_DESC, 16> samplers;

   for (auto& param : desc.parameters) {
      auto d3d12_param = boost::variant2::visit(
         [&]<typename T>(const T& param) -> D3D12_ROOT_PARAMETER1 {
            if constexpr (std::is_same_v<T, gpu::root_parameter_descriptor_table>) {
               const auto ranges_stack_offset = descriptor_ranges_stack.size();

               descriptor_ranges_stack.resize(ranges_stack_offset +
                                              param.ranges.size());

               std::copy_n(param.ranges.begin(), param.ranges.size(),
                           descriptor_ranges_stack.begin() + ranges_stack_offset);

               return D3D12_ROOT_PARAMETER1{
                  .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                  .DescriptorTable =
                     {
                        .NumDescriptorRanges = to_uint32(param.ranges.size()),
                        .pDescriptorRanges = &descriptor_ranges_stack[ranges_stack_offset],
                     },
                  .ShaderVisibility = param.visibility};
            }
            else {
               return param;
            }
         },
         param);

      parameters.push_back(d3d12_param);
   }

   samplers.resize(desc.samplers.size());

   std::ranges::copy(desc.samplers, samplers.begin());

   const D3D12_VERSIONED_ROOT_SIGNATURE_DESC
      d3d12_desc{.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
                 .Desc_1_1 = {.NumParameters = to_uint32(parameters.size()),
                              .pParameters = parameters.data(),
                              .NumStaticSamplers = to_uint32(samplers.size()),
                              .pStaticSamplers = samplers.data(),
                              .Flags = desc.flags}};

   return gpu::create_root_signature(*device_d3d, d3d12_desc, desc.name);
}

void device::process_deferred_resource_destructions()
{
   std::scoped_lock lock{_deferred_destruction_mutex};

   std::erase_if(_deferred_destructions, [=](const deferred_destruction& resource) {
      return resource.last_used_frame <= completed_fence_value;
   });
}

}
