#pragma once

#include "error.hpp"
#include "types.hpp"
#include "utility/com_ptr.hpp"

#include <atomic>
#include <concepts>
#include <vector>

#include <d3d12.h>

namespace we::graphics::gpu::detail {

struct command_allocator_pool {
   auto try_aquire(std::atomic_uint64_t& current_frame, ID3D12Fence& frame_fence)
      -> utility::com_ptr<ID3D12CommandAllocator>
   {
#ifndef NDEBUG
      exclusivity_assert exclusivity_assert{std::atomic_ref{_other_thread_using}};
#endif

      for (auto allocator_it = _allocators.begin();
           allocator_it != _allocators.end(); ++allocator_it) {
         if (allocator_it->frame_used == current_frame.load(std::memory_order_acquire)) {
            utility::com_ptr<ID3D12CommandAllocator> allocator =
               std::move(allocator_it->command_allocator);

            _allocators.erase(allocator_it);

            return allocator;
         }
         else if (frame_fence.GetCompletedValue() >= allocator_it->frame_used) {
            utility::com_ptr<ID3D12CommandAllocator> allocator =
               std::move(allocator_it->command_allocator);

            throw_if_fail(allocator->Reset());

            _allocators.erase(allocator_it);

            return allocator;
         }
      }

      return nullptr;
   }

   void add(utility::com_ptr<ID3D12CommandAllocator> allocator,
            std::atomic_uint64_t& current_frame)
   {
#ifndef NDEBUG
      exclusivity_assert exclusivity_assert{std::atomic_ref{_other_thread_using}};
#endif

      _allocators.emplace_back(std::move(allocator),
                               current_frame.load(std::memory_order_acquire));
   }

   void view_and_clear(std::invocable<utility::com_ptr<ID3D12CommandAllocator>> auto viewer)
   {
#ifndef NDEBUG
      exclusivity_assert exclusivity_assert{std::atomic_ref{_other_thread_using}};
#endif

      for (auto& [allocator, frame_used] : _allocators) {
         viewer(std::move(allocator));
      }

      _allocators.clear();
   }

private:
   struct allocator {
      utility::com_ptr<ID3D12CommandAllocator> command_allocator;
      uint64 frame_used = 0;
   };

   std::vector<allocator> _allocators;

#ifndef NDEBUG
   struct exclusivity_assert {
      exclusivity_assert(std::atomic_ref<bool> other_thread_using)
         : other_thread_using{other_thread_using}
      {
         if (other_thread_using.exchange(true)) {
            std::terminate(); // Two threads using command_allocator_pool at once.
         }
      }

      ~exclusivity_assert()
      {
         if (not other_thread_using.exchange(false)) {
            std::terminate(); // Two threads using command_allocator_pool at once.
         }
      }

      std::atomic_ref<bool> other_thread_using;
   };

   bool _other_thread_using = false;
#endif
};

}