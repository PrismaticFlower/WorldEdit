#pragma once

#include "d3d12_mem_alloc.hpp"
#include "hresult_error.hpp"
#include "set_debug_name.hpp"
#include "utility/com_ptr.hpp"

#include <atomic>
#include <cassert>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/container/small_vector.hpp>
#include <d3d12.h>

namespace we::graphics::gpu {

class async_copy_manager;

struct copy_context {
public:
   ~copy_context()
   {
      assert(closed_and_executed);
   }

   copy_context(const copy_context&) = delete;
   auto operator=(const copy_context&) -> copy_context& = delete;
   copy_context(copy_context&&) = delete;
   auto operator=(copy_context&& other) -> copy_context& = delete;

   ID3D12GraphicsCommandList5& command_list;

   auto create_upload_resource(const D3D12_RESOURCE_DESC& desc) -> ID3D12Resource&
   {
      const D3D12MA::ALLOCATION_DESC alloc_desc{.HeapType = D3D12_HEAP_TYPE_UPLOAD};

      copy_upload_resource& upload_resource = upload_resources.emplace_back();

      throw_if_failed(
         allocator.CreateResource(&alloc_desc, &desc,
                                  D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                  upload_resource.allocation.clear_and_assign(),
                                  IID_PPV_ARGS(
                                     upload_resource.resource.clear_and_assign())));

      return *upload_resource.resource;
   }

private:
   friend async_copy_manager;

   copy_context(ID3D12CommandAllocator& command_allocator,
                D3D12MA::Allocator& allocator, ID3D12GraphicsCommandList5& command_list)
      : command_list{command_list}, command_allocator{command_allocator}, allocator{allocator} {};

   ID3D12CommandAllocator& command_allocator;
   D3D12MA::Allocator& allocator;

   struct copy_upload_resource {
      utility::com_ptr<ID3D12Resource> resource;
      release_ptr<D3D12MA::Allocation> allocation;
   };

   using resource_vector_type =
      boost::container::small_vector<copy_upload_resource, 4>;

   resource_vector_type upload_resources;

   bool closed_and_executed = false;
};

class async_copy_manager {
public:
   explicit async_copy_manager(ID3D12Device6& device, D3D12MA::Allocator& allocator)
      : _device{device}, _allocator{allocator}
   {
      const D3D12_COMMAND_QUEUE_DESC desc{.Type = D3D12_COMMAND_LIST_TYPE_COPY};
      throw_if_failed(
         _device.CreateCommandQueue(&desc,
                                    IID_PPV_ARGS(_command_queue.clear_and_assign())));

      throw_if_failed(_device.CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                          IID_PPV_ARGS(_fence.clear_and_assign())));

      set_debug_name(*_command_queue, "Async Copy Queue");
      set_debug_name(*_fence, "Async Copy Fence");
   }

   async_copy_manager(const async_copy_manager&) = delete;
   auto operator=(const async_copy_manager&) -> async_copy_manager& = delete;

   async_copy_manager(async_copy_manager&&) = delete;
   auto operator=(async_copy_manager&&) -> async_copy_manager& = delete;

   ~async_copy_manager() = default;

   [[nodiscard]] auto aquire_context() -> copy_context
   {
      std::scoped_lock lock{_mutex};

      update_completed_lockless();

      auto command_allocator = aquire_command_allocator();
      auto command_list = aquire_command_list();

      throw_if_failed(command_list->Reset(command_allocator.get(), nullptr));

      return {*command_allocator.release(), _allocator, *command_list.release()};
   }

   auto close_and_execute(copy_context& copy_context) -> UINT64
   {
      throw_if_failed(copy_context.command_list.Close());

      ID3D12CommandList* command_list = &copy_context.command_list;
      _command_queue->ExecuteCommandLists(1, &command_list);

      std::scoped_lock lock{_mutex};

      const UINT64 copy_fence_value = ++_fence_value;
      throw_if_failed(_command_queue->Signal(_fence.get(), copy_fence_value));

      _queued_copy.store(true, std::memory_order_relaxed);

      copy_context.closed_and_executed = true;

      return_copy_context(copy_fence_value, copy_context);

      return copy_fence_value;
   }

   [[nodiscard]] auto fence() const noexcept -> ID3D12Fence&
   {
      return *_fence;
   }

   void enqueue_fence_wait_if_needed(ID3D12CommandQueue& queue) noexcept
   {
      if (_queued_copy.exchange(false, std::memory_order_relaxed)) {
         queue.Wait(_fence.get(), _fence_value.load(std::memory_order_relaxed));
      }
   }

   void update_completed()
   {
      std::scoped_lock lock{_mutex};

      update_completed_lockless();
   }

private:
   void update_completed_lockless()
   {
      const auto completed_value = _fence->GetCompletedValue();

      for (auto& pending : _pending_copy_resources) {
         if (pending.fence_value > completed_value) continue;

         pending.resources.clear();
         _free_command_allocators.emplace_back(std::move(pending.command_allocator));
      }

      std::erase_if(_pending_copy_resources, [=](const auto& used_allocator) {
         return used_allocator.fence_value <= completed_value;
      });
   }

   auto aquire_command_allocator() -> utility::com_ptr<ID3D12CommandAllocator>
   {
      if (_free_command_allocators.empty()) {
         utility::com_ptr<ID3D12CommandAllocator> command_allocator;

         throw_if_failed(
            _device.CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY,
                                           IID_PPV_ARGS(
                                              command_allocator.clear_and_assign())));

         set_debug_name(*command_allocator, "Async Copy Command Allocator");

         return command_allocator;
      }

      auto command_allocator = std::move(_free_command_allocators.back());

      _free_command_allocators.pop_back();

      return command_allocator;
   }

   auto aquire_command_list() -> utility::com_ptr<ID3D12GraphicsCommandList5>
   {
      if (_command_lists.empty()) {
         utility::com_ptr<ID3D12GraphicsCommandList5> command_list;

         throw_if_failed(
            _device.CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_COPY,
                                       D3D12_COMMAND_LIST_FLAG_NONE,
                                       IID_PPV_ARGS(command_list.clear_and_assign())));

         set_debug_name(*command_list, "Async Copy Command List");

         return command_list;
      }

      auto command_list = std::move(_command_lists.back());

      _command_lists.pop_back();

      return command_list;
   }

   void return_copy_context(const UINT64 wait_fence_value, copy_context& context)
   {
      _command_lists.emplace_back(&context.command_list);

      _pending_copy_resources.push_back(
         {.fence_value = wait_fence_value,
          .command_allocator = utility::com_ptr{&context.command_allocator},
          .resources = std::move(context.upload_resources)});
   }

   ID3D12Device6& _device;
   D3D12MA::Allocator& _allocator;
   utility::com_ptr<ID3D12CommandQueue> _command_queue;
   utility::com_ptr<ID3D12Fence> _fence;
   std::atomic<UINT64> _fence_value = 0;
   std::atomic_bool _queued_copy = false;

   std::mutex _mutex;
   std::vector<utility::com_ptr<ID3D12CommandAllocator>> _free_command_allocators;
   std::vector<utility::com_ptr<ID3D12GraphicsCommandList5>> _command_lists;

   struct pending_copy_context {
      UINT64 fence_value;
      utility::com_ptr<ID3D12CommandAllocator> command_allocator;
      copy_context::resource_vector_type resources;
   };

   std::vector<pending_copy_context> _pending_copy_resources;
};
}
