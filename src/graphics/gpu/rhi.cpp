#include "rhi.hpp"
#include "detail/command_allocator_pool.hpp"
#include "detail/descriptor_heap.hpp"
#include "detail/error.hpp"
#include "detail/format_helpers.hpp"
#include "detail/handle_packing.hpp"
#include "detail/release_queue.hpp"
#include "io/output_file.hpp"
#include "utility/com_ptr.hpp"

#include <atomic>
#include <bit>
#include <shared_mutex>
#include <string>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>

#include <wil/resource.h>

#include <absl/container/inlined_vector.h>

namespace we::graphics::gpu {

using namespace detail::handle_packing;
using detail::command_allocator_pool;
using detail::descriptor_heap;
using detail::release_queue;
using detail::terminate_if_fail;
using detail::throw_if_fail;
using detail::unique_descriptor_releaser;

using utility::com_ptr;

namespace {

constexpr uint32 d3d12_srv_all_mips = 0xff'ff'ff'ffu;

constexpr uint32 num_cbv_srv_uav_descriptors = 131'072;
constexpr uint32 num_rtv_descriptors = 1024;
constexpr uint32 num_dsv_descriptors = 1024;

constexpr uint32 bindless_descriptor_space_start = 10'000;

auto create_dxgi_factory(const device_desc& device_desc) -> com_ptr<IDXGIFactory7>
{
   com_ptr<IDXGIFactory7> factory;

   terminate_if_fail(
      CreateDXGIFactory2(device_desc.enable_debug_layer ? DXGI_CREATE_FACTORY_DEBUG : 0,
                         IID_PPV_ARGS(factory.clear_and_assign())));

   return factory;
}

auto create_d3d12_device(IDXGIFactory7& factory, const device_desc& device_desc,
                         com_ptr<IDXGIAdapter4>& out_adapter) -> com_ptr<ID3D12Device10>
{
   if (device_desc.enable_debug_layer) {
      com_ptr<ID3D12Debug5> debug;

      if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debug.clear_and_assign())))) {
         debug->EnableDebugLayer();

         if (device_desc.enable_gpu_based_validation) {
            debug->SetEnableGPUBasedValidation(true);
            debug->SetGPUBasedValidationFlags(
               device_desc.enable_gpu_based_validation_state_tracking
                  ? D3D12_GPU_BASED_VALIDATION_FLAGS_NONE
                  : D3D12_GPU_BASED_VALIDATION_FLAGS_DISABLE_STATE_TRACKING);
         }
      }
   }

   io::output_file debug_ouput{"D3D12 Create Device.log"};

   com_ptr<IDXGIAdapter4> adapter;
   com_ptr<ID3D12Device10> device;

   for (int i = 0; SUCCEEDED(factory.EnumAdapterByGpuPreference(
           i, static_cast<DXGI_GPU_PREFERENCE>(device_desc.gpu_preference),
           IID_PPV_ARGS(adapter.clear_and_assign())));
        ++i) {
      DXGI_ADAPTER_DESC3 adapter_desc{};

      adapter->GetDesc3(&adapter_desc);

      const std::array<char, 129> adapter_name = [&] {
         static_assert(sizeof(DXGI_ADAPTER_DESC3::Description) / 2 == 128);

         std::array<char, 129> name{};

         for (std::size_t i = 0; i < name.size(); ++i) {
            // This is bad but it is just temporary. (so in maybe 10 years it'll get removed)
            name[i] = adapter_desc.Description[i] < 128
                         ? static_cast<char>(adapter_desc.Description[i])
                         : ' ';
         }

         name[128] = '\0';

         return name;
      }();

      debug_ouput.write_ln("Trying {}...", std::string_view{adapter_name.data()});

      if (auto hr = D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_11_0,
                                      IID_PPV_ARGS(device.clear_and_assign()))) {
         debug_ouput.write_ln("DX12 Agility SDK does not appear to be in use.");

         continue;
      }

      bool unsupported = false;

      if (D3D12_FEATURE_DATA_D3D12_OPTIONS options{};
          SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS,
                                                &options, sizeof(options)))) {
         if (options.ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_2) {
            debug_ouput.write_ln(
               "GPU doesn't support D3D12_RESOURCE_BINDING_TIER_2");

            unsupported = true;
         }
      }
      else {
         debug_ouput.write_ln("Failed to query D3D12_FEATURE_D3D12_OPTIONS.");

         unsupported = true;
      }

      if (D3D12_FEATURE_DATA_D3D12_OPTIONS1 options1{};
          SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1,
                                                &options1, sizeof(options1)))) {
         if (not options1.WaveOps) {
            debug_ouput.write_ln("GPU doesn't support WaveOps");

            unsupported = true;
         }
      }
      else {
         debug_ouput.write_ln("Failed to query D3D12_FEATURE_D3D12_OPTIONS1.");

         unsupported = true;
      }

      if (D3D12_FEATURE_DATA_SHADER_MODEL shader_model_support{
             .HighestShaderModel = D3D_SHADER_MODEL_6_1};
          SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shader_model_support,
                                                sizeof(shader_model_support)))) {

         if (shader_model_support.HighestShaderModel < D3D_SHADER_MODEL_6_1) {
            debug_ouput.write_ln("GPU doesn't support D3D_SHADER_MODEL_6_1");

            unsupported = true;
         }
      }
      else {
         debug_ouput.write_ln("GPU doesn't support D3D_SHADER_MODEL_6_1");

         unsupported = true;
      }

      if (unsupported) continue;

      debug_ouput.write_ln("Requirements met. Using GPU.");

      out_adapter = std::move(adapter);

      return device;
   }

   throw exception{error::no_suitable_device,
                   "No D3D12 device was found that met requirements for use. "
                   "See \"D3D12 Create Device.log\" for more info."};
}

auto create_fence(ID3D12Device& device, uint64 initial_value) -> com_ptr<ID3D12Fence>
{
   com_ptr<ID3D12Fence> fence;

   terminate_if_fail(device.CreateFence(initial_value, D3D12_FENCE_FLAG_NONE,
                                        IID_PPV_ARGS(fence.clear_and_assign())));

   return fence;
}

void set_debug_name(ID3D12Object& child, std::string_view name) noexcept
{
   child.SetPrivateData(WKPDID_D3DDebugObjectName,
                        static_cast<uint32>(name.size()), name.data());
}

bool check_enhanced_barriers_support(ID3D12Device& device) noexcept
{
   D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12{};

   if (FAILED(device.CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12,
                                         &options12, sizeof(options12)))) {
      return false;
   }

   return options12.EnhancedBarriersSupported;
}

bool check_shader_barycentrics_support(ID3D12Device& device) noexcept
{
   D3D12_FEATURE_DATA_D3D12_OPTIONS3 options3{};

   if (FAILED(device.CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3,
                                         &options3, sizeof(options3)))) {
      return false;
   }

   return options3.BarycentricsSupported;
}

bool check_conservative_rasterization_support(ID3D12Device& device) noexcept
{
   D3D12_FEATURE_DATA_D3D12_OPTIONS options{};

   if (FAILED(device.CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options,
                                         sizeof(options)))) {
      return false;
   }

   return options.ConservativeRasterizationTier !=
          D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED;
}

bool check_shader_model_6_6_support(ID3D12Device& device) noexcept
{
   D3D12_FEATURE_DATA_SHADER_MODEL shader_model_support{.HighestShaderModel =
                                                           D3D_SHADER_MODEL_6_6};

   if (SUCCEEDED(device.CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shader_model_support,
                                            sizeof(shader_model_support)))) {

      if (shader_model_support.HighestShaderModel < D3D_SHADER_MODEL_6_6) {
         return false;
      }
   }
   else {
      return false;
   }

   return true;
}

bool check_open_existing_heap_support(ID3D12Device& device) noexcept
{
   D3D12_FEATURE_DATA_EXISTING_HEAPS existing_heaps_support{};

   if (FAILED(device.CheckFeatureSupport(D3D12_FEATURE_EXISTING_HEAPS, &existing_heaps_support,
                                         sizeof(existing_heaps_support)))) {
      return false;
   }

   return existing_heaps_support.Supported;
}

bool check_write_buffer_immediate_support(ID3D12Device& device) noexcept
{
   D3D12_FEATURE_DATA_D3D12_OPTIONS3 options3{};

   if (FAILED(device.CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3,
                                         &options3, sizeof(options3)))) {
      return false;
   }

   const D3D12_COMMAND_LIST_SUPPORT_FLAGS desired_flags =
      D3D12_COMMAND_LIST_SUPPORT_FLAG_DIRECT |
      D3D12_COMMAND_LIST_SUPPORT_FLAG_COMPUTE | D3D12_COMMAND_LIST_SUPPORT_FLAG_COPY;

   return (options3.WriteBufferImmediateSupportFlags & desired_flags) == desired_flags;
}

bool check_casting_fully_typed_format_supported(ID3D12Device& device) noexcept
{
   D3D12_FEATURE_DATA_D3D12_OPTIONS3 options3{};

   if (FAILED(device.CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3,
                                         &options3, sizeof(options3)))) {
      return false;
   }

   return options3.CastingFullyTypedFormatSupported;
}

bool check_target_independent_rasterization_supported(IUnknown& adapter) noexcept
{
   return D3D12CreateDevice(&adapter, D3D_FEATURE_LEVEL_11_1, IID_ID3D12Device,
                            nullptr) == S_FALSE;
}

}

struct command_queue_init {
   D3D12_COMMAND_LIST_TYPE type;
   device_state* device_state = nullptr;
   std::string_view debug_name = "";
};

struct device_state {
   device_state(const device_desc& desc)
      : factory{create_dxgi_factory(desc)},
        device{create_d3d12_device(*factory, desc, adapter)},
        supports_enhanced_barriers{desc.force_legacy_barriers
                                      ? false
                                      : check_enhanced_barriers_support(*device)},
        supports_shader_model_6_6{desc.force_no_shader_model_6_6
                                     ? false
                                     : check_shader_model_6_6_support(*device)},
        supports_open_existing_heap{desc.force_no_open_existing_heap
                                       ? false
                                       : check_open_existing_heap_support(*device)},
        supports_write_buffer_immediate{
           desc.force_no_write_buffer_immediate
              ? false
              : check_write_buffer_immediate_support(*device)},
        supports_casting_fully_typed_format{
           desc.force_no_casting_fully_typed_format
              ? false
              : check_casting_fully_typed_format_supported(*device)},
        supports_target_independent_rasterization{
           desc.force_no_target_independent_rasterization
              ? false
              : check_target_independent_rasterization_supported(*adapter)}
   {
   }

   com_ptr<IDXGIFactory7> factory;
   com_ptr<IDXGIAdapter4> adapter;
   com_ptr<ID3D12Device10> device;

   com_ptr<ID3D12Fence> frame_fence =
      create_fence(*device, frame_pipeline_length - 1);
   std::atomic_uint64_t current_frame = frame_pipeline_length;

   descriptor_heap srv_uav_descriptor_heap{*device,
                                           {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                                            .NumDescriptors = num_cbv_srv_uav_descriptors,
                                            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE}};

   descriptor_heap rtv_descriptor_heap{*device,
                                       {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                                        .NumDescriptors = num_rtv_descriptors,
                                        .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE}};

   descriptor_heap dsv_descriptor_heap{*device,
                                       {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
                                        .NumDescriptors = num_dsv_descriptors,
                                        .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE}};

   const bool supports_enhanced_barriers : 1;
   const bool supports_shader_barycentrics : 1 =
      check_shader_barycentrics_support(*device);
   const bool supports_conservative_rasterization : 1 =
      check_conservative_rasterization_support(*device);
   const bool supports_shader_model_6_6 : 1;
   const bool supports_open_existing_heap : 1;
   const bool supports_write_buffer_immediate : 1;
   const bool supports_casting_fully_typed_format : 1;
   const bool supports_target_independent_rasterization : 1;
};

struct swap_chain_state {
   com_ptr<IDXGISwapChain4> swap_chain = nullptr;
   wil::unique_event frame_latency_event = nullptr;

   std::vector<com_ptr<ID3D12Resource2>> buffers;
   std::vector<rtv_handle> buffer_rtvs;

   uint32 width = 0;
   uint32 height = 0;
   uint32 flags = 0;

   device* device = nullptr;
   descriptor_heap* rtv_descriptor_heap = nullptr;
   swap_chain_desc description;
};

struct command_queue_state {
   com_ptr<ID3D12CommandQueue> command_queue;

   std::shared_mutex sync_mutex;
   com_ptr<ID3D12Fence> sync_fence;
   uint64 sync_value = 0;
   uint64 release_last_sync_value = 0;

   std::atomic_bool work_executed_since_release = false;

   release_queue<com_ptr<ID3D12DeviceChild>> release_queue_device_children;
   release_queue<unique_descriptor_releaser> release_queue_descriptors;

   device_state* device = nullptr;
};

struct command_list_state {
   com_ptr<ID3D12GraphicsCommandList7> command_list;

   bool supports_enhanced_barriers = false;
   bool supports_shader_model_6_6 = false;

   uint8 root_param_offset = supports_shader_model_6_6 ? uint8{0} : uint8{1};

   std::vector<D3D12_GLOBAL_BARRIER> deferred_global_barriers = [] {
      std::vector<D3D12_GLOBAL_BARRIER> init;

      init.reserve(128);

      return init;
   }();
   std::vector<D3D12_TEXTURE_BARRIER> deferred_texture_barriers = [] {
      std::vector<D3D12_TEXTURE_BARRIER> init;

      init.reserve(128);

      return init;
   }();
   std::vector<D3D12_BUFFER_BARRIER> deferred_buffer_barriers = [] {
      std::vector<D3D12_BUFFER_BARRIER> init;

      init.reserve(128);

      return init;
   }();
   std::vector<D3D12_RESOURCE_BARRIER> deferred_legacy_barriers = [] {
      std::vector<D3D12_RESOURCE_BARRIER> init;

      init.reserve(128);

      return init;
   }();

   com_ptr<ID3D12CommandAllocator> command_allocator;
   com_ptr<ID3D12DescriptorHeap> descriptor_heap;

   command_allocator_pool allocator_pool;
   device_state* device_state = nullptr;
   device* device = nullptr;
};

device::device(const device_desc& desc)
   : state{desc},
     direct_queue{command_queue_init{.type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     .device_state = &state.get(),
                                     .debug_name = "Direct Queue"}},
     copy_queue{command_queue_init{.type = D3D12_COMMAND_LIST_TYPE_COPY,
                                   .device_state = &state.get(),
                                   .debug_name = "Copy Queue"}},
     async_compute_queue{
        command_queue_init{.type = D3D12_COMMAND_LIST_TYPE_COMPUTE,
                           .device_state = &state.get(),
                           .debug_name = "Async Compute Queue"}},
     background_copy_queue{
        command_queue_init{.type = D3D12_COMMAND_LIST_TYPE_COPY,
                           .device_state = &state.get(),
                           .debug_name = "Background Copy Queue"}}
{
   if (desc.enable_debug_layer) {
      if (com_ptr<ID3D12InfoQueue> info_queue;
          SUCCEEDED(state->device->QueryInterface(info_queue.clear_and_assign()))) {
         info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
         info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
         info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
      }
   }
}

device::~device()
{
   wait_for_idle();
}

auto device::frame_index() -> uint64
{
   return state->current_frame % frame_pipeline_length;
}

void device::new_frame()
{
   // Wait for the oldest frame to complete if needed.
   const uint64 completed_frame =
      (state->current_frame - 1) - (frame_pipeline_length - 1);

   if (const HRESULT hr =
          state->frame_fence->SetEventOnCompletion(completed_frame, nullptr);
       FAILED(hr)) {
      throw_if_fail(state->device->GetDeviceRemovedReason());

      throw_if_fail(hr);
   }
}

void device::end_frame()
{
   // Signal for the current frame.
   direct_queue.state->command_queue->Signal(state->frame_fence.get(),
                                             state->current_frame);
   copy_queue.state->command_queue->Wait(state->frame_fence.get(), state->current_frame);

   // Onto the next frame.
   state->current_frame += 1;

   const uint64 direct_queue_sync_value =
      direct_queue.state->sync_fence->GetCompletedValue();
   const uint64 copy_queue_sync_value =
      copy_queue.state->sync_fence->GetCompletedValue();
   const uint64 async_compute_queue_sync_value =
      async_compute_queue.state->sync_fence->GetCompletedValue();
   const uint64 background_copy_queue_sync_value =
      background_copy_queue.state->sync_fence->GetCompletedValue();

   direct_queue.state->release_queue_device_children.process(direct_queue_sync_value);
   direct_queue.state->release_queue_descriptors.process(direct_queue_sync_value);

   copy_queue.state->release_queue_device_children.process(copy_queue_sync_value);
   copy_queue.state->release_queue_descriptors.process(copy_queue_sync_value);

   async_compute_queue.state->release_queue_device_children.process(
      async_compute_queue_sync_value);
   async_compute_queue.state->release_queue_descriptors.process(
      async_compute_queue_sync_value);

   background_copy_queue.state->release_queue_device_children.process(
      background_copy_queue_sync_value);
   background_copy_queue.state->release_queue_descriptors.process(
      background_copy_queue_sync_value);
}

void device::wait_for_idle()
{
   const auto insert_queue_signal = [&](command_queue& queue) -> uint64 {
      std::scoped_lock lock{queue.state->sync_mutex};

      const uint64 signal_value = ++queue.state->sync_value;

      queue.state->command_queue->Signal(queue.state->sync_fence.get(), signal_value);

      return signal_value;
   };

   const uint64 direct_queue_sync_value = insert_queue_signal(direct_queue);
   const uint64 copy_queue_sync_value = insert_queue_signal(copy_queue);
   const uint64 async_compute_queue_sync_value =
      insert_queue_signal(async_compute_queue);
   const uint64 background_copy_queue_sync_value =
      insert_queue_signal(background_copy_queue);

   std::array fences{direct_queue.state->sync_fence.get(),
                     copy_queue.state->sync_fence.get(),
                     async_compute_queue.state->sync_fence.get(),
                     background_copy_queue.state->sync_fence.get()};

   std::array sync_values{direct_queue_sync_value, copy_queue_sync_value,
                          async_compute_queue_sync_value,
                          background_copy_queue_sync_value};

   static_assert(fences.size() == sync_values.size());

   // Make sure we're not about to wait on a lost device.
   if (FAILED(state->device->GetDeviceRemovedReason())) return;

   throw_if_fail(state->device->SetEventOnMultipleFenceCompletion(
      fences.data(), sync_values.data(), static_cast<uint32>(fences.size()),
      D3D12_MULTIPLE_FENCE_WAIT_FLAG_ALL, nullptr));

   // Release resources we now know the GPU to be done with.

   direct_queue.state->release_queue_device_children.process(direct_queue_sync_value);
   direct_queue.state->release_queue_descriptors.process(direct_queue_sync_value);

   copy_queue.state->release_queue_device_children.process(copy_queue_sync_value);
   copy_queue.state->release_queue_descriptors.process(copy_queue_sync_value);

   async_compute_queue.state->release_queue_device_children.process(
      async_compute_queue_sync_value);
   async_compute_queue.state->release_queue_descriptors.process(
      async_compute_queue_sync_value);

   background_copy_queue.state->release_queue_device_children.process(
      background_copy_queue_sync_value);
   background_copy_queue.state->release_queue_descriptors.process(
      background_copy_queue_sync_value);
}

auto device::create_copy_command_list(const command_list_desc& desc) -> copy_command_list
{
   com_ptr<ID3D12GraphicsCommandList7> d3d12_command_list;

   throw_if_fail(
      state->device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_COPY,
                                        D3D12_COMMAND_LIST_FLAG_NONE,
                                        IID_PPV_ARGS(
                                           d3d12_command_list.clear_and_assign())));

   set_debug_name(*d3d12_command_list, desc.debug_name);

   copy_command_list command_list;

   command_list.state =
      command_list_state{.command_list = std::move(d3d12_command_list),
                         .supports_enhanced_barriers = supports_enhanced_barriers(),
                         .supports_shader_model_6_6 = supports_shader_model_6_6(),
                         .command_allocator = nullptr,
                         .descriptor_heap = nullptr,
                         .device_state = &state.get(),
                         .device = this};

   return command_list;
}

auto device::create_compute_command_list(const command_list_desc& desc) -> compute_command_list
{
   com_ptr<ID3D12GraphicsCommandList7> d3d12_command_list;

   throw_if_fail(
      state->device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_COMPUTE,
                                        D3D12_COMMAND_LIST_FLAG_NONE,
                                        IID_PPV_ARGS(
                                           d3d12_command_list.clear_and_assign())));

   set_debug_name(*d3d12_command_list, desc.debug_name);

   compute_command_list command_list;

   command_list.state =
      command_list_state{.command_list = std::move(d3d12_command_list),
                         .supports_enhanced_barriers = supports_enhanced_barriers(),
                         .supports_shader_model_6_6 = supports_shader_model_6_6(),
                         .command_allocator = nullptr,
                         .descriptor_heap = state->srv_uav_descriptor_heap.heap,
                         .device_state = &state.get(),
                         .device = this};

   return command_list;
}

auto device::create_graphics_command_list(const command_list_desc& desc) -> graphics_command_list
{
   com_ptr<ID3D12GraphicsCommandList7> d3d12_command_list;

   throw_if_fail(
      state->device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                        D3D12_COMMAND_LIST_FLAG_NONE,
                                        IID_PPV_ARGS(
                                           d3d12_command_list.clear_and_assign())));

   set_debug_name(*d3d12_command_list, desc.debug_name);

   graphics_command_list command_list;

   command_list.state =
      command_list_state{.command_list = std::move(d3d12_command_list),
                         .supports_enhanced_barriers = supports_enhanced_barriers(),
                         .supports_shader_model_6_6 = supports_shader_model_6_6(),
                         .command_allocator = nullptr,
                         .descriptor_heap = state->srv_uav_descriptor_heap.heap,
                         .device_state = &state.get(),
                         .device = this};

   return command_list;
}

auto device::create_root_signature(const root_signature_desc& desc) -> root_signature_handle
{
   D3D12_ROOT_SIGNATURE_DESC1 d3d12_desc{.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE};

   if (supports_shader_model_6_6()) {
      d3d12_desc.Flags |= D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
   }

   // clang-format off
   if (desc.flags.allow_input_assembler_input_layout) d3d12_desc.Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
   if (desc.flags.deny_vertex_shader_root_access) d3d12_desc.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
   if (desc.flags.deny_pixel_shader_root_access) d3d12_desc.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
   if (desc.flags.deny_amplification_shader_root_access) d3d12_desc.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
   if (desc.flags.deny_mesh_shader_root_access) d3d12_desc.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;
   // clang-format on

   d3d12_desc.NumParameters = [&]() -> uint32 {
      for (uint32 i = 0; i < desc.parameters.size(); ++i) {
         if (desc.parameters[i].type == root_parameter_type::paramters_end) {
            return i;
         }
      }

      return max_root_parameters;
   }();

   std::array<D3D12_ROOT_PARAMETER1, max_root_parameters + 1> parameters{};
   std::array<D3D12_DESCRIPTOR_RANGE1, 16> bindless_srv_descriptor_ranges{};

   if (not supports_shader_model_6_6()) {
      d3d12_desc.NumParameters += 1;

      for (uint32 i = 0; i < bindless_srv_descriptor_ranges.size(); ++i) {
         bindless_srv_descriptor_ranges[i] = {
            .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
            .NumDescriptors = UINT_MAX,
            .BaseShaderRegister = 0,
            .RegisterSpace = bindless_descriptor_space_start + i,
            .Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE,
            .OffsetInDescriptorsFromTableStart = 0,
         };
      }

      parameters[0] =
         D3D12_ROOT_PARAMETER1{.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                               .DescriptorTable =
                                  {
                                     .NumDescriptorRanges = static_cast<uint32>(
                                        bindless_srv_descriptor_ranges.size()),
                                     .pDescriptorRanges =
                                        bindless_srv_descriptor_ranges.data(),
                                  },
                               .ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL};
   }

   const uint32 param_offset = supports_shader_model_6_6() ? 0 : 1;

   std::array<D3D12_DESCRIPTOR_RANGE1, max_root_parameters> uav_descriptor_ranges_pool{};
   uint32 allocated_uav_descriptor_ranges = 0;

   for (uint32 i = param_offset; i < d3d12_desc.NumParameters; ++i) {
      const root_parameter& param = desc.parameters[i - param_offset];

      switch (param.type) {
      case root_parameter_type::_32bit_constants:
         parameters[i] = {.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
                          .Constants = {.ShaderRegister = param.shader_register,
                                        .RegisterSpace = param.register_space,
                                        .Num32BitValues = param.values_count},
                          .ShaderVisibility =
                             static_cast<D3D12_SHADER_VISIBILITY>(param.visibility)};
         break;
      case root_parameter_type::constant_buffer_view:
         parameters[i] = {.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                          .Descriptor = {.ShaderRegister = param.shader_register,
                                         .RegisterSpace = param.register_space},
                          .ShaderVisibility =
                             static_cast<D3D12_SHADER_VISIBILITY>(param.visibility)};
         break;
      case root_parameter_type::shader_resource_view:
         parameters[i] = {.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV,
                          .Descriptor = {.ShaderRegister = param.shader_register,
                                         .RegisterSpace = param.register_space},
                          .ShaderVisibility =
                             static_cast<D3D12_SHADER_VISIBILITY>(param.visibility)};
         break;
      case root_parameter_type::unordered_access_view:
         parameters[i] = {.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV,
                          .Descriptor = {.ShaderRegister = param.shader_register,
                                         .RegisterSpace = param.register_space},
                          .ShaderVisibility =
                             static_cast<D3D12_SHADER_VISIBILITY>(param.visibility)};
         break;
      case root_parameter_type::unordered_access_view_resource_view: {
         if (allocated_uav_descriptor_ranges >= uav_descriptor_ranges_pool.size()) {
            std::terminate();
         }

         D3D12_DESCRIPTOR_RANGE1* descriptor_range =
            &uav_descriptor_ranges_pool[allocated_uav_descriptor_ranges++];

         *descriptor_range = D3D12_DESCRIPTOR_RANGE1{
            .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
            .NumDescriptors = 1,
            .BaseShaderRegister = param.shader_register,
            .RegisterSpace = param.register_space,
         };

         parameters[i] = {.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                          .DescriptorTable = {.NumDescriptorRanges = 1,
                                              .pDescriptorRanges = descriptor_range},
                          .ShaderVisibility =
                             static_cast<D3D12_SHADER_VISIBILITY>(param.visibility)};
      } break;
      default:
         std::terminate(); // Bad root parameter.
      }
   }

   d3d12_desc.pParameters = parameters.data();

   std::array<D3D12_STATIC_SAMPLER_DESC, 16> static_samplers{};

   if (desc.samplers.size() <= max_root_static_samplers) {
      for (uint32 i = 0; i < desc.samplers.size(); ++i) {
         static_samplers[i] =
            {.Filter = static_cast<D3D12_FILTER>(desc.samplers[i].filter),
             .AddressU =
                static_cast<D3D12_TEXTURE_ADDRESS_MODE>(desc.samplers[i].address_u),
             .AddressV =
                static_cast<D3D12_TEXTURE_ADDRESS_MODE>(desc.samplers[i].address_v),
             .AddressW =
                static_cast<D3D12_TEXTURE_ADDRESS_MODE>(desc.samplers[i].address_w),
             .MipLODBias = desc.samplers[i].mip_lod_bias,
             .MaxAnisotropy = desc.samplers[i].max_anisotropy,
             .ComparisonFunc =
                static_cast<D3D12_COMPARISON_FUNC>(desc.samplers[i].comparison),
             .BorderColor =
                static_cast<D3D12_STATIC_BORDER_COLOR>(desc.samplers[i].border_color),
             .MinLOD = 0.0f,
             .MaxLOD = D3D12_FLOAT32_MAX,

             .ShaderRegister = desc.samplers[i].shader_register,
             .ShaderVisibility =
                static_cast<D3D12_SHADER_VISIBILITY>(desc.samplers[i].visibility)};
      }

      d3d12_desc.pStaticSamplers = static_samplers.data();
      d3d12_desc.NumStaticSamplers = static_cast<uint32>(desc.samplers.size());
   }
   else {
      std::terminate(); // Too many static samplers. (For us, not D3D12).
   }

   D3D12_VERSIONED_ROOT_SIGNATURE_DESC versioned_desc{.Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
                                                      .Desc_1_1 = d3d12_desc};

   com_ptr<ID3DBlob> root_signature_blob;

   if (com_ptr<ID3DBlob> root_signature_error_blob;
       FAILED(D3D12SerializeVersionedRootSignature(
          &versioned_desc, root_signature_blob.clear_and_assign(),
          root_signature_error_blob.clear_and_assign()))) {
      std::string message{static_cast<const char*>(
                             root_signature_error_blob->GetBufferPointer()),
                          root_signature_error_blob->GetBufferSize()};

      OutputDebugStringA(message.c_str());

      if (IsDebuggerPresent()) DebugBreak();

      std::terminate();
   }

   com_ptr<ID3D12RootSignature> root_sig;

   throw_if_fail(
      state->device->CreateRootSignature(0, root_signature_blob->GetBufferPointer(),
                                         root_signature_blob->GetBufferSize(),
                                         IID_PPV_ARGS(root_sig.clear_and_assign())));

   if (not desc.debug_name.empty()) {
      set_debug_name(*root_sig, desc.debug_name);
   }

   return pack_root_signature_handle(root_sig.release());
}

auto device::create_graphics_pipeline(const graphics_pipeline_desc& desc) -> pipeline_handle
{
   D3D12_BLEND_DESC blend_state{.AlphaToCoverageEnable =
                                   desc.blend_state.alpha_to_coverage_enabled,
                                .IndependentBlendEnable =
                                   desc.blend_state.independent_blend_enabled};

   for (std::size_t i = 0; i < 8; ++i) {
      switch (desc.blend_state.render_target[i]) {
      case render_target_blend::disabled:
         blend_state.RenderTarget[i] = {.BlendEnable = false,
                                        .SrcBlend = D3D12_BLEND_ONE,
                                        .DestBlend = D3D12_BLEND_ZERO,
                                        .BlendOp = D3D12_BLEND_OP_ADD,
                                        .SrcBlendAlpha = D3D12_BLEND_ONE,
                                        .DestBlendAlpha = D3D12_BLEND_ZERO,
                                        .BlendOpAlpha = D3D12_BLEND_OP_ADD,
                                        .RenderTargetWriteMask =
                                           D3D12_COLOR_WRITE_ENABLE_ALL};
         break;
      case render_target_blend::premult_alpha_blend:
         blend_state.RenderTarget[i] = {.BlendEnable = true,
                                        .SrcBlend = D3D12_BLEND_ONE,
                                        .DestBlend = D3D12_BLEND_INV_SRC_ALPHA,
                                        .BlendOp = D3D12_BLEND_OP_ADD,
                                        .SrcBlendAlpha = D3D12_BLEND_ONE,
                                        .DestBlendAlpha = D3D12_BLEND_ZERO,
                                        .BlendOpAlpha = D3D12_BLEND_OP_ADD,
                                        .RenderTargetWriteMask =
                                           D3D12_COLOR_WRITE_ENABLE_ALL};
         break;
      case render_target_blend::additive_blend:
         blend_state.RenderTarget[i] = {.BlendEnable = true,
                                        .SrcBlend = D3D12_BLEND_ONE,
                                        .DestBlend = D3D12_BLEND_ONE,
                                        .BlendOp = D3D12_BLEND_OP_ADD,
                                        .SrcBlendAlpha = D3D12_BLEND_ONE,
                                        .DestBlendAlpha = D3D12_BLEND_ZERO,
                                        .BlendOpAlpha = D3D12_BLEND_OP_ADD,
                                        .RenderTargetWriteMask =
                                           D3D12_COLOR_WRITE_ENABLE_ALL};
         break;
      case render_target_blend::alpha_belnd:
         blend_state.RenderTarget[i] = {.BlendEnable = true,
                                        .SrcBlend = D3D12_BLEND_SRC_ALPHA,
                                        .DestBlend = D3D12_BLEND_INV_SRC_ALPHA,
                                        .BlendOp = D3D12_BLEND_OP_ADD,
                                        .SrcBlendAlpha = D3D12_BLEND_ONE,
                                        .DestBlendAlpha = D3D12_BLEND_ZERO,
                                        .BlendOpAlpha = D3D12_BLEND_OP_ADD,
                                        .RenderTargetWriteMask =
                                           D3D12_COLOR_WRITE_ENABLE_ALL};
         break;
      default:
         std::unreachable();
      }
   }

   absl::InlinedVector<D3D12_INPUT_ELEMENT_DESC, 8> input_layout_elements;
   input_layout_elements.reserve(desc.input_layout.size());

   for (auto& element : desc.input_layout) {
      input_layout_elements.push_back(
         D3D12_INPUT_ELEMENT_DESC{.SemanticName = element.semantic_name,
                                  .SemanticIndex = element.semantic_index,
                                  .Format = element.format,
                                  .InputSlot = element.input_slot,
                                  .AlignedByteOffset = element.aligned_byte_offset});
   }

   D3D12_GRAPHICS_PIPELINE_STATE_DESC d3d12_desc{
      .pRootSignature = unpack_root_signature_handle(desc.root_signature),

      .VS = {.pShaderBytecode = desc.vs_bytecode.data(),
             .BytecodeLength = desc.vs_bytecode.size()},
      .PS = {.pShaderBytecode = desc.ps_bytecode.data(),
             .BytecodeLength = desc.ps_bytecode.size()},
      .GS = {.pShaderBytecode = desc.gs_bytecode.data(),
             .BytecodeLength = desc.gs_bytecode.size()},

      .BlendState = blend_state,
      .SampleMask = 0xffffffffu,
      .RasterizerState =
         {
            .FillMode = D3D12_FILL_MODE_SOLID,
            .CullMode = static_cast<D3D12_CULL_MODE>(desc.rasterizer_state.cull_mode),
            .FrontCounterClockwise = true,
            .DepthBias = desc.rasterizer_state.depth_bias,
            .DepthBiasClamp = desc.rasterizer_state.depth_bias_clamp,
            .DepthClipEnable = true,
            .AntialiasedLineEnable = desc.rasterizer_state.antialiased_lines,
            .ForcedSampleCount = desc.rasterizer_state.forced_sample_count,
            .ConservativeRaster = desc.rasterizer_state.conservative_raster
                                     ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON
                                     : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF,
         },
      .DepthStencilState =
         {.DepthEnable = desc.depth_stencil_state.depth_test_enabled,
          .DepthWriteMask = desc.depth_stencil_state.write_depth
                               ? D3D12_DEPTH_WRITE_MASK_ALL
                               : D3D12_DEPTH_WRITE_MASK_ZERO,
          .DepthFunc = static_cast<D3D12_COMPARISON_FUNC>(
             desc.depth_stencil_state.depth_test_func),
          .StencilEnable = desc.depth_stencil_state.stencil_enabled,
          .StencilReadMask = desc.depth_stencil_state.stencil_read_mask,
          .StencilWriteMask = desc.depth_stencil_state.stencil_write_mask,
          .FrontFace = {.StencilFailOp = static_cast<D3D12_STENCIL_OP>(
                           desc.depth_stencil_state.stencil_front_face.fail_op),
                        .StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(
                           desc.depth_stencil_state.stencil_front_face.depth_fail_op),
                        .StencilPassOp = static_cast<D3D12_STENCIL_OP>(
                           desc.depth_stencil_state.stencil_front_face.pass_op),
                        .StencilFunc = static_cast<D3D12_COMPARISON_FUNC>(
                           desc.depth_stencil_state.stencil_front_face.func)},
          .BackFace = {.StencilFailOp = static_cast<D3D12_STENCIL_OP>(
                          desc.depth_stencil_state.stencil_back_face.fail_op),
                       .StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(
                          desc.depth_stencil_state.stencil_back_face.depth_fail_op),
                       .StencilPassOp = static_cast<D3D12_STENCIL_OP>(
                          desc.depth_stencil_state.stencil_back_face.pass_op),
                       .StencilFunc = static_cast<D3D12_COMPARISON_FUNC>(
                          desc.depth_stencil_state.stencil_back_face.func)}},

      .InputLayout = {.pInputElementDescs = not input_layout_elements.empty()
                                               ? input_layout_elements.data()
                                               : nullptr,
                      .NumElements = static_cast<uint32>(input_layout_elements.size())},
      .PrimitiveTopologyType =
         static_cast<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(desc.primitive_type),

      .NumRenderTargets = desc.render_target_count,
      .RTVFormats = {desc.rtv_formats[0], desc.rtv_formats[1], desc.rtv_formats[2],
                     desc.rtv_formats[3], desc.rtv_formats[4], desc.rtv_formats[5],
                     desc.rtv_formats[6], desc.rtv_formats[7]},
      .DSVFormat = desc.dsv_format,
      .SampleDesc = {desc.sample_count, desc.sample_count > 1 ? DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN
                                                              : 0}};

   com_ptr<ID3D12PipelineState> pipeline;

   throw_if_fail(state->device->CreateGraphicsPipelineState(
      &d3d12_desc, IID_PPV_ARGS(pipeline.clear_and_assign())));

   if (not desc.debug_name.empty()) {
      set_debug_name(*pipeline, desc.debug_name);
   }

   return pack_pipeline_handle(pipeline.release());
}

auto device::create_compute_pipeline(const compute_pipeline_desc& desc) -> pipeline_handle
{
   D3D12_COMPUTE_PIPELINE_STATE_DESC d3d12_desc{
      .pRootSignature = unpack_root_signature_handle(desc.root_signature),

      .CS = {.pShaderBytecode = desc.cs_bytecode.data(),
             .BytecodeLength = desc.cs_bytecode.size()},
   };

   com_ptr<ID3D12PipelineState> pipeline;

   throw_if_fail(
      state->device->CreateComputePipelineState(&d3d12_desc,
                                                IID_PPV_ARGS(
                                                   pipeline.clear_and_assign())));

   if (not desc.debug_name.empty()) {
      set_debug_name(*pipeline, desc.debug_name);
   }

   return pack_pipeline_handle(pipeline.release());
}

auto device::create_buffer(const buffer_desc& desc, const heap_type heap_type) -> resource_handle
{
   D3D12_HEAP_PROPERTIES heap_properties{.Type = static_cast<D3D12_HEAP_TYPE>(heap_type)};
   D3D12_RESOURCE_DESC1 d3d12_desc{.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                                   .Alignment = 0,
                                   .Width = desc.size,
                                   .Height = 1,
                                   .DepthOrArraySize = 1,
                                   .MipLevels = 1,
                                   .Format = DXGI_FORMAT_UNKNOWN,
                                   .SampleDesc = {1, 0},
                                   .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
                                   .Flags = desc.flags.allow_unordered_access
                                               ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
                                               : D3D12_RESOURCE_FLAG_NONE};

   com_ptr<ID3D12Resource2> resource;

   throw_if_fail(state->device->CreateCommittedResource3(
      &heap_properties, D3D12_HEAP_FLAG_CREATE_NOT_ZEROED, &d3d12_desc,
      D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, nullptr, 0, nullptr,
      IID_PPV_ARGS(resource.clear_and_assign())));

   set_debug_name(*resource, desc.debug_name);

   return pack_resource_handle(resource.release());
}

auto device::create_texture(const texture_desc& desc,
                            [[maybe_unused]] const barrier_layout initial_resource_layout,
                            [[maybe_unused]] const legacy_resource_state legacy_initial_resource_state)
   -> resource_handle
{
   D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

   // clang-format off
   if (desc.flags.allow_render_target)    flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
   if (desc.flags.allow_depth_stencil)    flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
   if (desc.flags.allow_unordered_access) flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
   if (desc.flags.deny_shader_resource)   flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
   // clang-format on

   const DXGI_FORMAT format = state->supports_casting_fully_typed_format
                                 ? desc.format
                                 : detail::to_typeless(desc.format);

   D3D12_HEAP_PROPERTIES heap_properties{.Type = D3D12_HEAP_TYPE_DEFAULT};
   D3D12_RESOURCE_DESC1 d3d12_desc{
      .Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(desc.dimension),
      .Width = desc.width,
      .Height = desc.height,
      .DepthOrArraySize = static_cast<uint16>(
         desc.dimension == texture_dimension::t_3d ? desc.depth : desc.array_size),
      .MipLevels = static_cast<uint16>(desc.mip_levels),
      .Format = format,
      .SampleDesc = {desc.sample_count, desc.sample_count > 1 ? DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN
                                                              : 0},
      .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
      .Flags = flags};

   D3D12_CLEAR_VALUE clear_value{.Format = desc.optimized_clear_value.format};
   bool has_clear_value = false;

   if (desc.flags.allow_depth_stencil) {
      has_clear_value = true;

      clear_value.DepthStencil = {.Depth =
                                     desc.optimized_clear_value.depth_stencil.depth,
                                  .Stencil =
                                     desc.optimized_clear_value.depth_stencil.stencil};
   }
   else if (desc.flags.allow_render_target) {
      has_clear_value = true;

      clear_value.Color[0] = desc.optimized_clear_value.color.x;
      clear_value.Color[1] = desc.optimized_clear_value.color.y;
      clear_value.Color[2] = desc.optimized_clear_value.color.z;
      clear_value.Color[3] = desc.optimized_clear_value.color.w;
   }

   com_ptr<ID3D12Resource2> resource;

   [[likely]] if (supports_enhanced_barriers()) {
      throw_if_fail(state->device->CreateCommittedResource3(
         &heap_properties, D3D12_HEAP_FLAG_CREATE_NOT_ZEROED, &d3d12_desc,
         static_cast<D3D12_BARRIER_LAYOUT>(initial_resource_layout),
         has_clear_value ? &clear_value : nullptr, nullptr, 0, nullptr,
         IID_PPV_ARGS(resource.clear_and_assign())));
   }
   else {
      throw_if_fail(state->device->CreateCommittedResource2(
         &heap_properties, D3D12_HEAP_FLAG_CREATE_NOT_ZEROED, &d3d12_desc,
         static_cast<D3D12_RESOURCE_STATES>(legacy_initial_resource_state),
         has_clear_value ? &clear_value : nullptr, nullptr,
         IID_PPV_ARGS(resource.clear_and_assign())));
   }

   set_debug_name(*resource, desc.debug_name);

   return pack_resource_handle(resource.release());
}

auto device::open_existing_buffer(const void* address, const buffer_desc& desc)
   -> resource_handle
{
   com_ptr<ID3D12Heap> heap;

   throw_if_fail(
      state->device->OpenExistingHeapFromAddress(address,
                                                 IID_PPV_ARGS(heap.clear_and_assign())));

   D3D12_RESOURCE_DESC1 d3d12_desc{.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                                   .Alignment = 0,
                                   .Width = desc.size,
                                   .Height = 1,
                                   .DepthOrArraySize = 1,
                                   .MipLevels = 1,
                                   .Format = DXGI_FORMAT_UNKNOWN,
                                   .SampleDesc = {1, 0},
                                   .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
                                   .Flags = D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER |
                                            (desc.flags.allow_unordered_access
                                                ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
                                                : D3D12_RESOURCE_FLAG_NONE)};

   com_ptr<ID3D12Resource2> resource;

   throw_if_fail(
      state->device->CreatePlacedResource2(heap.get(), 0, &d3d12_desc,
                                           D3D12_BARRIER_LAYOUT_UNDEFINED,
                                           nullptr, 0, nullptr,
                                           IID_PPV_ARGS(resource.clear_and_assign())));

   set_debug_name(*resource, desc.debug_name);

   return pack_resource_handle(resource.release());
}

auto device::get_gpu_virtual_address(resource_handle resource) -> gpu_virtual_address
{
   return unpack_resource_handle(resource)->GetGPUVirtualAddress();
}

auto device::map(resource_handle resource, uint32 subresource,
                 memory_range read_range) -> void*
{
   D3D12_RANGE d3d12_range{read_range.begin, read_range.end};

   void* data = nullptr;

   throw_if_fail(
      unpack_resource_handle(resource)->Map(subresource, &d3d12_range, &data));

   return data;
}

void device::unmap(resource_handle resource, uint32 subresource, memory_range write_range)
{
   D3D12_RANGE d3d12_range{write_range.begin, write_range.end};

   unpack_resource_handle(resource)->Unmap(subresource, &d3d12_range);
}

auto device::create_shader_resource_view(resource_handle resource,
                                         const shader_resource_view_desc& desc) -> resource_view
{
   ID3D12Resource2* const d3d12_resource = unpack_resource_handle(resource);

   D3D12_RESOURCE_DESC1 resource_desc = d3d12_resource->GetDesc1();

   D3D12_SHADER_RESOURCE_VIEW_DESC d3d12_desc{
      .Format = desc.format,
      .Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(
         std::to_underlying(desc.shader_4_component_mapping.component_0),
         std::to_underlying(desc.shader_4_component_mapping.component_1),
         std::to_underlying(desc.shader_4_component_mapping.component_2),
         std::to_underlying(desc.shader_4_component_mapping.component_3))};

   switch (resource_desc.Dimension) {
   case D3D12_RESOURCE_DIMENSION_UNKNOWN: {
      d3d12_desc.ViewDimension = D3D12_SRV_DIMENSION_UNKNOWN;
   } break;
   case D3D12_RESOURCE_DIMENSION_BUFFER: {
      d3d12_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
      d3d12_desc.Buffer = {.FirstElement = desc.buffer.first_element,
                           .NumElements = desc.buffer.number_elements,
                           .StructureByteStride = desc.buffer.structure_byte_stride,
                           .Flags = desc.buffer.raw_buffer
                                       ? D3D12_BUFFER_SRV_FLAG_RAW
                                       : D3D12_BUFFER_SRV_FLAG_NONE};
   } break;
   case D3D12_RESOURCE_DIMENSION_TEXTURE1D: {
      if (resource_desc.DepthOrArraySize <= 1) {
         d3d12_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
         d3d12_desc.Texture1D = {.MipLevels = d3d12_srv_all_mips};
      }
      else {
         d3d12_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
         d3d12_desc.Texture1DArray = {.MipLevels = d3d12_srv_all_mips,
                                      .ArraySize = resource_desc.DepthOrArraySize};
      }
   } break;
   case D3D12_RESOURCE_DIMENSION_TEXTURE2D: {
      if (resource_desc.SampleDesc.Count <= 1) {
         if (resource_desc.DepthOrArraySize <= 1) {
            d3d12_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            d3d12_desc.Texture2D = {.MipLevels = d3d12_srv_all_mips};
         }
         else {
            if (desc.texture_cube_view) {
               if (resource_desc.DepthOrArraySize <= 6) {
                  d3d12_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                  d3d12_desc.TextureCube = {.MipLevels = d3d12_srv_all_mips};
               }
               else {
                  d3d12_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                  d3d12_desc.TextureCubeArray = {.MipLevels = d3d12_srv_all_mips,
                                                 .First2DArrayFace = 0,
                                                 .NumCubes =
                                                    resource_desc.DepthOrArraySize / 6u};
               }
            }
            else {
               d3d12_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
               d3d12_desc.Texture2DArray = {.MipLevels = d3d12_srv_all_mips,
                                            .ArraySize = resource_desc.DepthOrArraySize};
            }
         }
      }
      else {
         if (resource_desc.DepthOrArraySize <= 1) {
            d3d12_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
            d3d12_desc.Texture2DMS = {};
         }
         else {
            d3d12_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
            d3d12_desc.Texture2DMSArray = {.ArraySize = resource_desc.DepthOrArraySize};
         }
      }
   } break;
   case D3D12_RESOURCE_DIMENSION_TEXTURE3D: {
      d3d12_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
      d3d12_desc.Texture3D = {.MipLevels = d3d12_srv_all_mips};
   } break;
   default:
      std::terminate();
   }

   descriptor_heap& descriptor_heap = state->srv_uav_descriptor_heap;

   resource_view view{.index = descriptor_heap.allocator.allocate()};

   D3D12_CPU_DESCRIPTOR_HANDLE descriptor = descriptor_heap.index_cpu(view.index);

   state->device->CreateShaderResourceView(d3d12_resource, &d3d12_desc, descriptor);

   return view;
}

auto device::create_unordered_access_view(resource_handle resource,
                                          const unordered_access_view_desc& desc)
   -> resource_view
{
   ID3D12Resource2* const d3d12_resource = unpack_resource_handle(resource);

   D3D12_UNORDERED_ACCESS_VIEW_DESC d3d12_desc{};

   d3d12_desc.Format = desc.format;
   d3d12_desc.ViewDimension = static_cast<D3D12_UAV_DIMENSION>(desc.dimension);

   ID3D12Resource2* counter_resource = nullptr;

   switch (desc.dimension) {
   case uav_dimension::buffer: {
      d3d12_desc.Buffer = {.FirstElement = desc.buffer.first_element,
                           .NumElements = desc.buffer.number_elements,
                           .StructureByteStride = desc.buffer.structure_byte_stride,
                           .CounterOffsetInBytes = desc.buffer.counter_offset_in_bytes,
                           .Flags = desc.buffer.raw_buffer
                                       ? D3D12_BUFFER_UAV_FLAG_RAW
                                       : D3D12_BUFFER_UAV_FLAG_NONE};

      if (desc.buffer.counter_resource != null_resource_handle) {
         counter_resource = unpack_resource_handle(desc.buffer.counter_resource);
      }
   } break;
   case uav_dimension::texture1d: {
      d3d12_desc.Texture1D = {.MipSlice = desc.texture1d.mip_slice};
   } break;
   case uav_dimension::texture1d_array: {
      d3d12_desc.Texture1DArray = {.MipSlice = desc.texture1d.mip_slice,
                                   .FirstArraySlice = desc.texture1d_array.first_array_slice,
                                   .ArraySize = desc.texture1d_array.array_size};
   } break;
   case uav_dimension::texture2d: {
      d3d12_desc.Texture2D = {.MipSlice = desc.texture2d.mip_slice};

   } break;
   case uav_dimension::texture2d_array: {
      d3d12_desc.Texture2DArray = {.MipSlice = desc.texture1d.mip_slice,
                                   .FirstArraySlice = desc.texture2d_array.first_array_slice,
                                   .ArraySize = desc.texture2d_array.array_size};

   } break;
   case uav_dimension::texture3d: {
      d3d12_desc.Texture3D = {.MipSlice = desc.texture3d.mip_slice,
                              .FirstWSlice = desc.texture3d.first_w_slice,
                              .WSize = desc.texture3d.w_size};

   } break;
   default:
      std::unreachable();
   }

   descriptor_heap& descriptor_heap = state->srv_uav_descriptor_heap;

   resource_view view{.index = descriptor_heap.allocator.allocate()};

   D3D12_CPU_DESCRIPTOR_HANDLE descriptor = descriptor_heap.index_cpu(view.index);

   state->device->CreateUnorderedAccessView(d3d12_resource, counter_resource,
                                            &d3d12_desc, descriptor);

   return view;
}

auto device::create_render_target_view(resource_handle resource,
                                       const render_target_view_desc& desc) -> rtv_handle
{
   ID3D12Resource2* const d3d12_resource = unpack_resource_handle(resource);

   D3D12_RENDER_TARGET_VIEW_DESC d3d12_desc{};

   d3d12_desc.Format = desc.format;
   d3d12_desc.ViewDimension = static_cast<D3D12_RTV_DIMENSION>(desc.dimension);

   switch (desc.dimension) {
   case rtv_dimension::texture1d: {
      d3d12_desc.Texture1D = {.MipSlice = desc.texture1d.mip_slice};
   } break;
   case rtv_dimension::texture1d_array: {
      d3d12_desc.Texture1DArray = {.MipSlice = desc.texture1d_array.mip_slice,
                                   .FirstArraySlice = desc.texture1d_array.first_array_slice,
                                   .ArraySize = desc.texture1d_array.array_size};
   } break;
   case rtv_dimension::texture2d: {
      d3d12_desc.Texture2D = {.MipSlice = desc.texture2d.mip_slice};

   } break;
   case rtv_dimension::texture2d_array: {
      d3d12_desc.Texture2DArray = {.MipSlice = desc.texture2d_array.mip_slice,
                                   .FirstArraySlice = desc.texture2d_array.first_array_slice,
                                   .ArraySize = desc.texture2d_array.array_size};

   } break;
   case rtv_dimension::texture2d_ms: {
      d3d12_desc.Texture2DMS = {};

   } break;
   case rtv_dimension::texture2d_ms_array: {
      d3d12_desc.Texture2DMSArray = {.FirstArraySlice =
                                        desc.texture2d_ms_array.first_array_slice,
                                     .ArraySize = desc.texture2d_ms_array.array_size};

   } break;
   case rtv_dimension::texture3d: {
      d3d12_desc.Texture3D = {.MipSlice = desc.texture3d.mip_slice,
                              .FirstWSlice = desc.texture3d.first_w_slice,
                              .WSize = desc.texture3d.w_size};

   } break;
   default:
      std::unreachable();
   }

   descriptor_heap& descriptor_heap = state->rtv_descriptor_heap;

   resource_view view{.index = descriptor_heap.allocator.allocate()};

   D3D12_CPU_DESCRIPTOR_HANDLE descriptor = descriptor_heap.index_cpu(view.index);

   state->device->CreateRenderTargetView(d3d12_resource, &d3d12_desc, descriptor);

   return pack_rtv_handle(descriptor);
}

auto device::create_depth_stencil_view(resource_handle resource,
                                       const depth_stencil_view_desc& desc) -> dsv_handle
{
   ID3D12Resource2* const d3d12_resource = unpack_resource_handle(resource);

   D3D12_DEPTH_STENCIL_VIEW_DESC d3d12_desc{};

   d3d12_desc.Format = desc.format;
   d3d12_desc.ViewDimension = static_cast<D3D12_DSV_DIMENSION>(desc.dimension);

   switch (desc.dimension) {
   case dsv_dimension::texture1d: {
      d3d12_desc.Texture1D = {.MipSlice = desc.texture1d.mip_slice};
   } break;
   case dsv_dimension::texture1d_array: {
      d3d12_desc.Texture1DArray = {.MipSlice = desc.texture1d.mip_slice,
                                   .FirstArraySlice = desc.texture1d_array.first_array_slice,
                                   .ArraySize = desc.texture1d_array.array_size};
   } break;
   case dsv_dimension::texture2d: {
      d3d12_desc.Texture2D = {.MipSlice = desc.texture2d.mip_slice};

   } break;
   case dsv_dimension::texture2d_array: {
      d3d12_desc.Texture2DArray = {.MipSlice = desc.texture1d.mip_slice,
                                   .FirstArraySlice = desc.texture2d_array.first_array_slice,
                                   .ArraySize = desc.texture2d_array.array_size};

   } break;
   case dsv_dimension::texture2d_ms: {
      d3d12_desc.Texture2DMS = {};

   } break;
   case dsv_dimension::texture2d_ms_array: {
      d3d12_desc.Texture2DMSArray = {.FirstArraySlice =
                                        desc.texture2d_array.first_array_slice,
                                     .ArraySize = desc.texture2d_array.array_size};

   } break;
   default:
      std::unreachable();
   }

   descriptor_heap& descriptor_heap = state->dsv_descriptor_heap;

   resource_view view{.index = descriptor_heap.allocator.allocate()};

   D3D12_CPU_DESCRIPTOR_HANDLE descriptor = descriptor_heap.index_cpu(view.index);

   state->device->CreateDepthStencilView(d3d12_resource, &d3d12_desc, descriptor);

   return pack_dsv_handle(descriptor);
}

auto device::create_timestamp_query_heap(const uint32 count) -> query_heap_handle
{
   const D3D12_QUERY_HEAP_DESC desc{.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP,
                                    .Count = count};

   com_ptr<ID3D12QueryHeap> query_heap;

   throw_if_fail(
      state->device->CreateQueryHeap(&desc,
                                     IID_PPV_ARGS(query_heap.clear_and_assign())));

   return pack_query_heap_handle(query_heap.release());
}

auto device::create_swap_chain(const swap_chain_desc& desc) -> swap_chain
{
   const HWND window = static_cast<HWND>(desc.window);

   RECT rect{};
   GetClientRect(window, &rect);

   const uint32 width = std::max(static_cast<uint32>(rect.right - rect.left), 1u);
   const uint32 height = std::max(static_cast<uint32>(rect.bottom - rect.top), 1u);

   uint32 flags = 0;

   if (desc.frame_latency_waitable) {
      flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
   }

   if (desc.allow_tearing) {
      flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
   }

   const DXGI_SWAP_CHAIN_DESC1 dxgi_desc{.Width = width,
                                         .Height = height,
                                         .Format = desc.format,
                                         .SampleDesc =
                                            {
                                               .Count = 1,
                                            },
                                         .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                                         .BufferCount = desc.buffer_count,
                                         .Scaling = DXGI_SCALING_NONE,
                                         .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
                                         .Flags = flags};

   com_ptr<IDXGISwapChain1> dxgi_swap_chain_1;
   com_ptr<IDXGISwapChain4> dxgi_swap_chain;

   throw_if_fail(
      state->factory->CreateSwapChainForHwnd(direct_queue.state->command_queue.get(),
                                             window, &dxgi_desc, nullptr, nullptr,
                                             dxgi_swap_chain_1.clear_and_assign()));

   throw_if_fail(dxgi_swap_chain_1->QueryInterface(dxgi_swap_chain.clear_and_assign()));

   state->factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);

   dxgi_swap_chain->SetMaximumFrameLatency(desc.maximum_frame_latency);

   swap_chain swap_chain;

   swap_chain.state =
      swap_chain_state{.swap_chain = dxgi_swap_chain,
                       .frame_latency_event =
                          wil::unique_event{
                             dxgi_swap_chain->GetFrameLatencyWaitableObject()},

                       .width = width,
                       .height = height,
                       .flags = flags,

                       .device = this,
                       .rtv_descriptor_heap = &state->rtv_descriptor_heap,
                       .description = desc};

   swap_chain.state->buffers.resize(desc.buffer_count);
   swap_chain.state->buffer_rtvs.resize(desc.buffer_count);

   for (uint32 i = 0; i < desc.buffer_count; ++i) {
      terminate_if_fail(dxgi_swap_chain->GetBuffer(
         i, IID_PPV_ARGS(swap_chain.state->buffers[i].clear_and_assign())));

      swap_chain.state->buffer_rtvs[i] =
         create_render_target_view(pack_resource_handle(
                                      swap_chain.state->buffers[i].get()),
                                   {.format = desc.format_rtv,
                                    .dimension = rtv_dimension::texture2d});
   }

   return swap_chain;
}

void device::release_root_signature(root_signature_handle root_signature)
{
   unpack_root_signature_handle(root_signature)->Release();
}

void device::release_pipeline(pipeline_handle pipeline)
{
   unpack_pipeline_handle(pipeline)->Release();
}

void device::release_resource(resource_handle resource)
{
   unpack_resource_handle(resource)->Release();
}

void device::release_resource_view(resource_view resource_view)
{
   state->srv_uav_descriptor_heap.allocator.free(resource_view.index);
}

void device::release_render_target_view(rtv_handle render_target_view)
{
   state->rtv_descriptor_heap.allocator.free(
      state->rtv_descriptor_heap.to_index(unpack_rtv_handle(render_target_view)));
}

void device::release_depth_stencil_view(dsv_handle depth_stencil_view)
{
   state->dsv_descriptor_heap.allocator.free(
      state->dsv_descriptor_heap.to_index(unpack_dsv_handle(depth_stencil_view)));
}

void device::release_query_heap(query_heap_handle query_heap)
{
   unpack_query_heap_handle(query_heap)->Release();
}

[[msvc::forceinline]] bool device::supports_enhanced_barriers() const noexcept
{
   return state->supports_enhanced_barriers;
}

[[msvc::forceinline]] bool device::supports_shader_barycentrics() const noexcept
{
   return state->supports_shader_barycentrics;
}

[[msvc::forceinline]] bool device::supports_conservative_rasterization() const noexcept
{
   return state->supports_conservative_rasterization;
}

[[msvc::forceinline]] bool device::supports_shader_model_6_6() const noexcept
{
   return state->supports_shader_model_6_6;
}

[[msvc::forceinline]] bool device::supports_open_existing_heap() const noexcept
{
   return state->supports_open_existing_heap;
}

[[msvc::forceinline]] bool device::supports_write_buffer_immediate() const noexcept
{
   return state->supports_write_buffer_immediate;
}

[[msvc::forceinline]] bool device::supports_target_independent_rasterization() const noexcept
{
   return state->supports_target_independent_rasterization;
}

swap_chain::swap_chain() = default;

swap_chain::~swap_chain()
{
   state->device->wait_for_idle();

   for (auto& view : state->buffer_rtvs) {
      state->rtv_descriptor_heap->allocator.free(
         state->rtv_descriptor_heap->to_index(unpack_rtv_handle(view)));
   }
}

swap_chain::swap_chain(swap_chain&& other) noexcept = default;

auto swap_chain::operator=(swap_chain&& other) noexcept -> swap_chain& = default;

auto swap_chain::waitable_object() noexcept -> void*
{
   return state->frame_latency_event.get();
}

void swap_chain::present(bool allow_tearing)
{
   throw_if_fail(
      state->swap_chain->Present(allow_tearing ? 0 : 1,
                                 allow_tearing ? DXGI_PRESENT_ALLOW_TEARING : 0));
}

void swap_chain::resize(uint32 new_width, uint32 new_height)
{
   if (new_width == state->width and new_height == state->height) return;

   state->device->wait_for_idle();

   state->width = std::max(new_width, 1u);
   state->height = std::max(new_height, 1u);

   for (auto& buffer : state->buffers) buffer = nullptr;

   const swap_chain_desc& desc = state->description;

   IDXGISwapChain4& swap_chain = *state->swap_chain;

   throw_if_fail(swap_chain.ResizeBuffers(desc.buffer_count, state->width,
                                          state->height, desc.format, state->flags));

   for (uint32 i = 0; i < desc.buffer_count; ++i) {
      terminate_if_fail(
         swap_chain.GetBuffer(i, IID_PPV_ARGS(state->buffers[i].clear_and_assign())));

      state->rtv_descriptor_heap->allocator.free(state->rtv_descriptor_heap->to_index(
         unpack_rtv_handle(state->buffer_rtvs[i])));
      state->buffer_rtvs[i] = state->device->create_render_target_view(
         pack_resource_handle(state->buffers[i].get()),
         {.format = desc.format_rtv, .dimension = rtv_dimension::texture2d});
   }
}

auto swap_chain::current_back_buffer() -> current_backbuffer
{
   const uint32 index = state->swap_chain->GetCurrentBackBufferIndex();

   return {.resource = pack_resource_handle(state->buffers[index].get()),
           .rtv = state->buffer_rtvs[index]};
}

auto swap_chain::width() -> uint32
{
   return state->width;
}

auto swap_chain::height() -> uint32
{
   return state->height;
}

command_queue::command_queue() = default;

command_queue::command_queue(const command_queue_init& init)
{
   state->device = init.device_state;

   D3D12_COMMAND_QUEUE_DESC desc{.Type = init.type};

   terminate_if_fail(state->device->device->CreateCommandQueue(
      &desc, IID_PPV_ARGS(state->command_queue.clear_and_assign())));

   terminate_if_fail(
      state->device->device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                         IID_PPV_ARGS(
                                            state->sync_fence.clear_and_assign())));

   if (not init.debug_name.empty()) {
      set_debug_name(*state->command_queue, init.debug_name);
   }
}

command_queue::~command_queue() = default;

void command_queue::execute_command_lists(std::span<command_list*> command_lists)
{
   absl::InlinedVector<ID3D12CommandList*, 32> d3d12_command_lists;

   d3d12_command_lists.reserve(command_lists.size());

   for (auto* list : command_lists) {
      if (not list->state->command_allocator) {
         // Attempt to reuse a command list. This is not compatible with the pooling method used for command allocators.
         std::terminate();
      }

      d3d12_command_lists.push_back(list->state->command_list.get());
   }

   state->command_queue->ExecuteCommandLists(static_cast<uint32>(
                                                d3d12_command_lists.size()),
                                             d3d12_command_lists.data());
   state->work_executed_since_release = true;

   for (command_list* list : command_lists) {
      list->state->allocator_pool.add(std::exchange(list->state->command_allocator, nullptr),
                                      state->device->current_frame.load());
   }
}

void command_queue::sync_with(command_queue& other)
{
   const uint64 sync_value = [&] {
      std::scoped_lock lock{other.state->sync_mutex};

      const uint64 signal_value = ++other.state->sync_value;

      other.state->command_queue->Signal(other.state->sync_fence.get(), signal_value);

      return signal_value;
   }();

   state->command_queue->Wait(other.state->sync_fence.get(), sync_value);
}

void command_queue::wait_for_idle()
{
   const uint64 sync_value = [&] {
      std::scoped_lock lock{state->sync_mutex};

      const uint64 signal_value = ++state->sync_value;

      state->command_queue->Signal(state->sync_fence.get(), signal_value);

      return signal_value;
   }();

   state->sync_fence->SetEventOnCompletion(sync_value, nullptr);

   state->release_queue_device_children.process(sync_value);
   state->release_queue_descriptors.process(sync_value);
}

void command_queue::release_root_signature(root_signature_handle root_signature)
{
   const uint64 sync_value = [&] {
      std::scoped_lock lock{state->sync_mutex};

      if (state->work_executed_since_release.exchange(false)) {
         state->release_last_sync_value = ++state->sync_value;

         state->command_queue->Signal(state->sync_fence.get(),
                                      state->release_last_sync_value);
      }

      return state->release_last_sync_value;
   }();

   state->release_queue_device_children.push(sync_value, com_ptr{unpack_root_signature_handle(
                                                            root_signature)});
}

void command_queue::release_pipeline(pipeline_handle pipeline)
{
   const uint64 sync_value = [&] {
      std::scoped_lock lock{state->sync_mutex};

      if (state->work_executed_since_release.exchange(false)) {
         state->release_last_sync_value = ++state->sync_value;

         state->command_queue->Signal(state->sync_fence.get(),
                                      state->release_last_sync_value);
      }

      return state->release_last_sync_value;
   }();

   state->release_queue_device_children.push(sync_value,
                                             com_ptr{unpack_pipeline_handle(pipeline)});
}

void command_queue::release_resource(resource_handle resource)
{
   const uint64 sync_value = [&] {
      std::scoped_lock lock{state->sync_mutex};

      if (state->work_executed_since_release.exchange(false)) {
         state->release_last_sync_value = ++state->sync_value;

         state->command_queue->Signal(state->sync_fence.get(),
                                      state->release_last_sync_value);
      }

      return state->release_last_sync_value;
   }();

   state->release_queue_device_children.push(sync_value,
                                             com_ptr{unpack_resource_handle(resource)});
}

void command_queue::release_resource_view(resource_view resource_view)
{
   const uint64 sync_value = [&] {
      std::scoped_lock lock{state->sync_mutex};

      if (state->work_executed_since_release.exchange(false)) {
         state->release_last_sync_value = ++state->sync_value;

         state->command_queue->Signal(state->sync_fence.get(),
                                      state->release_last_sync_value);
      }

      return state->release_last_sync_value;
   }();

   state->release_queue_descriptors.push(sync_value,
                                         {resource_view.index,
                                          state->device->srv_uav_descriptor_heap.allocator});
}

void command_queue::release_render_target_view(rtv_handle render_target_view)
{
   const uint64 sync_value = [&] {
      std::scoped_lock lock{state->sync_mutex};

      if (state->work_executed_since_release.exchange(false)) {
         state->release_last_sync_value = ++state->sync_value;

         state->command_queue->Signal(state->sync_fence.get(),
                                      state->release_last_sync_value);
      }

      return state->release_last_sync_value;
   }();

   state->release_queue_descriptors.push(sync_value,
                                         {state->device->rtv_descriptor_heap.to_index(
                                             unpack_rtv_handle(render_target_view)),
                                          state->device->rtv_descriptor_heap.allocator});
}

void command_queue::release_depth_stencil_view(dsv_handle depth_stencil_view)
{
   const uint64 sync_value = [&] {
      std::scoped_lock lock{state->sync_mutex};

      if (state->work_executed_since_release.exchange(false)) {
         state->release_last_sync_value = ++state->sync_value;

         state->command_queue->Signal(state->sync_fence.get(),
                                      state->release_last_sync_value);
      }

      return state->release_last_sync_value;
   }();

   state->release_queue_descriptors.push(sync_value,
                                         {state->device->dsv_descriptor_heap.to_index(
                                             unpack_dsv_handle(depth_stencil_view)),
                                          state->device->dsv_descriptor_heap.allocator});
}

void command_queue::release_query_heap(query_heap_handle query_heap)
{
   const uint64 sync_value = [&] {
      std::scoped_lock lock{state->sync_mutex};

      if (state->work_executed_since_release.exchange(false)) {
         state->release_last_sync_value = ++state->sync_value;

         state->command_queue->Signal(state->sync_fence.get(),
                                      state->release_last_sync_value);
      }

      return state->release_last_sync_value;
   }();

   state->release_queue_device_children.push(sync_value, com_ptr{unpack_query_heap_handle(
                                                            query_heap)});
}

void command_queue::release_command_allocator(command_allocator_handle command_allocator)
{
   const uint64 sync_value = [&] {
      std::scoped_lock lock{state->sync_mutex};

      if (state->work_executed_since_release.exchange(false)) {
         state->release_last_sync_value = ++state->sync_value;

         state->command_queue->Signal(state->sync_fence.get(),
                                      state->release_last_sync_value);
      }

      return state->release_last_sync_value;
   }();

   state->release_queue_device_children.push(sync_value, com_ptr{unpack_command_allocator_handle(
                                                            command_allocator)});
}

auto command_queue::get_timestamp_frequency() -> uint64
{
   uint64 frequency = 0;

   terminate_if_fail(state->command_queue->GetTimestampFrequency(&frequency));

   return frequency;
}

command_list::command_list() = default;

command_list::~command_list()
{
   if (not state->command_list) return;

   switch (state->command_list->GetType()) {
   case D3D12_COMMAND_LIST_TYPE_DIRECT:
      state->allocator_pool.view_and_clear(
         [&](utility::com_ptr<ID3D12CommandAllocator> allocator) {
            state->device->direct_queue.release_command_allocator(
               pack_command_allocator_handle(allocator.release()));
         });
      break;
   case D3D12_COMMAND_LIST_TYPE_COMPUTE:
      state->allocator_pool.view_and_clear(
         [&](utility::com_ptr<ID3D12CommandAllocator> allocator) {
            state->device->async_compute_queue.release_command_allocator(
               pack_command_allocator_handle(allocator.release()));
         });
   case D3D12_COMMAND_LIST_TYPE_COPY:
      state->allocator_pool.view_and_clear(
         [&](utility::com_ptr<ID3D12CommandAllocator> allocator) {
            auto allocator_ref_background = allocator;

            state->device->copy_queue.release_command_allocator(
               pack_command_allocator_handle(allocator.release()));
            state->device->background_copy_queue.release_command_allocator(
               pack_command_allocator_handle(allocator_ref_background.release()));
         });
   default:
      state->device->wait_for_idle();
   }
}

command_list::command_list(command_list&& other) noexcept = default;

auto command_list::operator=(command_list&& other) noexcept -> command_list& = default;

void command_list::close()
{
   throw_if_fail(state->command_list->Close());
}

void command_list::reset_common()
{
   state->command_allocator =
      state->allocator_pool.try_aquire(state->device_state->current_frame.load(),
                                       frame_pipeline_length);

   if (not state->command_allocator) {
      throw_if_fail(state->device_state->device->CreateCommandAllocator(
         state->command_list->GetType(),
         IID_PPV_ARGS(state->command_allocator.clear_and_assign())));
   }

   throw_if_fail(state->command_list->Reset(state->command_allocator.get(), nullptr));
}

void command_list::reset()
{
   reset_common();

   if (state->command_list->GetType() != D3D12_COMMAND_LIST_TYPE_COPY) {
      std::array descriptor_heaps{state->descriptor_heap.get()};

      state->command_list->SetDescriptorHeaps(static_cast<uint32>(
                                                 descriptor_heaps.size()),
                                              descriptor_heaps.data());
   }
}

[[msvc::forceinline]] void command_list::clear_state()
{
   state->command_list->ClearState(nullptr);
}

[[msvc::forceinline]] void copy_command_list::deferred_barrier(
   const std::span<const global_barrier> barriers)
{
   state->deferred_global_barriers.reserve(
      state->deferred_global_barriers.size() + barriers.size());

   for (auto barrier : barriers) deferred_barrier(barrier);
}

[[msvc::forceinline]] void copy_command_list::deferred_barrier(const global_barrier& barrier)
{
   state->deferred_global_barriers.push_back(
      {.SyncBefore = static_cast<D3D12_BARRIER_SYNC>(barrier.sync_before),
       .SyncAfter = static_cast<D3D12_BARRIER_SYNC>(barrier.sync_after),
       .AccessBefore = static_cast<D3D12_BARRIER_ACCESS>(barrier.access_before),
       .AccessAfter = static_cast<D3D12_BARRIER_ACCESS>(barrier.access_after)});
}

[[msvc::forceinline]] void copy_command_list::deferred_barrier(
   const std::span<const texture_barrier> barriers)
{
   state->deferred_texture_barriers.reserve(
      state->deferred_texture_barriers.size() + barriers.size());

   for (auto barrier : barriers) deferred_barrier(barrier);
}

[[msvc::forceinline]] void copy_command_list::deferred_barrier(const texture_barrier& barrier)
{
   static_assert(
      std::is_layout_compatible_v<barrier_subresource_range, D3D12_BARRIER_SUBRESOURCE_RANGE>);

   state->deferred_texture_barriers.push_back(
      {.SyncBefore = static_cast<D3D12_BARRIER_SYNC>(barrier.sync_before),
       .SyncAfter = static_cast<D3D12_BARRIER_SYNC>(barrier.sync_after),
       .AccessBefore = static_cast<D3D12_BARRIER_ACCESS>(barrier.access_before),
       .AccessAfter = static_cast<D3D12_BARRIER_ACCESS>(barrier.access_after),
       .LayoutBefore = static_cast<D3D12_BARRIER_LAYOUT>(barrier.layout_before),
       .LayoutAfter = static_cast<D3D12_BARRIER_LAYOUT>(barrier.layout_after),
       .pResource = unpack_resource_handle(barrier.resource),
       .Subresources = std::bit_cast<D3D12_BARRIER_SUBRESOURCE_RANGE>(barrier.subresources),
       .Flags = static_cast<D3D12_TEXTURE_BARRIER_FLAGS>(barrier.flags)});
}

[[msvc::forceinline]] void copy_command_list::deferred_barrier(
   const std::span<const buffer_barrier> barriers)
{
   state->deferred_buffer_barriers.reserve(
      state->deferred_buffer_barriers.size() + barriers.size());

   for (auto barrier : barriers) deferred_barrier(barrier);
}

[[msvc::forceinline]] void copy_command_list::deferred_barrier(const buffer_barrier& barrier)
{
   state->deferred_buffer_barriers.push_back(
      {.SyncBefore = static_cast<D3D12_BARRIER_SYNC>(barrier.sync_before),
       .SyncAfter = static_cast<D3D12_BARRIER_SYNC>(barrier.sync_after),
       .AccessBefore = static_cast<D3D12_BARRIER_ACCESS>(barrier.access_before),
       .AccessAfter = static_cast<D3D12_BARRIER_ACCESS>(barrier.access_after),
       .pResource = unpack_resource_handle(barrier.resource),
       .Offset = barrier.offset,
       .Size = barrier.size});
}

void copy_command_list::deferred_barrier(
   const std::span<const legacy_resource_transition_barrier> barriers)
{
   state->deferred_legacy_barriers.reserve(
      state->deferred_legacy_barriers.size() + barriers.size());

   for (const auto& barrier : barriers) {
      state->deferred_legacy_barriers.push_back(D3D12_RESOURCE_BARRIER{
         .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
         .Transition = {.pResource = unpack_resource_handle(barrier.resource),
                        .Subresource = barrier.subresource,
                        .StateBefore =
                           static_cast<D3D12_RESOURCE_STATES>(barrier.state_before),
                        .StateAfter = static_cast<D3D12_RESOURCE_STATES>(
                           barrier.state_after)}});
   }
}

void copy_command_list::deferred_barrier(const legacy_resource_transition_barrier& barrier)
{
   deferred_barrier(std::span{&barrier, 1});
}

void copy_command_list::deferred_barrier(
   const std::span<const legacy_resource_aliasing_barrier> barriers)
{
   state->deferred_legacy_barriers.reserve(
      state->deferred_legacy_barriers.size() + barriers.size());

   for (const auto& barrier : barriers) {
      state->deferred_legacy_barriers.push_back(
         D3D12_RESOURCE_BARRIER{.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING,
                                .Aliasing = {.pResourceBefore = unpack_resource_handle(
                                                barrier.resource_before),
                                             .pResourceAfter = unpack_resource_handle(
                                                barrier.resource_after)}});
   }
}

void copy_command_list::deferred_barrier(const legacy_resource_aliasing_barrier& barrier)
{
   deferred_barrier(std::span{&barrier, 1});
}

void copy_command_list::deferred_barrier(const std::span<const legacy_resource_uav_barrier> barriers)
{
   state->deferred_legacy_barriers.reserve(
      state->deferred_legacy_barriers.size() + barriers.size());

   for (const auto& barrier : barriers) {
      state->deferred_legacy_barriers.push_back(
         D3D12_RESOURCE_BARRIER{.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV,
                                .UAV = {.pResource = unpack_resource_handle(
                                           barrier.resource)}});
   }
}

void copy_command_list::deferred_barrier(const legacy_resource_uav_barrier& barrier)
{
   deferred_barrier(std::span{&barrier, 1});
}

void copy_command_list::flush_barriers()
{
   [[likely]] if (state->supports_enhanced_barriers) {
      if (not state->deferred_legacy_barriers.empty()) {
         std::terminate(); // Mixing Enhanced & Legacy Barriers usage.
      }

      absl::InlinedVector<D3D12_BARRIER_GROUP, 4> barriers;

      if (not state->deferred_global_barriers.empty()) {
         barriers.push_back(
            D3D12_BARRIER_GROUP{.Type = D3D12_BARRIER_TYPE_GLOBAL,
                                .NumBarriers = static_cast<uint32>(
                                   state->deferred_global_barriers.size()),
                                .pGlobalBarriers =
                                   state->deferred_global_barriers.data()});
      }

      if (not state->deferred_texture_barriers.empty()) {
         barriers.push_back(
            D3D12_BARRIER_GROUP{.Type = D3D12_BARRIER_TYPE_TEXTURE,
                                .NumBarriers = static_cast<uint32>(
                                   state->deferred_texture_barriers.size()),
                                .pTextureBarriers =
                                   state->deferred_texture_barriers.data()});
      }

      if (not state->deferred_buffer_barriers.empty()) {
         barriers.push_back(
            D3D12_BARRIER_GROUP{.Type = D3D12_BARRIER_TYPE_BUFFER,
                                .NumBarriers = static_cast<uint32>(
                                   state->deferred_buffer_barriers.size()),
                                .pBufferBarriers =
                                   state->deferred_buffer_barriers.data()});
      }

      state->command_list->Barrier(static_cast<uint32>(barriers.size()),
                                   barriers.data());

      state->deferred_global_barriers.clear();
      state->deferred_buffer_barriers.clear();
      state->deferred_texture_barriers.clear();
   }
   else {
      if (not state->deferred_global_barriers.empty() or
          not state->deferred_texture_barriers.empty() or
          not state->deferred_buffer_barriers.empty()) {
         std::terminate(); // Mixing Enhanced & Legacy Barriers usage.
      }

      state->command_list->ResourceBarrier(static_cast<uint32>(
                                              state->deferred_legacy_barriers.size()),
                                           state->deferred_legacy_barriers.data());

      state->deferred_legacy_barriers.clear();
   }
}

[[msvc::forceinline]] void copy_command_list::copy_buffer_region(
   const resource_handle dst_buffer, const uint64 dst_offset,
   const resource_handle src_buffer, const uint64 src_offset, const uint64 num_bytes)
{
   state->command_list->CopyBufferRegion(unpack_resource_handle(dst_buffer), dst_offset,
                                         unpack_resource_handle(src_buffer),
                                         src_offset, num_bytes);
}

[[msvc::forceinline]] void copy_command_list::copy_buffer_to_texture(
   resource_handle dst_texture, const uint32 dst_subresource, const uint32 dst_x,
   const uint32 dst_y, const uint32 dst_z, resource_handle src_buffer,
   const texture_copy_buffer_footprint& src_footprint, const box* const src_box)
{
   D3D12_TEXTURE_COPY_LOCATION dest{.pResource = unpack_resource_handle(dst_texture),
                                    .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
                                    .SubresourceIndex = dst_subresource};
   D3D12_TEXTURE_COPY_LOCATION
   src{.pResource = unpack_resource_handle(src_buffer),
       .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
       .PlacedFootprint = {.Offset = src_footprint.offset,
                           .Footprint = {.Format = src_footprint.format,
                                         .Width = src_footprint.width,
                                         .Height = src_footprint.height,
                                         .Depth = src_footprint.depth,
                                         .RowPitch = src_footprint.row_pitch}}};

   static_assert(std::is_layout_compatible_v<box, D3D12_BOX>);

   state->command_list->CopyTextureRegion(&dest, dst_x, dst_y, dst_z, &src,
                                          reinterpret_cast<const D3D12_BOX*>(src_box));
}

[[msvc::forceinline]] void copy_command_list::copy_texture_to_buffer(
   resource_handle dst_buffer, const texture_copy_buffer_footprint& dst_footprint,
   const uint32 dst_x, const uint32 dst_y, const uint32 dst_z,
   resource_handle src_texture, const uint32 src_subresource, const box* const src_box)
{

   D3D12_TEXTURE_COPY_LOCATION
   dest{.pResource = unpack_resource_handle(dst_buffer),
        .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
        .PlacedFootprint = {.Offset = dst_footprint.offset,
                            .Footprint = {.Format = dst_footprint.format,
                                          .Width = dst_footprint.width,
                                          .Height = dst_footprint.height,
                                          .Depth = dst_footprint.depth,
                                          .RowPitch = dst_footprint.row_pitch}}};
   D3D12_TEXTURE_COPY_LOCATION
   src{.pResource = unpack_resource_handle(src_texture),
       .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
       .SubresourceIndex = src_subresource};

   static_assert(std::is_layout_compatible_v<box, D3D12_BOX>);

   state->command_list->CopyTextureRegion(&dest, dst_x, dst_y, dst_z, &src,
                                          reinterpret_cast<const D3D12_BOX*>(src_box));
}

[[msvc::forceinline]] void copy_command_list::copy_texture_region(
   const resource_handle dst_texture, const uint32 dst_subresource, const uint32 dst_x,
   const uint32 dst_y, const uint32 dst_z, const resource_handle src_texture,
   const uint32 src_subresource, const box* const src_box)
{
   D3D12_TEXTURE_COPY_LOCATION dest{.pResource = unpack_resource_handle(dst_texture),
                                    .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
                                    .SubresourceIndex = dst_subresource};

   D3D12_TEXTURE_COPY_LOCATION
   src{.pResource = unpack_resource_handle(src_texture),
       .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
       .SubresourceIndex = src_subresource};

   static_assert(std::is_layout_compatible_v<box, D3D12_BOX>);

   state->command_list->CopyTextureRegion(&dest, dst_x, dst_y, dst_z, &src,
                                          reinterpret_cast<const D3D12_BOX*>(src_box));
}

[[msvc::forceinline]] void copy_command_list::copy_resource(resource_handle dst_resource,
                                                            resource_handle src_resource)
{
   state->command_list->CopyResource(unpack_resource_handle(dst_resource),
                                     unpack_resource_handle(src_resource));
}

[[msvc::forceinline]] void copy_command_list::write_buffer_immediate(
   const gpu_virtual_address address, const uint32 value)
{
   D3D12_WRITEBUFFERIMMEDIATE_PARAMETER param{.Dest = address, .Value = value};
   D3D12_WRITEBUFFERIMMEDIATE_MODE mode = D3D12_WRITEBUFFERIMMEDIATE_MODE_DEFAULT;

   state->command_list->WriteBufferImmediate(1, &param, &mode);
}

[[msvc::forceinline]] void compute_command_list::dispatch(
   const uint32 thread_group_count_x, const uint32 thread_group_count_y,
   const uint32 thread_group_count_z)
{
   state->command_list->Dispatch(thread_group_count_x, thread_group_count_y,
                                 thread_group_count_z);
}

[[msvc::forceinline]] void compute_command_list::set_pipeline_state(const pipeline_handle pipeline)
{
   state->command_list->SetPipelineState(unpack_pipeline_handle(pipeline));
}

[[msvc::forceinline]] void compute_command_list::set_compute_root_signature(
   const root_signature_handle root_signature)
{
   state->command_list->SetComputeRootSignature(
      unpack_root_signature_handle(root_signature));

   [[unlikely]] if (not state->supports_shader_model_6_6) {
      state->command_list->SetComputeRootDescriptorTable(
         0, state->descriptor_heap->GetGPUDescriptorHandleForHeapStart());
   }
}

[[msvc::forceinline]] void compute_command_list::set_compute_32bit_constant(
   const uint32 parameter_index, const uint32 constant,
   const uint32 dest_offset_in_32bit_values)
{
   state->command_list->SetComputeRoot32BitConstant(parameter_index + state->root_param_offset,
                                                    constant,
                                                    dest_offset_in_32bit_values);
}

[[msvc::forceinline]] void compute_command_list::set_compute_32bit_constants(
   const uint32 parameter_index, const std::span<const std::byte> constants,
   const uint32 dest_offset_in_32bit_values)
{
   assert(constants.size() % 4 == 0);

   state->command_list->SetComputeRoot32BitConstants(
      parameter_index + state->root_param_offset,
      static_cast<uint32>(constants.size() / sizeof(uint32)), constants.data(),
      dest_offset_in_32bit_values);
}

[[msvc::forceinline]] void compute_command_list::set_compute_cbv(
   const uint32 parameter_index, const gpu_virtual_address buffer_location)
{
   state->command_list->SetComputeRootConstantBufferView(parameter_index + state->root_param_offset,
                                                         buffer_location);
}

[[msvc::forceinline]] void compute_command_list::set_compute_srv(
   const uint32 parameter_index, const gpu_virtual_address buffer_location)
{
   state->command_list->SetComputeRootShaderResourceView(parameter_index + state->root_param_offset,
                                                         buffer_location);
}

[[msvc::forceinline]] void compute_command_list::set_compute_uav(
   const uint32 parameter_index, const gpu_virtual_address buffer_location)
{
   state->command_list->SetComputeRootUnorderedAccessView(parameter_index +
                                                             state->root_param_offset,
                                                          buffer_location);
}

[[msvc::forceinline]] void compute_command_list::set_compute_uav_resource_view(
   const uint32 parameter_index, const resource_view view)
{
   assert(view != invalid_resource_view);

   state->command_list->SetComputeRootDescriptorTable(
      parameter_index + state->root_param_offset,
      state->device_state->srv_uav_descriptor_heap.index_gpu(view.index));
}

[[msvc::forceinline]] void compute_command_list::discard_resource(const resource_handle resource)
{
   state->command_list->DiscardResource(unpack_resource_handle(resource), nullptr);
}

[[msvc::forceinline]] void compute_command_list::query_timestamp(
   const query_heap_handle query_heap, const uint32 query_index)
{
   state->command_list->EndQuery(unpack_query_heap_handle(query_heap),
                                 D3D12_QUERY_TYPE_TIMESTAMP, query_index);
}

[[msvc::forceinline]] void compute_command_list::resolve_query_timestamp(
   const query_heap_handle query_heap, const uint32 start_query,
   const uint32 query_count, const resource_handle destination_buffer,
   const uint32 aligned_destination_buffer_offset)
{
   state->command_list->ResolveQueryData(unpack_query_heap_handle(query_heap),
                                         D3D12_QUERY_TYPE_TIMESTAMP,
                                         start_query, query_count,
                                         unpack_resource_handle(destination_buffer),
                                         aligned_destination_buffer_offset);
}

[[msvc::forceinline]] void graphics_command_list::draw_instanced(
   const uint32 vertex_count_per_instance, const uint32 instance_count,
   const uint32 start_vertex_location, const uint32 start_instance_location)
{
   state->command_list->DrawInstanced(vertex_count_per_instance, instance_count,
                                      start_vertex_location, start_instance_location);
}

[[msvc::forceinline]] void graphics_command_list::draw_indexed_instanced(
   const uint32 index_count_per_instance, const uint32 instance_count,
   const uint32 start_index_location, const int32 base_vertex_location,
   const uint32 start_instance_location)
{
   state->command_list->DrawIndexedInstanced(index_count_per_instance,
                                             instance_count, start_index_location,
                                             base_vertex_location,
                                             start_instance_location);
}

[[msvc::forceinline]] void graphics_command_list::resolve_subresource(
   resource_handle dst_resource, const uint32 dst_subresource,
   resource_handle src_resource, const uint32 src_subresource, const DXGI_FORMAT format)
{
   state->command_list->ResolveSubresource(unpack_resource_handle(dst_resource),
                                           dst_subresource,
                                           unpack_resource_handle(src_resource),
                                           src_subresource, format);
}

[[msvc::forceinline]] void graphics_command_list::set_graphics_root_signature(
   const root_signature_handle root_signature)
{
   state->command_list->SetGraphicsRootSignature(
      unpack_root_signature_handle(root_signature));

   [[unlikely]] if (not state->supports_shader_model_6_6) {
      state->command_list->SetGraphicsRootDescriptorTable(
         0, state->descriptor_heap->GetGPUDescriptorHandleForHeapStart());
   }
}

[[msvc::forceinline]] void graphics_command_list::set_graphics_32bit_constant(
   const uint32 parameter_index, const uint32 constant,
   const uint32 dest_offset_in_32bit_values)
{
   state->command_list->SetGraphicsRoot32BitConstant(parameter_index + state->root_param_offset,
                                                     constant,
                                                     dest_offset_in_32bit_values);
}

[[msvc::forceinline]] void graphics_command_list::set_graphics_32bit_constants(
   const uint32 parameter_index, const std::span<const std::byte> constants,
   const uint32 dest_offset_in_32bit_values)
{
   assert(constants.size() % 4 == 0);

   state->command_list->SetGraphicsRoot32BitConstants(
      parameter_index + state->root_param_offset,
      static_cast<uint32>(constants.size() / sizeof(uint32)), constants.data(),
      dest_offset_in_32bit_values);
}

[[msvc::forceinline]] void graphics_command_list::set_graphics_cbv(
   const uint32 parameter_index, const gpu_virtual_address buffer_location)
{
   state->command_list->SetGraphicsRootConstantBufferView(parameter_index +
                                                             state->root_param_offset,
                                                          buffer_location);
}

[[msvc::forceinline]] void graphics_command_list::set_graphics_srv(
   const uint32 parameter_index, const gpu_virtual_address buffer_location)
{
   state->command_list->SetGraphicsRootShaderResourceView(parameter_index +
                                                             state->root_param_offset,
                                                          buffer_location);
}

[[msvc::forceinline]] void graphics_command_list::set_graphics_uav(
   const uint32 parameter_index, const gpu_virtual_address buffer_location)
{
   state->command_list->SetGraphicsRootUnorderedAccessView(parameter_index +
                                                              state->root_param_offset,
                                                           buffer_location);
}

[[msvc::forceinline]] void graphics_command_list::set_graphics_uav_resource_view(
   const uint32 parameter_index, const resource_view view)
{
   assert(view != invalid_resource_view);

   state->command_list->SetGraphicsRootDescriptorTable(
      parameter_index + state->root_param_offset,
      state->device_state->srv_uav_descriptor_heap.index_gpu(view.index));
}

[[msvc::forceinline]] void graphics_command_list::ia_set_primitive_topology(
   const primitive_topology primitive_topology)
{
   state->command_list->IASetPrimitiveTopology(
      static_cast<D3D12_PRIMITIVE_TOPOLOGY>(primitive_topology));
}

[[msvc::forceinline]] void graphics_command_list::ia_set_index_buffer(const index_buffer_view& view)
{
   D3D12_INDEX_BUFFER_VIEW ibv{.BufferLocation = view.buffer_location,
                               .SizeInBytes = view.size_in_bytes,
                               .Format = view.format};

   state->command_list->IASetIndexBuffer(&ibv);
}

[[msvc::forceinline]] void graphics_command_list::ia_set_vertex_buffers(
   const uint32 start_slot, const std::span<const vertex_buffer_view> views)
{
   static_assert(
      std::is_layout_compatible_v<D3D12_VERTEX_BUFFER_VIEW, vertex_buffer_view>);

   state->command_list->IASetVertexBuffers(start_slot,
                                           static_cast<uint32>(views.size()),
                                           reinterpret_cast<const D3D12_VERTEX_BUFFER_VIEW*>(
                                              views.data()));
}

[[msvc::forceinline]] void graphics_command_list::ia_set_vertex_buffers(
   const uint32 start_slot, const vertex_buffer_view& view)
{
   static_assert(
      std::is_layout_compatible_v<D3D12_VERTEX_BUFFER_VIEW, vertex_buffer_view>);

   state->command_list->IASetVertexBuffers(start_slot, 1,
                                           reinterpret_cast<const D3D12_VERTEX_BUFFER_VIEW*>(
                                              &view));
}

[[msvc::forceinline]] void graphics_command_list::rs_set_viewports(
   const std::span<const viewport> viewports)
{
   static_assert(std::is_layout_compatible_v<D3D12_VIEWPORT, viewport>);

   state->command_list->RSSetViewports(static_cast<uint32>(viewports.size()),
                                       reinterpret_cast<const D3D12_VIEWPORT*>(
                                          viewports.data()));
}

[[msvc::forceinline]] void graphics_command_list::rs_set_viewports(const viewport& viewport)
{
   static_assert(std::is_layout_compatible_v<D3D12_VIEWPORT, gpu::viewport>);

   state->command_list->RSSetViewports(1, reinterpret_cast<const D3D12_VIEWPORT*>(
                                             &viewport));
}

[[msvc::forceinline]] void graphics_command_list::rs_set_scissor_rects(
   const std::span<const rect> scissor_rects)
{
   absl::InlinedVector<RECT, 16> win32_rects;
   win32_rects.resize(scissor_rects.size());

   for (std::size_t i = 0; i < scissor_rects.size(); ++i) {
      win32_rects[i] = std::bit_cast<RECT>(scissor_rects[i]);
   }

   state->command_list->RSSetScissorRects(static_cast<uint32>(win32_rects.size()),
                                          win32_rects.data());
}

[[msvc::forceinline]] void graphics_command_list::rs_set_scissor_rects(const rect& scissor_rect)
{
   RECT win32_rect = std::bit_cast<RECT>(scissor_rect);

   state->command_list->RSSetScissorRects(1, &win32_rect);
}

[[msvc::forceinline]] void graphics_command_list::om_set_blend_factor(const float4& blend_factor)
{
   state->command_list->OMSetBlendFactor(&blend_factor.x);
}

[[msvc::forceinline]] void graphics_command_list::om_set_stencil_ref(const uint32 stencil_ref)
{
   state->command_list->OMSetStencilRef(stencil_ref);
}

[[msvc::forceinline]] void graphics_command_list::om_set_render_targets(
   const std::span<const rtv_handle> render_targets)
{
   std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 8> descriptors;

   const uint32 render_target_count =
      static_cast<uint32>(std::min(render_targets.size(), descriptors.size()));

   for (uint32 i = 0; i < render_target_count; ++i) {
      descriptors[i] = unpack_rtv_handle(render_targets[i]);
   }

   state->command_list->OMSetRenderTargets(render_target_count,
                                           descriptors.data(), false, nullptr);
}

[[msvc::forceinline]] void graphics_command_list::om_set_render_targets(
   const std::span<const rtv_handle> render_targets, const dsv_handle depth_stencil)
{
   std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 8> descriptors;

   const uint32 render_target_count =
      static_cast<uint32>(std::min(render_targets.size(), descriptors.size()));

   for (uint32 i = 0; i < render_target_count; ++i) {
      descriptors[i] = unpack_rtv_handle(render_targets[i]);
   }

   D3D12_CPU_DESCRIPTOR_HANDLE dsv_descriptor = unpack_dsv_handle(depth_stencil);

   state->command_list->OMSetRenderTargets(render_target_count, descriptors.data(),
                                           false, &dsv_descriptor);
}

[[msvc::forceinline]] void graphics_command_list::om_set_render_targets(const rtv_handle render_target)
{
   D3D12_CPU_DESCRIPTOR_HANDLE rtv_descriptor = unpack_rtv_handle(render_target);

   state->command_list->OMSetRenderTargets(1, &rtv_descriptor, false, nullptr);
}

[[msvc::forceinline]] void graphics_command_list::om_set_render_targets(
   const rtv_handle render_target, const dsv_handle depth_stencil)
{
   D3D12_CPU_DESCRIPTOR_HANDLE rtv_descriptor = unpack_rtv_handle(render_target);
   D3D12_CPU_DESCRIPTOR_HANDLE dsv_descriptor = unpack_dsv_handle(depth_stencil);

   state->command_list->OMSetRenderTargets(1, &rtv_descriptor, false, &dsv_descriptor);
}

[[msvc::forceinline]] void graphics_command_list::om_set_render_targets(const dsv_handle depth_stencil)
{
   D3D12_CPU_DESCRIPTOR_HANDLE dsv_descriptor = unpack_dsv_handle(depth_stencil);

   state->command_list->OMSetRenderTargets(0, nullptr, false, &dsv_descriptor);
}

[[msvc::forceinline]] void graphics_command_list::clear_depth_stencil_view(
   const dsv_handle depth_stencil, const depth_stencil_clear_flags flags,
   const float depth, const uint8 stencil, const rect* const rect)
{
   D3D12_CLEAR_FLAGS d3d12_flags{};

   if (flags.clear_depth) d3d12_flags |= D3D12_CLEAR_FLAG_DEPTH;
   if (flags.clear_stencil) d3d12_flags |= D3D12_CLEAR_FLAG_STENCIL;

   RECT win32_rect{};

   if (rect) win32_rect = std::bit_cast<RECT>(*rect);

   state->command_list->ClearDepthStencilView(unpack_dsv_handle(depth_stencil),
                                              d3d12_flags, depth, stencil,
                                              rect ? 1 : 0,
                                              rect ? &win32_rect : nullptr);
}

[[msvc::forceinline]] void graphics_command_list::clear_render_target_view(
   const rtv_handle rendertarget, const float4& color, const rect* const rect)
{
   RECT win32_rect{};

   if (rect) win32_rect = std::bit_cast<RECT>(*rect);

   state->command_list->ClearRenderTargetView(unpack_rtv_handle(rendertarget),
                                              &color.x, rect ? 1 : 0,
                                              rect ? &win32_rect : nullptr);
}
}
