#pragma once

#include "gpu/rhi.hpp"

#include <array>

namespace we::graphics {

constexpr static uint32 sampler_max_anisotropy = 4;

constexpr static uint32 sampler_bilinear_wrap = 0;
constexpr static uint32 sampler_trilinear_wrap = 1;
constexpr static uint32 sampler_anisotropic_wrap = 2;

constexpr static uint32 sampler_bilinear_clamp = 3;
constexpr static uint32 sampler_trilinear_clamp = 4;
constexpr static uint32 sampler_anisotropic_clamp = 5;

constexpr static uint32 sampler_shadow = 6;

constexpr static uint32 sampler_count = 7;

inline constexpr auto sampler_descriptions(gpu::root_shader_visibility visibility,
                                           const uint32 max_anisotropy = sampler_max_anisotropy)
   -> std::array<gpu::static_sampler_desc, sampler_count>
{
   return {{
      {.filter = gpu::filter::bilinear,
       .shader_register = sampler_bilinear_wrap,
       .visibility = visibility},
      {.filter = gpu::filter::trilinear,
       .shader_register = sampler_trilinear_wrap,
       .visibility = visibility},
      {.filter = gpu::filter::anisotropic,
       .max_anisotropy = max_anisotropy,
       .shader_register = sampler_anisotropic_wrap,
       .visibility = visibility},

      {.filter = gpu::filter::bilinear,
       .address = gpu::texture_address_mode::clamp,
       .shader_register = sampler_bilinear_clamp,
       .visibility = visibility},
      {.filter = gpu::filter::trilinear,
       .address = gpu::texture_address_mode::clamp,
       .shader_register = sampler_trilinear_clamp,
       .visibility = visibility},
      {.filter = gpu::filter::anisotropic,
       .address = gpu::texture_address_mode::clamp,
       .max_anisotropy = max_anisotropy,
       .shader_register = sampler_anisotropic_clamp,
       .visibility = visibility},

      {.filter = gpu::filter::comparison_bilinear,
       .address = gpu::texture_address_mode::clamp,
       .comparison = gpu::comparison_func::less_equal,
       .shader_register = sampler_shadow,
       .visibility = visibility},
   }};
}

}