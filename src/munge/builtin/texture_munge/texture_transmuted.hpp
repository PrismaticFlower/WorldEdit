#pragma once

#include "texture.hpp"
#include "texture_format.hpp"

namespace we::munge {

/// @brief Represents a "transmuted" view of a texture subresource. See texture_transmuted_view.
struct texture_transmuted_view_subresource {
   struct init_params {
      std::span<std::byte> texels;
      uint32 width = 0;
      uint32 height = 0;
      uint32 depth = 0;
      uint32 slice_pitch = 0;
   };

   texture_transmuted_view_subresource(const init_params& init) noexcept;

   [[nodiscard]] auto as_bytes() noexcept -> std::span<std::byte>;

   [[nodiscard]] auto as_bytes() const noexcept -> std::span<const std::byte>;

   [[nodiscard]] auto width() const noexcept -> uint32;

   [[nodiscard]] auto height() const noexcept -> uint32;

   [[nodiscard]] auto depth() const noexcept -> uint32;

   [[nodiscard]] auto slice(uint32 z) noexcept -> std::span<std::byte>;

private:
   std::span<std::byte> _texels;
   uint32 _width = 0;
   uint32 _height = 0;
   uint32 _depth = 0;
   uint32 _slice_pitch = 0;
};

/// @brief Represents a "transmuted" view of a texture. This is used to convert a texture's format inplace and reuse
/// the memory. The view is only valid for as the input texture.
///
/// Reading directly from a texture after it has been modified through a texture_transmuted_view will
/// produce nonsense results.
struct texture_transmuted_view {
   texture_transmuted_view(texture& texture, texture_write_format format);

   using subresource_index = texture::subresource_index;

   [[nodiscard]] auto subresource(const subresource_index index) const noexcept
      -> texture_transmuted_view_subresource;

   [[nodiscard]] auto width() const noexcept -> uint32;

   [[nodiscard]] auto height() const noexcept -> uint32;

   [[nodiscard]] auto depth() const noexcept -> uint32;

   [[nodiscard]] auto mip_levels() const noexcept -> uint32;

   [[nodiscard]] auto array_size() const noexcept -> uint32;

   [[nodiscard]] auto format() const noexcept -> texture_write_format;

private:
   texture& _texture;
   texture_write_format _format;
};

}