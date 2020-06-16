
#include "texture.hpp"
#include "math/align.hpp"

#include <cassert>
#include <stdexcept>

#include <glm/glm.hpp>

#include <range/v3/numeric.hpp>
#include <range/v3/view.hpp>

namespace sk::assets::texture {

namespace {

struct texture_mip_level_desc {
   uint32 width = 0;
   uint32 height = 0;
   uint32 row_pitch = 0;
   std::size_t size = 0;
};

auto get_mip_level_desc(const uint32 width, const uint32 height, const uint32 mip_level,
                        const texture_format format) -> texture_mip_level_desc
{
   auto mip_width = glm::max(width >> mip_level, 1u);
   auto mip_height = glm::max(height >> mip_level, 1u);
   auto mip_row_pitch = static_cast<uint32>(
      math::align_up(mip_width * format_size(format), texture::pitch_alignment));

   return {.width = mip_width,
           .height = mip_height,
           .row_pitch = mip_row_pitch,
           .size = mip_height * mip_row_pitch};
}

}

texture_subresource_view::texture_subresource_view(const init_params init_params)
{
   if (init_params.row_pitch < (format_size(init_params.format) * init_params.width)) {
      throw std::invalid_argument{
         "attempt to create texture subresource view with invalid row pitch"};
   }

   if (init_params.data.size() < (init_params.row_pitch * init_params.height)) {
      throw std::invalid_argument{
         "attempt to create texture subresource view with invalid data size"};
   }

   _data_span = init_params.data;
   _offset = init_params.offset;
   _row_pitch = init_params.row_pitch;
   _width = init_params.width;
   _height = init_params.height;
   _format = init_params.format;
}

auto texture_subresource_view::data() noexcept -> std::byte*
{
   return _data_span.data();
}

auto texture_subresource_view::data() const noexcept -> const std::byte*
{
   return _data_span.data();
}

auto texture_subresource_view::size() const noexcept -> std::size_t
{
   return _data_span.size();
}

auto texture_subresource_view::offset() const noexcept -> std::size_t
{
   return _offset;
}

auto texture_subresource_view::row_pitch() const noexcept -> uint32
{
   return _row_pitch;
}

auto texture_subresource_view::width() const noexcept -> uint32
{
   return _width;
}

auto texture_subresource_view::height() const noexcept -> uint32
{
   return _height;
}

auto texture_subresource_view::format() const noexcept -> texture_format
{
   return _format;
}

auto texture_subresource_view::dxgi_format() const noexcept -> DXGI_FORMAT
{
   return to_dxgi_format(_format);
}

auto texture_subresource_view::load(const glm::uvec2 index) const noexcept -> float4
{
   assert(index.x < _width and index.y < _height);

   return load_texel(_format, index.x, index.y, _data_span, _width, _height, _row_pitch);
}

void texture_subresource_view::store(const glm::uvec2 index, const float4 value) noexcept
{
   assert(index.x < _width and index.y < _height);

   store_texel(value, _format, index.x, index.y, _data_span, _width, _height, _row_pitch);
}

texture::texture(const init_params init_params)
{
   _width = init_params.width;
   _height = init_params.height;
   _mip_levels = init_params.mip_levels;
   _array_size = init_params.array_size;
   _format = init_params.format;

   _texture_data.resize([&] {
      std::size_t size = 0;

      for ([[maybe_unused]] auto array_index :
           ranges::views::indices(init_params.array_size)) {
         for (auto mip_level : ranges::views::indices(init_params.mip_levels)) {
            size += get_mip_level_desc(_width, _height, mip_level, _format).size;
            size = math::align_up(size, subresource_alignment);
         }
      }

      return size;
   }());

   _subresources.reserve(init_params.array_size * init_params.mip_levels);

   std::size_t subresource_offset = 0;

   for ([[maybe_unused]] auto array_index :
        ranges::views::indices(init_params.array_size)) {
      for (auto mip_level : ranges::views::indices(init_params.mip_levels)) {
         const texture_mip_level_desc mip_level_desc =
            get_mip_level_desc(_width, _height, mip_level, _format);

         _subresources.emplace_back(
            texture_subresource_view::init_params{.data = {_texture_data.data() + subresource_offset,
                                                           mip_level_desc.size},
                                                  .offset = subresource_offset,
                                                  .row_pitch = mip_level_desc.row_pitch,
                                                  .width = mip_level_desc.width,
                                                  .height = mip_level_desc.height,
                                                  .format = _format});

         subresource_offset += mip_level_desc.size;
         subresource_offset =
            math::align_up(subresource_offset, subresource_alignment);
      }
   }
}

auto texture::subresource(const subresource_index index) -> texture_subresource_view&
{
   return _subresources[flatten_subresource_index(index)];
}

auto texture::subresource(const subresource_index index) const
   -> const texture_subresource_view&
{
   return _subresources[flatten_subresource_index(index)];
}

auto texture::subresource(const std::size_t flat_index) -> texture_subresource_view&
{
   if (flat_index >= _subresources.size()) {
      throw std::invalid_argument{
         "attempt to access nonexistent subresource in texture"};
   }

   return _subresources[flat_index];
}

auto texture::subresource(const std::size_t flat_index) const
   -> const texture_subresource_view&
{
   if (flat_index >= _subresources.size()) {
      throw std::invalid_argument{
         "attempt to access nonexistent subresource in texture"};
   }

   return _subresources[flat_index];
}

auto texture::subresource_count() const noexcept -> std::size_t
{
   return _subresources.size();
}

auto texture::data() noexcept -> std::byte*
{
   return _texture_data.data();
}

auto texture::data() const noexcept -> const std::byte*
{
   return _texture_data.data();
}

auto texture::size() const noexcept -> std::size_t
{
   return _texture_data.size();
}

auto texture::width() const noexcept -> uint32
{
   return _width;
}

auto texture::height() const noexcept -> uint32
{
   return _height;
}

auto texture::mip_levels() const noexcept -> uint16
{
   return _mip_levels;
}

auto texture::array_size() const noexcept -> uint16
{
   return _array_size;
}

auto texture::format() const noexcept -> texture_format
{
   return _format;
}

auto texture::dxgi_format() const noexcept -> DXGI_FORMAT
{
   return to_dxgi_format(_format);
}

auto texture::load(const subresource_index subresource, const glm::uvec2 index) const
   -> float4
{
   return _subresources[flatten_subresource_index(subresource)].load(index);
}

void texture::store(const subresource_index subresource, const glm::uvec2 index,
                    const float4 value)
{
   _subresources[flatten_subresource_index(subresource)].store(index, value);
}

auto texture::flatten_subresource_index(const subresource_index index) const -> std::size_t
{
   if (index.mip_level >= _mip_levels or index.array_index >= _array_size) {
      throw std::invalid_argument{
         "attempt to access nonexistent subresource in texture"};
   }

   return (index.array_index * _mip_levels) + index.mip_level;
}

}