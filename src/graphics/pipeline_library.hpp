#pragma once

#include "async/thread_pool.hpp"
#include "container/enum_array.hpp"
#include "types.hpp"
#include "utility/enum_bitflags.hpp"

#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"

#include <vector>

namespace we::graphics {

struct shader_library;
struct root_signature_library;

enum class depth_prepass_pipeline_flags : uint8 {
   none = 0b0,
   alpha_cutout = 0b1,
   doublesided = 0b10,

   COUNT = 0b100
};

constexpr bool marked_as_enum_bitflag(depth_prepass_pipeline_flags)
{
   return true;
}
enum class material_pipeline_flags : uint8 {
   none = 0b0,
   doublesided = 0b1,
   transparent = 0b10,

   COUNT = 0b100
};

constexpr bool marked_as_enum_bitflag(material_pipeline_flags)
{
   return true;
}

enum class thumbnail_mesh_pipeline_flags : uint8 {
   none = 0b0,
   doublesided = 0b1,
   transparent = 0b10,
   alpha_cutout = 0b100,

   COUNT = 0b1000
};

constexpr bool marked_as_enum_bitflag(thumbnail_mesh_pipeline_flags)
{
   return true;
}

using depth_prepass_pipelines =
   container::enum_array<gpu::unique_pipeline_handle, depth_prepass_pipeline_flags>;

using material_pipelines =
   container::enum_array<gpu::unique_pipeline_handle, material_pipeline_flags>;

using thumbnail_mesh_pipelines =
   container::enum_array<gpu::unique_pipeline_handle, thumbnail_mesh_pipeline_flags>;

struct pipeline_library {
   pipeline_library(gpu::device& device, const shader_library& shader_library,
                    const root_signature_library& root_signature_library);

   void reload(gpu::device& device, const shader_library& shader_library,
               const root_signature_library& root_signature_library);

   gpu::unique_pipeline_handle mesh_basic;
   gpu::unique_pipeline_handle mesh_basic_lighting;
   depth_prepass_pipelines mesh_shadow;
   depth_prepass_pipelines mesh_depth_prepass;
   material_pipelines mesh_normal;
   gpu::unique_pipeline_handle mesh_wireframe;
   gpu::unique_pipeline_handle mesh_wireframe_doublesided;

   gpu::unique_pipeline_handle sky_mesh;

   gpu::unique_pipeline_handle block_depth_prepass;
   gpu::unique_pipeline_handle block_basic;
   gpu::unique_pipeline_handle block_basic_lighting;

   gpu::unique_pipeline_handle terrain_depth_prepass;
   gpu::unique_pipeline_handle terrain_basic;
   gpu::unique_pipeline_handle terrain_lighting;
   gpu::unique_pipeline_handle terrain_normal;
   gpu::unique_pipeline_handle terrain_grid;
   gpu::unique_pipeline_handle terrain_foliage_map;

   gpu::unique_pipeline_handle terrain_cut_mesh_mark;
   gpu::unique_pipeline_handle terrain_cut_mesh_clear;

   gpu::unique_pipeline_handle water;

   gpu::unique_pipeline_handle grid_overlay;

   thumbnail_mesh_pipelines thumbnail_mesh;
   gpu::unique_pipeline_handle thumbnail_downsample;

   gpu::unique_pipeline_handle resample_env_map;
   gpu::unique_pipeline_handle env_map_downsample;

   gpu::unique_pipeline_handle meta_draw_shape;
   gpu::unique_pipeline_handle meta_draw_shape_outlined;
   gpu::unique_pipeline_handle meta_draw_shape_wireframe;
   gpu::unique_pipeline_handle meta_draw_sphere;
   gpu::unique_pipeline_handle meta_draw_sphere_wireframe;
   gpu::unique_pipeline_handle meta_draw_line_solid;
   gpu::unique_pipeline_handle meta_draw_line_overlay;
   gpu::unique_pipeline_handle meta_draw_triangle;
   gpu::unique_pipeline_handle meta_draw_triangle_wireframe;

   gpu::unique_pipeline_handle ai_overlay_shape;
   gpu::unique_pipeline_handle ai_overlay_apply;

   gpu::unique_pipeline_handle gizmo_cone;
   gpu::unique_pipeline_handle gizmo_cone_orthographic;
   gpu::unique_pipeline_handle gizmo_line;
   gpu::unique_pipeline_handle gizmo_quad;
   gpu::unique_pipeline_handle gizmo_rotation_widget;
   gpu::unique_pipeline_handle gizmo_rotation_widget_orthographic;

   gpu::unique_pipeline_handle tile_lights_clear;
   gpu::unique_pipeline_handle tile_lights_spheres;

   gpu::unique_pipeline_handle depth_reduce_minmax;

   gpu::unique_pipeline_handle imgui;
};

}
