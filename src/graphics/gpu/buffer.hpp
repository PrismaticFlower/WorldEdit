#pragma once

#include "device.hpp"
#include "types.hpp"

#include <cassert>
#include <cstddef>
#include <utility>

#include <d3d12.h>
#include <d3dx12.h>
#include <gsl/gsl>

namespace sk::graphics::gpu {

struct device;

struct buffer {
   buffer() = default;

   buffer(device& device, const UINT size, const D3D12_HEAP_TYPE heap_type,
          const D3D12_RESOURCE_STATES initial_resource_state)
   {
      const D3D12_HEAP_PROPERTIES heap_properties{.Type = heap_type,
                                                  .CPUPageProperty =
                                                     D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                                                  .MemoryPoolPreference =
                                                     D3D12_MEMORY_POOL_UNKNOWN};

      const auto buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(size);

      utility::com_ptr<ID3D12Resource> buffer_resource;

      throw_if_failed(device.device_d3d->CreateCommittedResource(
         &heap_properties, D3D12_HEAP_FLAG_NONE, &buffer_desc, initial_resource_state,
         nullptr, IID_PPV_ARGS(buffer_resource.clear_and_assign())));

      parent_device = &device;
      resource = buffer_resource.release();
      resource_state = initial_resource_state;
      this->size = size;
   }

   buffer(const buffer&) noexcept = delete;
   auto operator=(const buffer&) noexcept -> buffer& = delete;

   buffer(buffer&& other) noexcept
   {
      this->swap(other);
   }

   auto operator=(buffer&& other) noexcept -> buffer&
   {
      buffer discarded;

      discarded.swap(*this);
      this->swap(other);

      return *this;
   }

   ~buffer()
   {
      if (resource) {
         assert(parent_device);

         parent_device->deferred_destroy_resource(*this);
      }
   }

   void swap(buffer& other) noexcept
   {
      using std::swap;

      swap(this->parent_device, other.parent_device);
      swap(this->resource, other.resource);
      swap(this->resource_state, other.resource_state);
      swap(this->size, other.size);
   }

   bool alive() const noexcept
   {
      return resource;
   }

   explicit operator bool() const noexcept
   {
      return alive();
   }

   jss::object_ptr<device> parent_device = nullptr;
   gsl::owner<ID3D12Resource*> resource = nullptr;
   D3D12_RESOURCE_STATES resource_state = D3D12_RESOURCE_STATE_COMMON;
   UINT size = 0;
};

}
