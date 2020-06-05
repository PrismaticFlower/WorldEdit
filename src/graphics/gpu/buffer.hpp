#pragma once

#include "device.hpp"
#include "types.hpp"

#include <cassert>
#include <cstddef>
#include <utility>

#include <d3d12.h>
#include <d3dx12.h>
#include <gsl/gsl>
#include <object_ptr.hpp>

namespace sk::graphics::gpu {

struct device;

class buffer {
public:
   buffer() = default;

   buffer(device& device, const uint32 size, const D3D12_HEAP_TYPE heap_type,
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

      _parent_device = &device;
      _resource = buffer_resource.release();
      _size = size;
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
      if (_resource) _parent_device->deferred_destroy_resource(*this);
   }

   void swap(buffer& other) noexcept
   {
      using std::swap;

      swap(this->_parent_device, other._parent_device);
      swap(this->_resource, other._resource);
      swap(this->_size, other._size);
   }

   bool alive() const noexcept
   {
      return _resource != nullptr;
   }

   explicit operator bool() const noexcept
   {
      return alive();
   }

   auto parent_device() const noexcept -> device*
   {
      return _parent_device.get();
   }

   auto resource() const noexcept -> ID3D12Resource*
   {
      return _resource;
   }

   auto size() const noexcept -> uint32
   {
      return _size;
   }

private:
   jss::object_ptr<device> _parent_device = nullptr;
   gsl::owner<ID3D12Resource*> _resource = nullptr;
   uint32 _size = 0;
};

}
