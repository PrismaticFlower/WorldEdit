#pragma once

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

   auto size() const noexcept -> uint32
   {
      return _size;
   }

private:
   friend device;

   buffer(std::shared_ptr<ID3D12Resource> resource, const uint32 size)
      : _resource{resource}, _size{size}
   {
   }

   std::shared_ptr<ID3D12Resource> _resource = nullptr;
   uint32 _size = 0;
};

}
