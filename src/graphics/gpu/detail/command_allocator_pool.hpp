#pragma once

#include "error.hpp"
#include "types.hpp"
#include "utility/com_ptr.hpp"

#include <atomic>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <vector>

#include <absl/container/flat_hash_map.h>

#include <d3d12.h>

namespace we::graphics::gpu::detail {

struct command_allocator_pool {
   command_allocator_pool(ID3D12Device& device, ID3D12Fence& frame_fence,
                          std::atomic_uint64_t& current_frame,
                          D3D12_COMMAND_LIST_TYPE type)
      : _device{device},
        _frame_fence{frame_fence},
        _current_frame{current_frame},
        _type{type} {};

   auto aquire(const std::string_view name) -> utility::com_ptr<ID3D12CommandAllocator>
   {
      utility::com_ptr<ID3D12CommandAllocator> allocator =
         get_subpool(name).try_get_ready_allocator(_frame_fence);

      if (allocator) return allocator;

      throw_if_fail(
         _device.CreateCommandAllocator(_type,
                                        IID_PPV_ARGS(allocator.clear_and_assign())));

      return allocator;
   }

   void add(const std::string_view name, utility::com_ptr<ID3D12CommandAllocator> allocator)
   {
      get_subpool(name).add(std::move(allocator),
                            _current_frame.load(std::memory_order_acquire));
   }

private:
   struct allocator {
      utility::com_ptr<ID3D12CommandAllocator> command_allocator;
      uint64 frame_used = 0;
   };

   struct subpool {
      void add(utility::com_ptr<ID3D12CommandAllocator> allocator, uint64 frame_used)
      {
         std::scoped_lock lock{_mutex};

         _allocators.emplace_back(std::move(allocator), frame_used);
      }

      auto try_get_ready_allocator(ID3D12Fence& frame_fence)
         -> utility::com_ptr<ID3D12CommandAllocator>
      {
         std::scoped_lock subpool_lock{_mutex};

         for (auto allocator_it = _allocators.begin();
              allocator_it != _allocators.end(); ++allocator_it) {
            if (frame_fence.GetCompletedValue() >= allocator_it->frame_used) {
               utility::com_ptr<ID3D12CommandAllocator> allocator =
                  std::move(allocator_it->command_allocator);

               _allocators.erase(allocator_it);

               return allocator;
            }
         }

         return nullptr;
      }

   private:
      std::shared_mutex _mutex;
      std::vector<allocator> _allocators;
   };

   auto get_subpool(const std::string_view name) -> subpool&
   {
      std::scoped_lock lock{_subpools_mutex};

      if (auto it = _subpools.find(name); it != _subpools.end()) {
         return *it->second;
      }

      return *(_subpools[name] = std::make_unique<subpool>());
   }

   ID3D12Device& _device;
   ID3D12Fence& _frame_fence;
   std::atomic_uint64_t& _current_frame;
   D3D12_COMMAND_LIST_TYPE _type;

   std::shared_mutex _subpools_mutex;
   absl::flat_hash_map<std::string, std::unique_ptr<subpool>> _subpools;
};

}