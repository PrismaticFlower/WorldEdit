#pragma once

#include "common.hpp"
#include "types.hpp"
#include "utility/com_ptr.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <utility>

#include <d3d12.h>
#include <gsl/gsl>

namespace we::graphics::gpu {

class device;

class texture {
public:
   texture() = default;

   texture(const texture&) noexcept = default;
   auto operator=(const texture&) noexcept -> texture& = default;

   texture(texture&& other) noexcept = default;

   auto operator=(texture&& other) noexcept -> texture& = default;

   void swap(texture& other) noexcept
   {
      using std::swap;

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
      return _resource != nullptr;
   }

   auto resource() const noexcept -> std::shared_ptr<ID3D12Resource>
   {
      return _resource;
   }

   auto view_resource() const noexcept -> ID3D12Resource*
   {
      return _resource.get();
   }

   auto format() const noexcept -> DXGI_FORMAT
   {
      return _format;
   }

   auto width() const noexcept -> uint32
   {
      return _width;
   }

   auto height() const noexcept -> uint32
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

private:
   friend device;

   texture(std::shared_ptr<ID3D12Resource> resource, const texture_desc& desc)
   {
      _resource = std::move(resource);
      _format = desc.format;
      _width = desc.width;
      _height = desc.height;
      _depth = desc.depth;
      _mip_levels = desc.mip_levels;
      _array_size = desc.array_size;
   }

   std::shared_ptr<ID3D12Resource> _resource;

   DXGI_FORMAT _format = DXGI_FORMAT_UNKNOWN;
   uint32 _width = 0;
   uint32 _height = 0;
   uint16 _depth = 0;
   uint16 _mip_levels = 0;
   uint16 _array_size = 0;
};

}
