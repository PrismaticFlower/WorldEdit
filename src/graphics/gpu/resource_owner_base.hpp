#pragma once

#include "concepts.hpp"

#include <utility>

#include <d3d12.h>
#include <gsl/gsl>

namespace sk::graphics::gpu {

struct device;

struct resource_destroyer_helper {
private:
   template<typename T>
   friend struct resource_owner_base;

   static void destroy_resource(device& device, ID3D12Resource& resource);
};

template<typename T>
struct resource_owner_base {
   resource_owner_base() = default;

   resource_owner_base(const resource_owner_base&) = delete;
   auto operator=(const resource_owner_base&) -> resource_owner_base& = delete;

   resource_owner_base(resource_owner_base&& other) noexcept
   {
      std::swap(this->resource, other.resource);
      std::swap(this->resource_state, other.resource_state);
   }

   auto operator=(resource_owner_base&& other) noexcept -> resource_owner_base&
   {
      std::swap(this->resource, other.resource);
      std::swap(this->resource_state, other.resource_state);

      return *this;
   }

   ~resource_owner_base() requires device_child<T>
   {
      auto& derived = static_cast<T&>(*this);

      if (resource) {
         resource_destroyer_helper::destroy_resource(*derived.parent_device,
                                                     *derived.resource);
      }
   }

   bool alive() const noexcept
   {
      return resource;
   }

   gsl::owner<ID3D12Resource*> resource = nullptr;
   D3D12_RESOURCE_STATES resource_state = D3D12_RESOURCE_STATE_COMMON;
};

}
