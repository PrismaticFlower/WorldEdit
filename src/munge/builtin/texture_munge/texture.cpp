#include "texture.hpp"

#include <bit>

namespace we::munge {

texture_slice::texture_slice(const init_params& init_params)
   : _texels{init_params.texels}, _width{init_params.width}, _height{init_params.height}
{
   assert(_width * _height <= _texels.size());
}

[[nodiscard]] auto texture_slice::as_bytes() noexcept -> std::span<std::byte>
{
   return std::as_writable_bytes(_texels);
}

[[nodiscard]] auto texture_slice::as_bytes() const noexcept
   -> std::span<const std::byte>
{
   return std::as_bytes(_texels);
}

[[nodiscard]] auto texture_slice::width() const noexcept -> uint32
{
   return _width;
}

[[nodiscard]] auto texture_slice::height() const noexcept -> uint32
{
   return _height;
}

texture_subresource::texture_subresource(const init_params& init_params)
   : _texels{init_params.texels},
     _width{init_params.width},
     _height{init_params.height},
     _depth{init_params.depth}
{
   assert(_width * _height * _depth <= _texels.size());
}

auto texture_subresource::as_bytes() noexcept -> std::span<std::byte>
{
   return std::as_writable_bytes(_texels);
}

auto texture_subresource::as_bytes() const noexcept -> std::span<const std::byte>
{
   return std::as_bytes(_texels);
}

auto texture_subresource::width() const noexcept -> uint32
{
   return _width;
}

auto texture_subresource::height() const noexcept -> uint32
{
   return _height;
}

auto texture_subresource::depth() const noexcept -> uint32
{
   return _depth;
}

auto texture_subresource::slice(uint32 z) noexcept -> texture_slice
{
   if (z >= _depth) z = z % _depth;

   const uint32 slice_size = _width * _height;

   return texture_slice::init_params{
      .texels = _texels.subspan(slice_size * z, slice_size),
      .width = _width,
      .height = _height,
   };
}

texture::texture(const init_params& init_params)
{
   const uint32 max_mip_levels = std::bit_width(
      std::max({init_params.width, init_params.height, init_params.depth}));
   const uint32 mip_levels = init_params.mip_levels != 0
                                ? std::min(init_params.mip_levels, max_mip_levels)
                                : max_mip_levels;

   _width = init_params.width;
   _height = init_params.height;
   _depth = init_params.depth;
   _mip_levels = mip_levels;
   _array_size = init_params.array_size;

   const uint32 subresource_count = _mip_levels * _array_size;

   for (uint32 mip = 0; mip < _mip_levels; ++mip) {
      const uint32 mip_width = std::max(_width >> mip, 1u);
      const uint32 mip_height = std::max(_height >> mip, 1u);
      const uint32 mip_slice_size = std::max(mip_width * mip_height, 4u);
      const uint32 mip_depth = std::max(_depth >> mip, 1u);
      const uint32 mip_subresource_size = mip_slice_size * mip_depth;

      _size += _array_size * mip_subresource_size;
   }

   _texels = std::make_unique_for_overwrite<uint32[]>(_size);
   _subresources = std::make_unique<texture_subresource[]>(subresource_count);

   uint32 data_offset = 0;

   for (uint32 array_index = 0; array_index < _array_size; ++array_index) {
      for (uint32 mip = 0; mip < _mip_levels; ++mip) {
         const uint32 mip_width = std::max(_width >> mip, 1u);
         const uint32 mip_height = std::max(_height >> mip, 1u);
         const uint32 mip_slice_size = std::max(mip_width * mip_height, 4u);
         const uint32 mip_depth = std::max(_depth >> mip, 1u);
         const uint32 mip_subresource_size = mip_slice_size * mip_depth;

         _subresources[flatten_subresource_index({.array_index = array_index, .mip_level = mip})] =
            texture_subresource::init_params{
               .texels = {&_texels[data_offset], mip_subresource_size},
               .width = mip_width,
               .height = mip_height,
               .depth = mip_depth,
            };

         data_offset += mip_subresource_size;
      }
   }
}

auto texture::flatten_subresource_index(const subresource_index index) const noexcept
   -> uint32
{
   assert(index.array_index < _array_size);
   assert(index.mip_level < _mip_levels);

   return index.array_index * _mip_levels + index.mip_level;
}

auto texture::subresource(const subresource_index index) const noexcept -> texture_subresource
{
   return _subresources[flatten_subresource_index(index)];
}

auto texture::width() const noexcept -> uint32
{
   return _width;
}

auto texture::height() const noexcept -> uint32
{
   return _height;
}

auto texture::depth() const noexcept -> uint32
{
   return _depth;
}

auto texture::mip_levels() const noexcept -> uint32
{
   return _mip_levels;
}

auto texture::array_size() const noexcept -> uint32
{
   return _array_size;
}

}