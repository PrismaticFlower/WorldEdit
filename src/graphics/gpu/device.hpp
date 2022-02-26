#pragma once

#include "async_copy_manager.hpp"
#include "buffer.hpp"
#include "command_allocator.hpp"
#include "command_list.hpp"
#include "command_queue.hpp"
#include "common.hpp"
#include "d3d12_mem_alloc.hpp"
#include "descriptor_allocation.hpp"
#include "descriptor_heap.hpp"
#include "device.hpp"
#include "resource.hpp"
#include "resource_view_set.hpp"
#include "swap_chain.hpp"
#include "texture.hpp"
#include "utility/com_ptr.hpp"

#include <array>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

#include <boost/variant2/variant.hpp>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wil/resource.h>

#include <range/v3/view.hpp>

namespace we::graphics::gpu {

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

   auto create_copy_command_list(const std::string_view debug_name = "")
      -> copy_command_list;

   auto create_compute_command_list(const std::string_view debug_name = "")
      -> compute_command_list;

   auto create_graphics_command_list(const std::string_view debug_name = "")
      -> graphics_command_list;

   auto create_root_signature(const root_signature_desc& desc)
      -> utility::com_ptr<ID3D12RootSignature>;

   auto create_buffer(const buffer_desc& desc, const D3D12_HEAP_TYPE heap_type,
                      const D3D12_RESOURCE_STATES initial_resource_state) -> buffer
   {
      return buffer{create_resource(desc, heap_type, initial_resource_state),
                    desc.size};
   }

   auto create_texture(const texture_desc& desc,
                       const D3D12_RESOURCE_STATES initial_resource_state) -> texture
   {
      return texture{create_resource(desc, D3D12_HEAP_TYPE_DEFAULT, initial_resource_state,
                                     desc.optimized_clear_value),
                     desc};
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

   auto create_resource_view_set(const std::span<const resource_view_desc> view_descriptions) noexcept
      -> resource_view_set
   {
      descriptor_range descriptors = descriptor_heap_srv_cbv_uav.allocate_static(
         static_cast<uint32>(view_descriptions.size()));
      std::vector<std::shared_ptr<ID3D12Resource>> resources;
      resources.reserve(view_descriptions.size());

      for (auto [index, desc] : view_descriptions | ranges::views::enumerate) {
         resources.push_back(desc.resource);

         if (desc.counter_resource) resources.push_back(desc.counter_resource);

         boost::variant2::visit(
            [&](const auto& view_desc) {
               using Type = std::remove_cvref_t<decltype(view_desc)>;

               if constexpr (std::is_same_v<shader_resource_view_desc, Type>) {
                  create_shader_resource_view(*desc.resource, view_desc,
                                              descriptors[index]);
               }
               else if constexpr (std::is_same_v<constant_buffer_view, Type>) {
                  create_constant_buffer_view(view_desc, descriptors[index]);
               }
               else if constexpr (std::is_same_v<unordered_access_view_desc, Type>) {
                  create_unordered_access_view(*desc.resource,
                                               desc.counter_resource.get(),
                                               view_desc, descriptors[index]);
               }
            },
            desc.view_desc);
      }

      return resource_view_set{*this, descriptors, std::move(resources)};
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

   void deferred_destroy_resource(utility::com_ptr<ID3D12Resource> resource,
                                  release_ptr<D3D12MA::Allocation> allocation)
   {
      assert(resource);

      std::lock_guard lock{_deferred_destruction_mutex};

      _deferred_destructions.push_back(
         {.last_used_frame = fence_value,
          .resource = resource_owner{.allocation = std::move(allocation),
                                     .resource = std::move(resource)}});
   }

   void deferred_free_descriptors(const D3D12_DESCRIPTOR_HEAP_TYPE type,
                                  descriptor_range descriptors)
   {
      std::lock_guard lock{_deferred_destruction_mutex};

      switch (type) {
      case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
         _deferred_destructions.push_back(
            {.last_used_frame = fence_value,
             .resource = std::make_unique<descriptor_range_owner>(descriptor_heap_srv_cbv_uav,
                                                                  descriptors)});
         return;
      case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
         _deferred_destructions.push_back(
            {.last_used_frame = fence_value,
             .resource = std::make_unique<descriptor_range_owner>(descriptor_heap_rtv,
                                                                  descriptors)});
         return;
      case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
         _deferred_destructions.push_back(
            {.last_used_frame = fence_value,
             .resource = std::make_unique<descriptor_range_owner>(descriptor_heap_dsv,
                                                                  descriptors)});
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
   utility::com_ptr<ID3D12Device10> device_d3d;
   release_ptr<D3D12MA::Allocator> allocator;
   utility::com_ptr<ID3D12Fence> fence;
   UINT64 fence_value = 1;
   UINT64 previous_frame_fence_value = 0;
   UINT64 completed_fence_value = 0;
   UINT64 frame_index = 0;
   wil::unique_event fence_event{CreateEventW(nullptr, false, false, nullptr)};
   command_queue command_queue;

   command_allocator_factory command_allocator_factory{device_d3d};

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

   async_copy_manager copy_manager{*device_d3d, *allocator};

   swap_chain swap_chain;

private:
   auto create_resource(const D3D12_RESOURCE_DESC& desc, const D3D12_HEAP_TYPE heap_type,
                        const D3D12_RESOURCE_STATES initial_resource_state,
                        std::optional<D3D12_CLEAR_VALUE> optimized_clear_value = std::nullopt)
      -> std::shared_ptr<ID3D12Resource>
   {
      const D3D12MA::ALLOCATION_DESC alloc_desc{.HeapType = heap_type};

      utility::com_ptr<ID3D12Resource> d3d12_resource;
      release_ptr<D3D12MA::Allocation> allocation;

      throw_if_failed(
         allocator->CreateResource(&alloc_desc, &desc, initial_resource_state,
                                   optimized_clear_value
                                      ? &optimized_clear_value.value()
                                      : nullptr,
                                   allocation.clear_and_assign(),
                                   IID_PPV_ARGS(d3d12_resource.clear_and_assign())));

      auto resource_owner =
         std::make_shared<resource>(*this, d3d12_resource, std::move(allocation));

      return std::shared_ptr<ID3D12Resource>{resource_owner, d3d12_resource.get()};
   }

   void process_deferred_resource_destructions();

   struct descriptor_range_owner {
      descriptor_range_owner(descriptor_heap& heap, descriptor_range range)
         : heap{&heap}, range{range}
      {
      }
      descriptor_range_owner(const descriptor_range_owner&) noexcept = delete;
      auto operator=(const descriptor_range_owner&) noexcept
         -> descriptor_range_owner& = delete;

      descriptor_range_owner(descriptor_range_owner&&) noexcept = delete;
      auto operator=(descriptor_range_owner&& other) noexcept
         -> descriptor_range_owner& = delete;

      ~descriptor_range_owner()
      {
         if (heap) heap->free_static(range);
      }

      descriptor_heap* heap = nullptr;
      descriptor_range range{};
   };

   struct resource_owner {
      release_ptr<D3D12MA::Allocation> allocation;
      utility::com_ptr<ID3D12Resource> resource;
   };

   struct deferred_destruction {
      UINT64 last_used_frame = 0;

      boost::variant2::variant<resource_owner, std::unique_ptr<descriptor_range_owner>> resource;
   };

   std::mutex _deferred_destruction_mutex;
   std::vector<deferred_destruction> _deferred_destructions;
};

}
