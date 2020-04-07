#pragma once

#include "texture_format.hpp"
#include "types.hpp"

#include <cstddef>
#include <optional>
#include <vector>

#include <d3d12.h>
#include <glm/glm.hpp>
#include <gsl/gsl>

namespace sk::assets::texture {

class texture_subresource_view {
public:
   struct init_params {
      gsl::span<std::byte> data;
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

   [[nodiscard]] auto load(const glm::uvec2 index) const noexcept -> float4;

   void store(const glm::uvec2 index, const float4 value) noexcept;

private:
   gsl::span<std::byte> _data_span;
   std::size_t _offset = 0;
   uint32 _row_pitch = 0;
   uint32 _width = 0;
   uint32 _height = 0;
   texture_format _format = texture_format::r8g8b8a8_unorm;
};

class texture {
public:
   constexpr static std::size_t pitch_alignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;

   struct init_params {
      uint32 width = 1;
      uint32 height = 1;
      uint16 mip_levels = 1;
      uint16 array_size = 1;
      texture_format format = texture_format::r8g8b8a8_unorm;
   };

   texture(const init_params init_params);

   struct subresource_index {
      const uint32 mip_level;
      const uint32 array_index = 0;
   };

   [[nodiscard]] auto subresource(const subresource_index index)
      -> texture_subresource_view&;

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

   [[nodiscard]] auto load(const subresource_index subresource,
                           const glm::uvec2 index) const -> float4;

   void store(const subresource_index subresource, const glm::uvec2 index,
              const float4 value);

private:
   auto flatten_subresource_index(const subresource_index index) const -> std::size_t;

   std::vector<texture_subresource_view> _subresources;
   std::vector<std::byte> _texture_data;

   uint32 _width = 0;
   uint32 _height = 0;
   uint16 _mip_levels = 0;
   uint16 _array_size = 0;
   texture_format _format = texture_format::r8g8b8a8_unorm;
};

}