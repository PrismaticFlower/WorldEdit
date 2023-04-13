#pragma once

#include "async/thread_pool.hpp"
#include "container/enum_array.hpp"
#include "types.hpp"
#include "utility/enum_bitflags.hpp"

#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"

#include <vector>

namespace we::graphics {

class shader_library;
struct root_signature_library;

enum class material_pipeline_flags : uint8 {
   none = 0b0,
   alpha_cutout = 0b1,
   doublesided = 0b10,
   transparent = 0b100,
   additive = 0b1000,

   count = 0b10000
};

constexpr bool marked_as_enum_bitflag(material_pipeline_flags)
{
   return true;
}

using material_pipelines =
   container::enum_array<gpu::unique_pipeline_handle, material_pipeline_flags>;

struct pipeline_library {
   pipeline_library(gpu::device& device, const shader_library& shader_library,
                    const root_signature_library& root_signature_library);

   void reload(gpu::device& device, const shader_library& shader_library,
               const root_signature_library& root_signature_library);

   gpu::unique_pipeline_handle mesh_depth_prepass;
   gpu::unique_pipeline_handle mesh_depth_prepass_doublesided;
   gpu::unique_pipeline_handle mesh_depth_prepass_alpha_cutout;
   gpu::unique_pipeline_handle mesh_depth_prepass_alpha_cutout_doublesided;
   gpu::unique_pipeline_handle mesh_shadow;
   gpu::unique_pipeline_handle mesh_shadow_alpha_cutout;
   gpu::unique_pipeline_handle mesh_basic;
   gpu::unique_pipeline_handle mesh_basic_lighting;
   material_pipelines mesh_normal;
   gpu::unique_pipeline_handle mesh_wireframe;
   gpu::unique_pipeline_handle mesh_wireframe_doublesided;

   gpu::unique_pipeline_handle terrain_depth_prepass;
   gpu::unique_pipeline_handle terrain_basic;
   gpu::unique_pipeline_handle terrain_lighting;
   gpu::unique_pipeline_handle terrain_normal;

   gpu::unique_pipeline_handle meta_draw_shape;
   gpu::unique_pipeline_handle meta_draw_shape_outlined;
   gpu::unique_pipeline_handle meta_draw_shape_wireframe;
   gpu::unique_pipeline_handle meta_draw_sphere;
   gpu::unique_pipeline_handle meta_draw_sphere_wireframe;
   gpu::unique_pipeline_handle meta_draw_line_solid;
   gpu::unique_pipeline_handle meta_draw_line_overlay;
   gpu::unique_pipeline_handle meta_draw_triangle;
   gpu::unique_pipeline_handle meta_draw_triangle_wireframe;

   gpu::unique_pipeline_handle tile_lights_clear;
   gpu::unique_pipeline_handle tile_lights_spheres;

   gpu::unique_pipeline_handle depth_reduce_minmax;

   gpu::unique_pipeline_handle imgui;
};

}
