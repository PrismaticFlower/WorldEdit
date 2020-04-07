#pragma once

#include "device.hpp"
#include "hresult_error.hpp"
#include "types.hpp"

#include <cassert>
#include <optional>
#include <utility>

#include <gsl/gsl>
#include <object_ptr.hpp>

namespace sk::graphics::gpu {

class texture {
public:
   struct texture_init {
      D3D12_RESOURCE_DIMENSION dimension = D3D12_RESOURCE_DIMENSION_UNKNOWN;
      D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
      D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
      DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
      uint16 width = 1;
      uint16 height = 1;
      uint16 depth = 1;
      uint16 mip_levels = 1;
      uint16 array_size = 1;

      std::optional<D3D12_CLEAR_VALUE> optimized_clear_value = std::nullopt;

      operator D3D12_RESOURCE_DESC() const noexcept
      {
         return {.Dimension = dimension,
                 .Alignment = 0,
                 .Width = width,
                 .Height = height,
                 .DepthOrArraySize =
                    dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? depth : array_size,
                 .MipLevels = mip_levels,
                 .Format = format,
                 .SampleDesc = {1, 0},
                 .Layout = layout,
                 .Flags = flags};
      };
   };

   texture() = default;

   texture(device& device, const texture_init init_params,
           const D3D12_HEAP_TYPE heap_type,
           const D3D12_RESOURCE_STATES initial_resource_state)
   {
      const D3D12_HEAP_PROPERTIES heap_properties{.Type = heap_type,
                                                  .CPUPageProperty =
                                                     D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                                                  .MemoryPoolPreference =
                                                     D3D12_MEMORY_POOL_UNKNOWN};
      const D3D12_RESOURCE_DESC desc = init_params;

      const D3D12_CLEAR_VALUE* optimized_clear_value =
         init_params.optimized_clear_value
            ? &init_params.optimized_clear_value.value()
            : nullptr;

      throw_if_failed(
         device.device_d3d->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE,
                                                    &desc, initial_resource_state,
                                                    optimized_clear_value,
                                                    IID_PPV_ARGS(&_resource)));

      resource_state = initial_resource_state;

      _parent_device = &device;
      _format = init_params.format;
      _width = init_params.width;
      _height = init_params.height;
      _depth = init_params.depth;
      _mip_levels = init_params.mip_levels;
      _array_size = init_params.array_size;
   }

   texture(const texture&) noexcept = delete;
   auto operator=(const texture&) noexcept -> texture& = delete;

   texture(texture&& other) noexcept
   {
      this->swap(other);
   }

   auto operator=(texture&& other) noexcept -> texture&
   {
      texture discarded;

      discarded.swap(*this);
      this->swap(other);

      return *this;
   }

   ~texture()
   {
      if (_resource) _parent_device->deferred_destroy_resource(*this);
   }

   void swap(texture& other) noexcept
   {
      using std::swap;

      swap(this->resource_state, other.resource_state);
      swap(this->_parent_device, other._parent_device);
      swap(this->_resource, other._resource);
      swap(this->_format, other._format);
      swap(this->_width, other._width);
      swap(this->_height, other._height);
      swap(this->_depth, other._depth);
      swap(this->_mip_levels, other._mip_levels);
      swap(this->_array_size, other._array_size);
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

   auto format() const noexcept -> DXGI_FORMAT
   {
      return _format;
   }

   auto width() const noexcept -> uint16
   {
      return _width;
   }

   auto height() const noexcept -> uint16
   {
      return _height;
   }

   auto depth() const noexcept -> uint16
   {
      return _depth;
   }

   auto mip_levels() const noexcept -> uint16
   {
      return _mip_levels;
   }

   auto array_size() const noexcept -> uint16
   {
      return _array_size;
   }

   D3D12_RESOURCE_STATES resource_state = D3D12_RESOURCE_STATE_COMMON;

private:
   jss::object_ptr<device> _parent_device = nullptr;
   gsl::owner<ID3D12Resource*> _resource = nullptr;

   DXGI_FORMAT _format = DXGI_FORMAT_UNKNOWN;
   uint16 _width = 0;
   uint16 _height = 0;
   uint16 _depth = 0;
   uint16 _mip_levels = 0;
   uint16 _array_size = 0;
};

}
