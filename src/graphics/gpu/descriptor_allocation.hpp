#pragma once

#include "descriptor_range.hpp"

#include <utility>

#include <object_ptr.hpp>

namespace we::graphics::gpu {

class device;

class descriptor_allocation : public descriptor_range {
public:
   descriptor_allocation() = default;

   descriptor_allocation(descriptor_allocation&& other) noexcept
   {
      other.swap(*this);
   }

   auto operator=(descriptor_allocation&& other) noexcept -> descriptor_allocation&
   {
      descriptor_allocation discarded{};

      discarded.swap(*this);
      other.swap(*this);

      return *this;
   }

   descriptor_allocation(const descriptor_allocation&) = delete;
   auto operator=(const descriptor_allocation&) -> descriptor_allocation& = delete;

   ~descriptor_allocation();

   void swap(descriptor_allocation& other) noexcept
   {
      using std::swap;

      swap(this->_parent_device, other._parent_device);
      swap(this->_type, other._type);
      swap(static_cast<descriptor_range&>(*this),
           static_cast<descriptor_range&>(other));
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

   auto type() const noexcept -> D3D12_DESCRIPTOR_HEAP_TYPE
   {
      return _type;
   }

private:
   friend device;

   descriptor_allocation(device& device, const D3D12_DESCRIPTOR_HEAP_TYPE type,
                         const descriptor_range range) noexcept
      : descriptor_range{range}, _parent_device{&device}, _type{type} {};

   jss::object_ptr<device> _parent_device = nullptr;
   D3D12_DESCRIPTOR_HEAP_TYPE _type{};
};

}
