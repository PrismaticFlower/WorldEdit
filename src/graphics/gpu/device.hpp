#pragma once

#include "async_copy_manager.hpp"
#include "command_list_pool.hpp"
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
#include <object_ptr.hpp>
#include <wil/resource.h>

namespace sk::graphics::gpu {

using command_allocators =
   std::array<utility::com_ptr<ID3D12CommandAllocator>, render_latency>;

struct device {
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

   template<resource_owner Owner>
   void deferred_destroy_resource(Owner& resource_owner)
   {
      std::lock_guard lock{_deferred_destruction_mutex};

      _deferred_resource_destructions.push_back(
         {// TODO (maybe, depending on how memory residency management shapes up): frame usage tracking for resources
          .last_used_frame = fence_value,
          .resource = utility::com_ptr{resource_owner.resource}});
   }

   constexpr static int rtv_descriptor_heap_size = 128;
   constexpr static int dsv_descriptor_heap_size = 32;

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
   command_list_pool direct_command_list_pool{D3D12_COMMAND_LIST_TYPE_DIRECT,
                                              *device_d3d};

   async_copy_manager copy_manager{*device_d3d};

   swap_chain swap_chain;

   root_signature_library root_signatures{*device_d3d};
   pipeline_library pipelines{*device_d3d, root_signatures};

   auto aquire_command_list(const D3D12_COMMAND_LIST_TYPE type) // TODO: Remove command list pool.
   {
      jss::object_ptr<command_list_pool> pool = nullptr;

      using command_list_interface = command_list_pool::command_list_interface;

      switch (type) {
      case D3D12_COMMAND_LIST_TYPE_DIRECT:
         pool = &direct_command_list_pool;
         break;
      default:
         std::terminate();
      }

      const auto free = [pool](command_list_interface* command_list) {
         pool->free(command_list);
      };

      return std::unique_ptr<command_list_interface, decltype(free)>{pool->aquire(),
                                                                     free};
   }

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