#pragma once

#include "async_copy_manager.hpp"
#include "buffer.hpp"
#include "common.hpp"
#include "concepts.hpp"
#include "descriptor_heap.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "swap_chain.hpp"
#include "utility/com_ptr.hpp"

#include <array>
#include <exception>
#include <memory>
#include <mutex>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wil/resource.h>

namespace sk::graphics::gpu {

using command_allocators =
   std::array<utility::com_ptr<ID3D12CommandAllocator>, render_latency>;

struct buffer_desc {
   uint32 alignment = 0;
   uint32 size = 0;
   D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
};

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

      const D3D12_RESOURCE_DESC d3d12_desc{.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                                           .Alignment = desc.alignment,
                                           .Width = desc.size,
                                           .Height = 1,
                                           .DepthOrArraySize = 1,
                                           .MipLevels = 1,
                                           .SampleDesc = {1, 0},
                                           .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
                                           .Flags = desc.flags};

      utility::com_ptr<ID3D12Resource> buffer_resource;

      throw_if_failed(device_d3d->CreateCommittedResource(
         &heap_properties, D3D12_HEAP_FLAG_NONE, &d3d12_desc, initial_resource_state,
         nullptr, IID_PPV_ARGS(buffer_resource.clear_and_assign())));

      return buffer{*this, desc.size, std::move(buffer_resource)};
   }

   template<resource_owner Owner>
   void deferred_destroy_resource(Owner& resource_owner)
   {
      std::lock_guard lock{_deferred_destruction_mutex};

      _deferred_resource_destructions.push_back(
         {// TODO (maybe, depending on how memory residency management shapes up): frame usage tracking for resources
          .last_used_frame = fence_value,
          .resource = utility::com_ptr{resource_owner.resource()}});
   }

   constexpr static int rtv_descriptor_heap_size = 128;
   constexpr static int dsv_descriptor_heap_size = 32;
   constexpr static int descriptor_static_heap_size = 16 * 4096;
   constexpr static int descriptor_dynamic_heap_size = 16 * 4096;

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

   descriptor_heap_cpu rtv_descriptor_heap{D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                                           rtv_descriptor_heap_size, *device_d3d};
   descriptor_heap_cpu dsv_descriptor_heap{D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
                                           dsv_descriptor_heap_size, *device_d3d};
   descriptor_heap descriptor_heap{D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                                   descriptor_static_heap_size,
                                   descriptor_dynamic_heap_size, *device_d3d};

   async_copy_manager copy_manager{*device_d3d};

   swap_chain swap_chain;

   root_signature_library root_signatures{*device_d3d};
   pipeline_library pipelines{*device_d3d, root_signatures};

private:
   void process_deferred_resource_destructions();

   struct deferred_resource_destruction {
      UINT64 last_used_frame = 0;
      utility::com_ptr<ID3D12Resource> resource;
   };

   std::mutex _deferred_destruction_mutex;
   std::vector<deferred_resource_destruction> _deferred_resource_destructions;
};

}