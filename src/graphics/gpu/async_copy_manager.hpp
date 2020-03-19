#pragma once

#include "hresult_error.hpp"
#include "utility/com_ptr.hpp"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <mutex>
#include <utility>
#include <vector>

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

private:
   friend async_copy_manager;

   copy_context(async_copy_manager& manager, ID3D12CommandAllocator& command_allocator,
                ID3D12GraphicsCommandList5& command_list)
      : command_list{command_list}, manager{manager}, command_allocator{command_allocator} {};

   async_copy_manager& manager;
   ID3D12CommandAllocator& command_allocator;
   bool closed_and_executed = false;
};

class async_copy_manager {
private:
   struct command_list_returner {
      async_copy_manager& copy_context;
      ID3D12CommandAllocator& allocator;

      void operator()(ID3D12GraphicsCommandList5* list) const noexcept
      {
         copy_context.return_command_list(*list);
      }
   };

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

      auto command_allocator = aquire_command_allocator();
      auto command_list = aquire_command_list();

      throw_if_failed(command_list->Reset(command_allocator.get(), nullptr));

      return {*this, *command_allocator.release(), *command_list.release()};
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

      return_command_allocator(copy_fence_value, copy_context.command_allocator);
      return_command_list(copy_context.command_list);

      return copy_fence_value;
   }

   [[nodiscard]] auto fence() const noexcept -> ID3D12Fence&
   {
      return *_fence;
   }

   void update_completed()
   {
      const auto completed_value = _fence->GetCompletedValue();

      std::scoped_lock lock{_mutex};

      for (auto& used_allocator : _used_command_allocators) {
         if (used_allocator.first <= completed_value) {
            _free_command_allocators.emplace_back(std::move(used_allocator.second));
         }
      }

      _used_command_allocators.erase(std::remove_if(_used_command_allocators.begin(),
                                                    _used_command_allocators.end(),
                                                    [](const auto& used) {
                                                       return used.second == nullptr;
                                                    }),
                                     _used_command_allocators.end());
   }

private:
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

   void return_command_allocator(const UINT64 wait_fence_value,
                                 ID3D12CommandAllocator& allocator)
   {
      _used_command_allocators.emplace_back(wait_fence_value, &allocator);
   }

   void return_command_list(ID3D12GraphicsCommandList5& command_list)
   {
      _command_lists.emplace_back(&command_list);
   }

   ID3D12Device6& _device;
   utility::com_ptr<ID3D12CommandQueue> _command_queue;
   utility::com_ptr<ID3D12Fence> _fence;
   std::atomic<UINT64> _fence_value = 0;

   std::mutex _mutex;
   std::vector<utility::com_ptr<ID3D12CommandAllocator>> _free_command_allocators;
   std::vector<utility::com_ptr<ID3D12GraphicsCommandList5>> _command_lists;

   std::vector<std::pair<UINT64, utility::com_ptr<ID3D12CommandAllocator>>> _used_command_allocators;
};
}