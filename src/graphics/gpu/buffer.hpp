#pragma once

#include "common.hpp"
#include "types.hpp"
#include "utility/com_ptr.hpp"

#include <cassert>
#include <cstddef>
#include <utility>

#include <d3d12.h>
#include <gsl/gsl>
#include <object_ptr.hpp>

namespace we::graphics::gpu {

class device;

class buffer {
public:
   buffer() = default;

   buffer(const buffer&) noexcept = default;
   auto operator=(const buffer&) noexcept -> buffer& = default;

   buffer(buffer&& other) noexcept = default;

   auto operator=(buffer&& other) noexcept -> buffer& = default;

   bool alive() const noexcept
   {
      return _resource != nullptr;
   }

   explicit operator bool() const noexcept
   {
      return alive();
   }

   auto resource() const noexcept -> std::shared_ptr<ID3D12Resource>
   {
      return _resource;
   }

   auto view_resource() const noexcept -> ID3D12Resource*
   {
      return _resource.get();
   }

   auto size() const noexcept -> uint64
   {
      return _size;
   }

   auto size_u32() const noexcept -> uint32
   {
      return to_uint32(_size);
   }

   auto gpu_virtual_address() const noexcept -> gpu_virtual_address
   {
      return _resource->GetGPUVirtualAddress();
   }

private:
   friend device;

   buffer(std::shared_ptr<ID3D12Resource> resource, const uint64 size)
      : _resource{resource}, _size{size}
   {
   }

   std::shared_ptr<ID3D12Resource> _resource = nullptr;
   uint64 _size = 0;
};

}
