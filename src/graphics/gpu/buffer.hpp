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

   ~buffer();

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
   friend device;

   buffer(device& device, const uint32 size, utility::com_ptr<ID3D12Resource> resource)
      : _parent_device{&device}, _resource{resource.release()}, _size{size}
   {
   }

   jss::object_ptr<device> _parent_device = nullptr;
   gsl::owner<ID3D12Resource*> _resource = nullptr;
   uint32 _size = 0;
};

}
