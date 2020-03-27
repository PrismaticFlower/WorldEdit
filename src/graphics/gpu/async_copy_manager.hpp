#pragma once

#include "hresult_error.hpp"
#include "utility/com_ptr.hpp"

#include <atomic>
#include <cassert>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/container/small_vector.hpp>
#include <d3d12.h>

namespace sk::graphics::gpu {

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
      const D3D12_HEAP_PROPERTIES heap_properties{.Type = D3D12_HEAP_TYPE_UPLOAD,
                                                  .CPUPageProperty =
                                                     D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                                                  .MemoryPoolPreference =
                                                     D3D12_MEMORY_POOL_UNKNOWN};

      throw_if_failed(device.CreateCommittedResource(
         &heap_properties, D3D12_HEAP_FLAG_NONE, &desc,
         D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
         IID_PPV_ARGS(upload_resources.emplace_back().clear_and_assign())));

      return *upload_resources.back();
   }

private:
   friend async_copy_manager;

   copy_context(ID3D12CommandAllocator& command_allocator,
                ID3D12Device6& device, ID3D12GraphicsCommandList5& command_list)
      : command_list{command_list}, device{device}, command_allocator{command_allocator} {};

   ID3D12Device6& device;
   ID3D12CommandAllocator& command_allocator;

   using resource_vector_type =
      boost::container::small_vector<utility::com_ptr<ID3D12Resource>, 4>;

   resource_vector_type upload_resources;

   bool closed_and_executed = false;
};

class async_copy_manager {
public:
   explicit async_copy_manager(ID3D12Device6& device) : _device{device}
   {
      const D3D12_COMMAND_QUEUE_DESC desc{.Type = D3D12_COMMAND_LIST_TYPE_COPY};
      throw_if_failed(
         _device.CreateCommandQueue(&desc,
                                    IID_PPV_ARGS(_command_queue.clear_and_assign())));

      throw_if_failed(_device.CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                          IID_PPV_ARGS(_fence.clear_and_assign())));
   }

   async_copy_manager(const async_copy_manager&) = delete;
   auto operator=(const async_copy_manager&) -> async_copy_manager& = delete;

   async_copy_manager(async_copy_manager&&) = delete;
   auto operator=(async_copy_manager &&) -> async_copy_manager& = delete;

   ~async_copy_manager() = default;

   [[nodiscard]] auto aquire_context() -> copy_context
   {
      std::scoped_lock lock{_mutex};

      update_completed_lockless();

      auto command_allocator = aquire_command_allocator();
      auto command_list = aquire_command_list();

      throw_if_failed(command_list->Reset(command_allocator.get(), nullptr));

      return {*command_allocator.release(), _device, *command_list.release()};
   }

   [[nodiscard]] auto close_and_execute(copy_context& copy_context) -> UINT64
   {
      throw_if_failed(copy_context.command_list.Close());

      ID3D12CommandList* command_list = &copy_context.command_list;
      _command_queue->ExecuteCommandLists(1, &command_list);

      const UINT64 copy_fence_value = ++_fence_value;
      throw_if_failed(_command_queue->Signal(_fence.get(), copy_fence_value));

      copy_context.closed_and_executed = true;

      std::scoped_lock lock{_mutex};

      return_copy_context(copy_fence_value, copy_context);

      return copy_fence_value;
   }

   [[nodiscard]] auto fence() const noexcept -> ID3D12Fence&
   {
      return *_fence;
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
         if (pending.first > completed_value) continue;

         pending.second.resources.clear();
         _free_command_allocators.emplace_back(
            std::move(pending.second.command_allocator));
      }

      std::erase_if(_pending_copy_resources, [=](const auto& used_allocator) {
         return used_allocator.first <= completed_value;
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

         return command_list;
      }

      auto command_list = std::move(_command_lists.back());

      _command_lists.pop_back();

      return command_list;
   }

   void return_copy_context(const UINT64 wait_fence_value, copy_context& context)
   {
      _command_lists.emplace_back(&context.command_list);

      _pending_copy_resources.emplace_back(
         wait_fence_value,
         pending_copy_context{.command_allocator =
                                 utility::com_ptr{&context.command_allocator},
                              .resources = std::move(context.upload_resources)});
   }

   ID3D12Device6& _device;
   utility::com_ptr<ID3D12CommandQueue> _command_queue;
   utility::com_ptr<ID3D12Fence> _fence;
   std::atomic<UINT64> _fence_value = 0;

   std::mutex _mutex;
   std::vector<utility::com_ptr<ID3D12CommandAllocator>> _free_command_allocators;
   std::vector<utility::com_ptr<ID3D12GraphicsCommandList5>> _command_lists;

   struct pending_copy_context {
      utility::com_ptr<ID3D12CommandAllocator> command_allocator;
      copy_context::resource_vector_type resources;
   };

   std::vector<std::pair<UINT64, pending_copy_context>> _pending_copy_resources;
};
}