#pragma once

#include "texture.hpp"

namespace we::munge {

enum class texture_format {
   unspecified,
   unknown,

   meta_detail,
   meta_bump,
   meta_bump_alpha,
   meta_compressed,
   meta_compressed_alpha,

   dxt1,
   dxt3,
   dxt5,

   a8r8g8b8,
   x8r8g8b8, // Doesn't work in pc_TextureMunge

   a4r4g4b4,
   a1r5g5b5,
   r5g6b5,
   x1r5g5b5, // Doesn't work in pc_TextureMunge

   a8l8,
   a8,
   l8,
   a4l4,

   v8u8,
};

enum class texture_write_format {
   dxt1,
   dxt3,
   dxt5,

   a8r8g8b8,

   a4r4g4b4,
   a1r5g5b5,
   r5g6b5,

   a8l8,
   a8,
   l8,
   a4l4,

   v8u8,
};

auto get_write_format(const texture& texture, const texture_format format,
                      const texture_traits traits) -> texture_write_format;

auto bytes_per_texel(const texture_write_format format) noexcept -> uint32;

auto bytes_per_block(const texture_write_format format) noexcept -> uint32;

bool is_block_compressed(const texture_write_format format) noexcept;

auto to_d3dformat(const texture_write_format format) noexcept -> uint32;

}