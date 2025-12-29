#pragma once

#include "types.hpp"

#include <bit>
#include <cassert>
#include <memory>
#include <span>

namespace we::munge {

struct texture_traits {
   bool is_greyscale = false;
   bool has_single_bit_alpha = false;
   bool has_alpha = false;
};

struct texture_slice {
   struct init_params {
      std::span<uint32> texels;
      uint32 width = 0;
      uint32 height = 0;
   };

   texture_slice() = default;

   texture_slice(const init_params& init_params);

   [[nodiscard]] auto as_bytes() noexcept -> std::span<std::byte>;

   [[nodiscard]] auto as_bytes() const noexcept -> std::span<const std::byte>;

   [[nodiscard]] auto width() const noexcept -> uint32;

   [[nodiscard]] auto height() const noexcept -> uint32;

   [[nodiscard]] auto at(std::size_t x, std::size_t y) noexcept -> uint32&
   {
      if (x >= _width) x = x % _width;
      if (y >= _height) y = y % _height;

      const std::size_t index = y * _width + x;

      assert(index < _texels.size());

      return _texels[index];
   }

   [[nodiscard]] auto at(std::size_t x, std::size_t y) const noexcept -> uint32
   {
      if (x >= _width) x = x % _width;
      if (y >= _height) y = y % _height;

      const std::size_t index = y * _width + x;

      assert(index < _texels.size());

      return _texels[index];
   }

private:
   std::span<uint32> _texels;
   uint32 _width = 0;
   uint32 _height = 0;
};

struct texture_subresource {
   struct init_params {
      std::span<uint32> texels;
      uint32 width = 0;
      uint32 height = 0;
      uint32 depth = 0;
   };

   texture_subresource() = default;

   texture_subresource(const init_params& init_params);

   [[nodiscard]] auto as_bytes() noexcept -> std::span<std::byte>;

   [[nodiscard]] auto as_bytes() const noexcept -> std::span<const std::byte>;

   [[nodiscard]] auto width() const noexcept -> uint32;

   [[nodiscard]] auto height() const noexcept -> uint32;

   [[nodiscard]] auto depth() const noexcept -> uint32;

   [[nodiscard]] auto slice(uint32 z) noexcept -> texture_slice;

   [[nodiscard]] auto at(std::size_t x, std::size_t y, std::size_t z) noexcept -> uint32&
   {
      if (x >= _width) x = x % _width;
      if (y >= _height) y = y % _height;
      if (z >= _depth) z = z % _depth;

      const std::size_t slice_size = _width * _height;
      const std::size_t index = z * slice_size + (y * _width + x);

      assert(index < _texels.size());

      return _texels[index];
   }

   [[nodiscard]] auto at(std::size_t x, std::size_t y, std::size_t z) const noexcept
      -> uint32
   {
      if (x >= _width) x = x % _width;
      if (y >= _height) y = y % _height;
      if (z >= _depth) z = z % _depth;

      const std::size_t slice_size = _width * _height;
      const std::size_t index = z * slice_size + (y * _width + x);

      assert(index < _texels.size());

      return _texels[index];
   }

private:
   std::span<uint32> _texels;
   uint32 _width = 0;
   uint32 _height = 0;
   uint32 _depth = 0;
};

struct texture {
   struct init_params {
      uint32 width = 1;
      uint32 height = 1;
      uint32 depth = 1;
      uint32 mip_levels = 0;
      uint32 array_size = 1;
   };

   texture(const init_params& init_params);

   struct subresource_index {
      const uint32 array_index = 0;
      const uint32 mip_level = 0;
   };

   [[nodiscard]] auto subresource(const subresource_index index) const noexcept
      -> texture_subresource;

   [[nodiscard]] auto width() const noexcept -> uint32;

   [[nodiscard]] auto height() const noexcept -> uint32;

   [[nodiscard]] auto depth() const noexcept -> uint32;

   [[nodiscard]] auto mip_levels() const noexcept -> uint32;

   [[nodiscard]] auto array_size() const noexcept -> uint32;

private:
   auto flatten_subresource_index(const subresource_index index) const noexcept
      -> uint32;

   std::unique_ptr<uint32[]> _texels;
   std::unique_ptr<texture_subresource[]> _subresources;

   uint32 _width = 0;
   uint32 _height = 0;
   uint32 _depth = 0;
   uint32 _array_size = 0;
   uint32 _mip_levels = 0;
   uint64 _size = 0;
};

}
