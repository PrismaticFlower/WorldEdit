#pragma once

#include "texture.hpp"

namespace we::munge {

void convert_to_detail_map(texture& texture, bool has_alpha);

void make_opaque(texture& texture) noexcept;

void adjust_saturation(texture& texture, float saturation) noexcept;

void generate_mipmaps(texture& texture);

void generate_normal_maps(texture& texture, float bump_scale);

void generate_normal_maps_highq(texture& texture, float bump_scale);

void normalize_maps(texture& texture) noexcept;

void override_z_to_one(texture& texture) noexcept;

void apply_u_border(texture& texture, uint32 border_and_mask,
                    uint32 border_or_mask) noexcept;

void apply_v_border(texture& texture, uint32 border_and_mask,
                    uint32 border_or_mask) noexcept;

}