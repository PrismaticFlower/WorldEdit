#pragma once

#include "texture_format.hpp"

#include "types.hpp"

namespace we::munge {

enum class bump_map { none, normal, highq };

enum class texture_type { _2d, cube, volume };

struct texture_munge_options {
   uint32 mip_levels = 0;

   bool u_border = false;
   bool v_border = false;

   uint32 u_border_color = 0xff'ff'ff;
   uint32 v_border_color = 0xff'ff'ff;

   bool u_border_alpha = false;
   bool v_border_alpha = false;

   uint32 u_border_alpha_value = 0xff;
   uint32 v_border_alpha_value = 0xff;

   float saturation = 0.5f;

   texture_type type = texture_type::_2d;

   uint32 depth = 0;

   bump_map bump_map = bump_map::none;

   bool override_Z_to_1 = false;

   float bump_scale = 1.0f;

   texture_format format = texture_format::unspecified;

   uint32 detail_bias = 0;
};

}