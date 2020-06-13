#pragma once

#include "async_copy_manager.hpp"
#include "buffer.hpp"
#include "common.hpp"
#include "concepts.hpp"
#include "descriptor_allocation.hpp"
#include "descriptor_heap.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "swap_chain.hpp"
#include "texture.hpp"
#include "utility/com_ptr.hpp"

#include <array>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <vector>

#include <boost/variant2/variant.hpp>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wil/resource.h>

namespace sk::graphics::gpu {

class device;

using command_allocators =
   std::array<utility::com_ptr<ID3D12CommandAllocator>, render_latency>;

class device {
public:
   explicit device(const HWND window);

   device(const device&) = delete;
   device operator=(const device&) = delete;

   device(device&&) = delete;
   device operator=(device&&) = delete;

   ~device();

   void wait_for_idle();

   void end_frame();

   auto create_command_allocators(const D3D12_COMMAND_LIST_TYPE type)
      -> command_allocators;

   auto create_command_list(const D3D12_COMMAND_LIST_TYPE type)
      -> utility::com_ptr<ID3D12GraphicsCommandList5>;

   auto create_buffer(const buffer_desc& desc, const D3D12_HEAP_TYPE heap_type,
                      const D3D12_RESOURCE_STATES initial_resource_state) -> buffer
   {
      const D3D12_HEAP_PROPERTIES heap_properties{.Type = heap_type,
                                                  .CPUPageProperty =
                                                     D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                                                  .MemoryPoolPreference =
                                                     D3D12_MEMORY_POOL_UNKNOWN};

      const D3D12_RESOURCE_DESC d3d12_desc = desc;

      utility::com_ptr<ID3D12Resource> buffer_resource;

      throw_if_failed(device_d3d->CreateCommittedResource(
         &heap_properties, D3D12_HEAP_FLAG_NONE, &d3d12_desc, initial_resource_state,
         nullptr, IID_PPV_ARGS(buffer_resource.clear_and_assign())));

      return buffer{*this, desc.size, std::move(buffer_resource)};
   }

   auto create_texture(const texture_desc& desc,
                       const D3D12_RESOURCE_STATES initial_resource_state) -> texture
   {
      const D3D12_HEAP_PROPERTIES heap_properties{.Type = D3D12_HEAP_TYPE_DEFAULT,
                                                  .CPUPageProperty =
                                                     D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                                                  .MemoryPoolPreference =
                                                     D3D12_MEMORY_POOL_UNKNOWN};

      const D3D12_RESOURCE_DESC d3d12_desc = desc;

      utility::com_ptr<ID3D12Resource> texture_resource;

      throw_if_failed(device_d3d->CreateCommittedResource(
         &heap_properties, D3D12_HEAP_FLAG_NONE, &d3d12_desc, initial_resource_state,
         desc.optimized_clear_value ? &desc.optimized_clear_value.value() : nullptr,
         IID_PPV_ARGS(texture_resource.clear_and_assign())));

      return texture{*this, desc, std::move(texture_resource)};
   }

   void create_shader_resource_view(ID3D12Resource& resource,
                                    const std::optional<shader_resource_view_desc> desc,
                                    descriptor_handle dest_descriptor) noexcept
   {
      if (desc) {
         const D3D12_SHADER_RESOURCE_VIEW_DESC d3d12_desc = *desc;

         device_d3d->CreateShaderResourceView(&resource, &d3d12_desc,
                                              dest_descriptor.cpu);
      }
      else {
         device_d3d->CreateShaderResourceView(&resource, nullptr,
                                              dest_descriptor.cpu);
      }
   }

   void create_constant_buffer_view(const constant_buffer_view& desc,
                                    descriptor_handle dest_descriptor) noexcept
   {
      const D3D12_CONSTANT_BUFFER_VIEW_DESC d3d12_desc = desc;

      device_d3d->CreateConstantBufferView(&d3d12_desc, dest_descriptor.cpu);
   }

   void create_unordered_access_view(ID3D12Resource& resource,
                                     ID3D12Resource* counter_resource,
                                     const std::optional<unordered_access_view_desc> desc,
                                     descriptor_handle dest_descriptor) noexcept
   {
      if (desc) {
         const D3D12_UNORDERED_ACCESS_VIEW_DESC d3d12_desc = *desc;

         device_d3d->CreateUnorderedAccessView(&resource, counter_resource,
                                               &d3d12_desc, dest_descriptor.cpu);
      }
      else {
         device_d3d->CreateUnorderedAccessView(&resource, counter_resource,
                                               nullptr, dest_descriptor.cpu);
      }
   }

   auto allocate_descriptors(const D3D12_DESCRIPTOR_HEAP_TYPE type,
                             const uint32 count) -> descriptor_allocation
   {
      switch (type) {
      case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
         return {*this, type, descriptor_heap_srv_cbv_uav.allocate_static(count)};
      case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
         return {*this, type, descriptor_heap_rtv.allocate_static(count)};
      case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
         return {*this, type, descriptor_heap_dsv.allocate_static(count)};
      default:
         throw std::runtime_error{
            "attempt to allocate from unknown or unsupported descriptor heap"};
      }
   }

   template<resource_owner Owner>
   void deferred_destroy_resource(Owner& resource_owner)
   {
      std::lock_guard lock{_deferred_destruction_mutex};

      _deferred_destructions.push_back(
         {// TODO (maybe, depending on how memory residency management shapes up): frame usage tracking for resources
          .last_used_frame = fence_value,
          .resource = utility::com_ptr{resource_owner.resource()}});
   }

   void deferred_free_descriptors(descriptor_allocation& allocation)
   {
      std::lock_guard lock{_deferred_destruction_mutex};

      switch (allocation.type()) {
      case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
         _deferred_destructions.push_back(
            {.last_used_frame = fence_value,
             .resource =
                descriptor_range_owner{descriptor_heap_srv_cbv_uav, allocation}});
         return;
      case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
         _deferred_destructions.push_back(
            {.last_used_frame = fence_value,
             .resource = descriptor_range_owner{descriptor_heap_rtv, allocation}});
         return;
      case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
         _deferred_destructions.push_back(
            {.last_used_frame = fence_value,
             .resource = descriptor_range_owner{descriptor_heap_dsv, allocation}});
         return;
      default:
         throw std::runtime_error{
            "attempt to allocate free unknown or unsupported descriptor heap"};
      }
   }

   constexpr static int descriptor_heap_rtv_size = 128;
   constexpr static int descriptor_heap_dsv_size = 32;
   constexpr static int descriptor_heap_cbv_srv_uav_size = 16 * 8192;

   utility::com_ptr<IDXGIFactory7> factory;
   utility::com_ptr<IDXGIAdapter4> adapter;
   utility::com_ptr<ID3D12Device6> device_d3d;
   utility::com_ptr<ID3D12Fence> fence;
   UINT64 fence_value = 1;
   UINT64 previous_frame_fence_value = 0;
   UINT64 completed_fence_value = 0;
   UINT64 frame_index = 0;
   wil::unique_event fence_event{CreateEventW(nullptr, false, false, nullptr)};
   utility::com_ptr<ID3D12CommandQueue> command_queue;

   descriptor_heap descriptor_heap_srv_cbv_uav{D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                                               D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
                                               descriptor_heap_cbv_srv_uav_size,
                                               *device_d3d};
   descriptor_heap descriptor_heap_rtv{D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                                       D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
                                       descriptor_heap_rtv_size, *device_d3d};
   descriptor_heap descriptor_heap_dsv{D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
                                       D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
                                       descriptor_heap_dsv_size, *device_d3d};

   async_copy_manager copy_manager{*device_d3d};

   swap_chain swap_chain;

   root_signature_library root_signatures{*device_d3d};
   pipeline_library pipelines{*device_d3d, root_signatures};

private:
   void process_deferred_resource_destructions();

   struct descriptor_range_owner {
      descriptor_range_owner(descriptor_heap& heap, descriptor_range range)
         : heap{&heap}, range{range}
      {
      }

      ~descriptor_range_owner()
      {
         heap->free_static(range);
      }

      gsl::not_null<descriptor_heap*> heap;
      descriptor_range range;
   };

   struct deferred_destruction {
      UINT64 last_used_frame = 0;

      boost::variant2::variant<utility::com_ptr<ID3D12Resource>, descriptor_range_owner> resource;
   };

   std::mutex _deferred_destruction_mutex;
   std::vector<deferred_destruction> _deferred_destructions;
};

}