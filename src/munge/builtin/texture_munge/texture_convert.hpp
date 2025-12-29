#pragma once

#include "texture.hpp"
#include "texture_format.hpp"
#include "texture_transmuted.hpp"

namespace we::munge {

/// @brief Take a texture and convert it to a write format.
/// @param texture The texture to convert.
/// @param format The format to convert the texture to.
/// @return A texture_transmuted_view reusing the memory of texture. This is only valid for the lifetime of texture.
auto convert_texture(texture& texture, const texture_write_format format)
   -> texture_transmuted_view;

}
