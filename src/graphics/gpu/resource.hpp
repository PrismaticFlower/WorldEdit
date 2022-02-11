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
      : _resource{resource.release()}, _allocation{allocation.release()}, _parent_device{&device}
   {
   }

   resource(const resource&) noexcept = delete;
   auto operator=(const resource&) noexcept -> resource& = delete;

   resource(resource&& other) noexcept
   {
      this->swap(other);
   }

   auto operator=(resource&& other) noexcept -> resource&
   {
      resource discarded;

      discarded.swap(*this);
      this->swap(other);

      return *this;
   }

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
      return _resource;
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

   gsl::owner<ID3D12Resource*> _resource = nullptr;
   gsl::owner<D3D12MA::Allocation*> _allocation = nullptr;
   device* _parent_device = nullptr;
};

}
