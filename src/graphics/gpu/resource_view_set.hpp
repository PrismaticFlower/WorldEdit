#pragma once

#include "descriptor_range.hpp"
#include "types.hpp"
#include "utility/com_ptr.hpp"

#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

#include <d3d12.h>
#include <gsl/gsl>
#include <object_ptr.hpp>

namespace we::graphics::gpu {

class device;

class resource_view_set {
public:
   resource_view_set() = default;

   resource_view_set(const resource_view_set&) noexcept = delete;
   auto operator=(const resource_view_set&) noexcept -> resource_view_set& = delete;

   resource_view_set(resource_view_set&& other) noexcept
   {
      this->swap(other);
   }

   auto operator=(resource_view_set&& other) noexcept -> resource_view_set&
   {
      resource_view_set discarded;

      discarded.swap(*this);
      this->swap(other);

      return *this;
   }

   ~resource_view_set();

   void swap(resource_view_set& other) noexcept
   {
      using std::swap;

      swap(this->_parent_device, other._parent_device);
      swap(this->_descriptor_range, other._descriptor_range);
      swap(this->_resources, other._resources);
   }

   bool alive() const noexcept
   {
      return _parent_device != nullptr;
   }

   explicit operator bool() const noexcept
   {
      return alive();
   }

   auto parent_device() const noexcept -> device*
   {
      return _parent_device.get();
   }

   auto descriptors() const noexcept -> descriptor_range
   {
      return _descriptor_range;
   }

private:
   friend device;

   resource_view_set(device& device, descriptor_range descriptors,
                     std::vector<gsl::owner<ID3D12Resource*>> resources)
      : _parent_device{&device}, _descriptor_range{descriptors}, _resources{std::move(resources)}
   {
   }

   jss::object_ptr<device> _parent_device = nullptr;

   descriptor_range _descriptor_range;
   std::vector<gsl::owner<ID3D12Resource*>> _resources;
};

}
