#pragma once

#include "d3d12_mem_alloc.hpp"
#include "utility/com_ptr.hpp"

#include <utility>

#include <d3d12.h>
#include <gsl/gsl>

namespace we::graphics::gpu {

class device;

class resource {
public:
   resource() = default;

   resource(device& device, utility::com_ptr<ID3D12Resource> resource,
            release_ptr<D3D12MA::Allocation> allocation)
      : _resource{resource}, _allocation{std::move(allocation)}, _parent_device{&device}
   {
   }

   resource(const resource&) noexcept = delete;
   auto operator=(const resource&) noexcept -> resource& = delete;

   resource(resource&& other) noexcept = default;
   auto operator=(resource&& other) noexcept -> resource& = default;

   ~resource();

   bool alive() const noexcept
   {
      return _resource != nullptr;
   }

   explicit operator bool() const noexcept
   {
      return alive();
   }

   auto get() const noexcept -> ID3D12Resource*
   {
      return _resource.get();
   }

   void swap(resource& other) noexcept
   {
      using std::swap;

      swap(this->_resource, other._resource);
      swap(this->_allocation, other._allocation);
      swap(this->_parent_device, other._parent_device);
   }

private:
   friend device;

   utility::com_ptr<ID3D12Resource> _resource = nullptr;
   release_ptr<D3D12MA::Allocation> _allocation = nullptr;
   device* _parent_device = nullptr;
};

inline void swap(resource& l, resource& r) noexcept
{
   l.swap(r);
}

}
