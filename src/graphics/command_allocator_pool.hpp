#pragma once

#include "hresult_error.hpp"
#include "utility/com_ptr.hpp"

#include <cassert>
#include <mutex>
#include <vector>

#include <boost/container/small_vector.hpp>

#include <d3d12.h>

namespace sk::graphics {

class command_allocator_pool {
public:
   command_allocator_pool(const D3D12_COMMAND_LIST_TYPE type, ID3D12Device& device) noexcept
      : _type{type}, _device{device}
   {
   }

   auto aquire(const UINT64 fence_value) -> ID3D12CommandAllocator&
   {
      std::lock_guard lock{_mutex};

      if (not _free_allocators.empty()) {
         const auto front_fence_value = _free_allocators.front().fence_value;

         if (_free_allocators.front().fence_value <= fence_value) {
            auto* allocator = _free_allocators.front().allocator;

            assert(allocator != nullptr);

            _free_allocators.erase(_free_allocators.begin());

            throw_if_failed(allocator->Reset());

            return *allocator;
         }
      }

      return get_new_allocator();
   }

   void free(ID3D12CommandAllocator& allocator, const UINT64 fence_value)
   {
      std::lock_guard lock{_mutex};

      _free_allocators.push_back(
         free_allocator{.fence_value = fence_value, .allocator = &allocator});
   }

private:
   auto get_new_allocator() -> ID3D12CommandAllocator&
   {
      throw_if_failed(_device.CreateCommandAllocator(
         _type, IID_PPV_ARGS(_allocators.emplace_back().clear_and_assign())));

      return *_allocators.back();
   }

   const static int inplace_pool_size = 16;

   template<typename T>
   using small_pool_vector = boost::container::small_vector<T, inplace_pool_size>;

   struct free_allocator {
      UINT64 fence_value;
      ID3D12CommandAllocator* allocator;
   };

   D3D12_COMMAND_LIST_TYPE _type = D3D12_COMMAND_LIST_TYPE_DIRECT;
   ID3D12Device& _device;
   std::mutex _mutex;

   small_pool_vector<free_allocator> _free_allocators;
   small_pool_vector<utility::com_ptr<ID3D12CommandAllocator>> _allocators;
};

}