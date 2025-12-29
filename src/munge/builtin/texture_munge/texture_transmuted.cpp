#include "texture_transmuted.hpp"

#include <cassert>

namespace we::munge {

texture_transmuted_view_subresource::texture_transmuted_view_subresource(
   const init_params& init) noexcept
   : _texels{init.texels},
     _width{init.width},
     _height{init.height},
     _depth{init.depth},
     _slice_pitch{init.slice_pitch}
{
   assert(_texels.size() == _depth * _slice_pitch);
}

auto texture_transmuted_view_subresource::as_bytes() noexcept -> std::span<std::byte>
{
   return _texels;
}

auto texture_transmuted_view_subresource::as_bytes() const noexcept
   -> std::span<const std::byte>
{
   return _texels;
}

auto texture_transmuted_view_subresource::width() const noexcept -> uint32
{
   return _width;
}

auto texture_transmuted_view_subresource::height() const noexcept -> uint32
{
   return _height;
}

auto texture_transmuted_view_subresource::depth() const noexcept -> uint32
{
   return _depth;
}

auto texture_transmuted_view_subresource::slice(uint32 z) noexcept
   -> std::span<std::byte>
{
   if (z >= _depth) z = z % _depth;

   return _texels.subspan(_slice_pitch * z, _slice_pitch);
}

texture_transmuted_view::texture_transmuted_view(texture& texture,
                                                 texture_write_format format)
   : _texture{texture}, _format{format}
{
}

auto texture_transmuted_view::subresource(const subresource_index index) const noexcept
   -> texture_transmuted_view_subresource
{
   texture_subresource subresource = _texture.subresource(index);

   if (is_block_compressed(_format)) {
      const uint32 width_blocks = (subresource.width() + 3) / 4;
      const uint32 height_blocks = (subresource.height() + 3) / 4;
      const uint32 slice_pitch =
         width_blocks * height_blocks * bytes_per_block(_format);
      const uint32 subresource_size = slice_pitch * subresource.depth();

      return texture_transmuted_view_subresource::init_params{
         .texels = subresource.as_bytes().subspan(0, subresource_size),
         .width = subresource.width(),
         .height = subresource.height(),
         .depth = subresource.depth(),
         .slice_pitch = slice_pitch,
      };
   }
   else {
      const uint32 slice_pitch =
         subresource.width() * subresource.height() * bytes_per_texel(_format);
      const uint32 subresource_size = slice_pitch * subresource.depth();

      return texture_transmuted_view_subresource::init_params{
         .texels = subresource.as_bytes().subspan(0, subresource_size),
         .width = subresource.width(),
         .height = subresource.height(),
         .depth = subresource.depth(),
         .slice_pitch = slice_pitch,
      };
   }
}

auto texture_transmuted_view::width() const noexcept -> uint32
{
   return _texture.width();
}

auto texture_transmuted_view::height() const noexcept -> uint32
{
   return _texture.height();
}

auto texture_transmuted_view::depth() const noexcept -> uint32
{
   return _texture.depth();
}

auto texture_transmuted_view::mip_levels() const noexcept -> uint32
{
   return _texture.mip_levels();
}

auto texture_transmuted_view::array_size() const noexcept -> uint32
{
   return _texture.array_size();
}

auto texture_transmuted_view::format() const noexcept -> texture_write_format
{
   return _format;
}

}