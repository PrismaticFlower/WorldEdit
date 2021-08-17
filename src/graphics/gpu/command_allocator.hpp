#pragma once

#include "hresult_error.hpp"
#include "set_debug_name.hpp"
#include "utility/com_ptr.hpp"

#include <algorithm>
#include <mutex>
#include <string_view>
#include <vector>

#include <d3d12.h>

#include <absl/container/flat_hash_map.h>

namespace we::graphics::gpu {

struct command_allocator_id : std::string_view {
   command_allocator_id() = default;

   consteval command_allocator_id(std::string_view str)
      : std::string_view{str} {};
   consteval command_allocator_id(const char* str) : std::string_view{str} {};
};

class command_allocator {
public:
   command_allocator(utility::com_ptr<ID3D12CommandAllocator> allocator,
                     D3D12_COMMAND_LIST_TYPE type, command_allocator_id id)
      : _allocator{std::move(allocator)}, _type{type}, _id{id}
   {
   }

   [[nodiscard]] auto get() const noexcept -> ID3D12CommandAllocator*
   {
      return _allocator.get();
   }

   [[nodiscard]] auto release() noexcept -> utility::com_ptr<ID3D12CommandAllocator>
   {
      return std::move(_allocator);
   }

   [[nodiscard]] auto id() const noexcept -> command_allocator_id
   {
      return _id;
   }

   [[nodiscard]] auto type() const noexcept -> D3D12_COMMAND_LIST_TYPE
   {
      return _type;
   }

private:
   utility::com_ptr<ID3D12CommandAllocator> _allocator;
   D3D12_COMMAND_LIST_TYPE _type{};
   command_allocator_id _id;
};

class command_allocator_factory {
public:
   command_allocator_factory(utility::com_ptr<ID3D12Device8> device) noexcept
      : _device{device}
   {
   }

   command_allocator_factory(const command_allocator_factory&) = delete;
   auto operator=(const command_allocator_factory&)
      -> command_allocator_factory& = delete;
   command_allocator_factory(command_allocator_factory&&) = delete;
   auto operator=(command_allocator_factory&&) -> command_allocator_factory& = delete;

   /// @brief Aquires a command_allocator.
   /// @param type The type of the allocator.
   /// @param id The id of the allocator.
   ///
   /// @details
   /// If a command allocator with the same
   /// type and id has been put_back into the factory and is now finished with
   /// by the GPU (according to the fence passed into put_back) returns that
   /// command allocator instead of making a new one.
   [[nodiscard]] auto aquire(D3D12_COMMAND_LIST_TYPE type, command_allocator_id id)
      -> command_allocator
   {
      std::scoped_lock lock{_mutex};

      if (auto cached_it = _allocators.find(index{type, id});
          cached_it != _allocators.end()) {
         if (auto allocator = try_find_ready_allocator(cached_it->second); allocator) {
            throw_if_failed(allocator->Reset());

            return {allocator, type, id};
         }
      }

      return {make_allocator(type, id), type, id};
   }

   /// @brief Puts a command allocator into the factory's cache.
   /// @param allocator The allocator.
   /// @param fence The fence used to track if the allocator is safe to reuse.
   /// @param fence_value The value of the fence that indicates the allocator is safe to reuse.
   void put_back(command_allocator allocator,
                 utility::com_ptr<ID3D12Fence> fence, UINT64 fence_value)
   {
      std::scoped_lock lock{_mutex};

      _allocators[index{allocator.type(), allocator.id()}]
         .emplace_back(fence, fence_value, allocator.release());
   }

private:
   struct index {
      D3D12_COMMAND_LIST_TYPE type;
      command_allocator_id id;

      bool operator==(const index&) const noexcept = default;

      template<typename H>
      friend H AbslHashValue(H h, const index& i)
      {
         return H::combine(std::move(h), i.type, i.id);
      }
   };

   struct cached_allocator {
      utility::com_ptr<ID3D12Fence> fence;
      UINT64 fence_value;
      utility::com_ptr<ID3D12CommandAllocator> allocator;
   };

   std::mutex _mutex;
   absl::flat_hash_map<index, std::vector<cached_allocator>> _allocators;

   utility::com_ptr<ID3D12Device8> _device;

   auto try_find_ready_allocator(std::vector<cached_allocator>& cache)
      -> utility::com_ptr<ID3D12CommandAllocator>
   {
      auto ready = std::ranges::find_if(cache, [](cached_allocator& allocator) {
         return allocator.fence->GetCompletedValue() >= allocator.fence_value;
      });

      if (ready == cache.end()) return nullptr;

      auto allocator = ready->allocator;

      cache.erase(ready);

      return allocator;
   }

   auto make_allocator(D3D12_COMMAND_LIST_TYPE type, command_allocator_id id)
      -> utility::com_ptr<ID3D12CommandAllocator>
   {
      utility::com_ptr<ID3D12CommandAllocator> allocator;

      throw_if_failed(
         _device->CreateCommandAllocator(type,
                                         IID_PPV_ARGS(allocator.clear_and_assign())));

      set_debug_name(*allocator, id);

      return allocator;
   }
};

/// @brief Creates and encapsulates a command_allocator such that it is put back
/// into it's command_allocator_factory at the end of scope.
class command_allocator_scoped {
public:
   /// @brief Constructs a scoped command_allocator.
   /// @param factory The factory to aquire the allocator from and to put back the allocator into. The factory's lifetime must be longer than this object's.
   /// @param type The type of the allocator.
   /// @param id The id of the allocator. See command_allocator_factory::aquire.
   /// @param fence The fence that will be used externally for the allocator's use tracking.
   /// @param fence_value The future value of fence that signals the allocator is finished being used.
   command_allocator_scoped(command_allocator_factory& factory,
                            D3D12_COMMAND_LIST_TYPE type, command_allocator_id id,
                            utility::com_ptr<ID3D12Fence> fence, UINT64 fence_value)
      : _allocator{factory.aquire(type, id)}, _factory{factory}, _fence{std::move(fence)}, _fence_value{fence_value}
   {
   }

   command_allocator_scoped(const command_allocator_scoped&) = delete;
   auto operator=(const command_allocator_scoped&) -> command_allocator_scoped& = delete;
   command_allocator_scoped(command_allocator_scoped&&) = delete;
   auto operator=(command_allocator_scoped&&) -> command_allocator_scoped& = delete;

   ~command_allocator_scoped()
   {
      _factory.put_back(_allocator, std::move(_fence), _fence_value);
   }

   operator const command_allocator&() noexcept
   {
      return _allocator;
   }

private:
   command_allocator _allocator;
   command_allocator_factory& _factory;
   const utility::com_ptr<ID3D12Fence> _fence;
   const UINT64 _fence_value;
};

namespace literals {

consteval auto operator"" _id(const char* str, size_t len) noexcept -> command_allocator_id
{
   return command_allocator_id{std::string_view{str, len}};
}

}

}
