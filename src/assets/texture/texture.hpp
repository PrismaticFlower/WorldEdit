#pragma once

#include "texture_format.hpp"
#include "types.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace we::assets::texture {

struct texture_flags {
   bool cube_map : 1 = false;
};

struct texture_index {
   uint32 x = 0;
   uint32 y = 0;
};

struct texture_subresource_view {
   struct init_params {
      std::span<std::byte> data;
      std::size_t offset = 0;
      uint32 row_pitch = 256;
      uint32 width = 1;
      uint32 height = 1;
      texture_format format = texture_format::r8g8b8a8_unorm;
   };

   texture_subresource_view(const init_params init_params);

   [[nodiscard]] auto data() noexcept -> std::byte*;

   [[nodiscard]] auto data() const noexcept -> const std::byte*;

   [[nodiscard]] auto size() const noexcept -> std::size_t;

   [[nodiscard]] auto offset() const noexcept -> std::size_t;

   [[nodiscard]] auto row_pitch() const noexcept -> uint32;

   [[nodiscard]] auto width() const noexcept -> uint32;

   [[nodiscard]] auto height() const noexcept -> uint32;

   [[nodiscard]] auto format() const noexcept -> texture_format;

   [[nodiscard]] auto dxgi_format() const noexcept -> DXGI_FORMAT;

private:
   std::span<std::byte> _data_span;
   std::size_t _offset = 0;
   uint32 _row_pitch = 0;
   uint32 _width = 0;
   uint32 _height = 0;
   texture_format _format = texture_format::r8g8b8a8_unorm;
};

struct texture {
   constexpr static std::size_t pitch_alignment = 256;
   constexpr static std::size_t subresource_alignment = 512;

   struct init_params {
      uint32 width = 1;
      uint32 height = 1;
      uint16 mip_levels = 1;
      uint16 array_size = 1;
      texture_format format = texture_format::r8g8b8a8_unorm;
      texture_flags flags = {.cube_map = false};
   };

   texture(const init_params init_params);

   struct subresource_index {
      const uint32 mip_level;
      const uint32 array_index = 0;
   };

   [[nodiscard]] auto subresource(const subresource_index index)
      -> texture_subresource_view&;

   [[nodiscard]] auto subresource(const subresource_index index) const
      -> const texture_subresource_view&;

   [[nodiscard]] auto subresource(const std::size_t flat_index)
      -> texture_subresource_view&;

   [[nodiscard]] auto subresource(const std::size_t flat_index) const
      -> const texture_subresource_view&;

   [[nodiscard]] auto subresource_count() const noexcept -> std::size_t;

   [[nodiscard]] auto data() noexcept -> std::byte*;

   [[nodiscard]] auto data() const noexcept -> const std::byte*;

   [[nodiscard]] auto size() const noexcept -> std::size_t;

   [[nodiscard]] auto width() const noexcept -> uint32;

   [[nodiscard]] auto height() const noexcept -> uint32;

   [[nodiscard]] auto mip_levels() const noexcept -> uint16;

   [[nodiscard]] auto array_size() const noexcept -> uint16;

   [[nodiscard]] auto format() const noexcept -> texture_format;

   [[nodiscard]] auto dxgi_format() const noexcept -> DXGI_FORMAT;

   [[nodiscard]] auto flags() const noexcept -> texture_flags;

private:
   auto flatten_subresource_index(const subresource_index index) const -> std::size_t;

   std::vector<texture_subresource_view> _subresources;
   std::unique_ptr<std::byte[]> _texture_data;

   uint32 _width = 0;
   uint32 _height = 0;
   uint16 _mip_levels = 0;
   uint16 _array_size = 0;
   texture_format _format = texture_format::r8g8b8a8_unorm;
   texture_flags _flags = {};
   uint64 _size = 0;
};

}
