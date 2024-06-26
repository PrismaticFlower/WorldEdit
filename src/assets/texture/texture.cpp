
#include "texture.hpp"
#include "math/align.hpp"

#include <cassert>
#include <stdexcept>

#include <Windows.h>

namespace we::assets::texture {

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
   auto mip_width = std::max(width >> mip_level, 1u);
   auto mip_height = std::max(height >> mip_level, 1u);
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

texture::texture(const init_params init_params)
{
   _width = init_params.width;
   _height = init_params.height;
   _mip_levels = init_params.mip_levels;
   _array_size = init_params.array_size;
   _format = init_params.format;
   _flags = init_params.flags;
   _size = [&] {
      std::size_t size = 0;

      for (uint32 array_index = 0; array_index < init_params.array_size; ++array_index) {
         for (uint32 mip_level = 0; mip_level < init_params.mip_levels; ++mip_level) {
            size += get_mip_level_desc(_width, _height, mip_level, _format).size;
            size = math::align_up(size, subresource_alignment);
         }
      }

      return size;
   }();
   _texture_data = {static_cast<std::byte*>(VirtualAlloc(nullptr, _size,
                                                         MEM_RESERVE | MEM_COMMIT,
                                                         PAGE_READWRITE)),
                    &free_texture_data};

   if (not _texture_data) std::terminate();

   _subresources.reserve(init_params.array_size * init_params.mip_levels);

   std::size_t subresource_offset = 0;

   for (uint32 array_index = 0; array_index < init_params.array_size; ++array_index) {
      for (uint32 mip_level = 0; mip_level < init_params.mip_levels; ++mip_level) {
         const texture_mip_level_desc mip_level_desc =
            get_mip_level_desc(_width, _height, mip_level, _format);

         _subresources.emplace_back(
            texture_subresource_view::init_params{.data = {_texture_data.get() + subresource_offset,
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

void texture::free_texture_data(std::byte* memory) noexcept
{
   if (memory) VirtualFree(memory, 0, MEM_RELEASE);
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
   return _texture_data.get();
}

auto texture::data() const noexcept -> const std::byte*
{
   return _texture_data.get();
}

auto texture::size() const noexcept -> std::size_t
{
   return _size;
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

auto texture::flags() const noexcept -> texture_flags
{
   return _flags;
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
