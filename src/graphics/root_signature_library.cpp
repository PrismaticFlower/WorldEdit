
#include "root_signature_library.hpp"
#include "sampler_list.hpp"

namespace we::graphics {

namespace {

constexpr uint32 frame_cb_register = 0;
constexpr uint32 lights_cb_register = 1;
constexpr uint32 object_cb_register = 2;
constexpr uint32 material_cb_register = 3;
constexpr uint32 terrain_cb_register = 4;
constexpr uint32 meta_mesh_cb_register = 5;
constexpr uint32 sky_mesh_cb_register = 6;
constexpr uint32 water_cb_register = 7;
constexpr uint32 thumbnail_camera_cb_register = 8;
constexpr uint32 grid_overlay_cb_register = 9;

constexpr uint32 terrain_patch_data_register = 0;
constexpr uint32 meta_draw_instance_data_register = 1;
constexpr uint32 water_patch_data_register = 2;

constexpr gpu::root_parameter frame_constant_buffer = {
   .type = gpu::root_parameter_type::constant_buffer_view,

   .shader_register = frame_cb_register,
   .visibility = gpu::root_shader_visibility::all,
};

constexpr gpu::root_parameter lights_constant_buffer = {
   .type = gpu::root_parameter_type::constant_buffer_view,

   .shader_register = lights_cb_register,
   .visibility = gpu::root_shader_visibility::pixel,
};

constexpr gpu::root_parameter object_constant_buffer = {
   .type = gpu::root_parameter_type::constant_buffer_view,

   .shader_register = object_cb_register,
   .visibility = gpu::root_shader_visibility::all,
};

constexpr gpu::root_parameter material_constant_buffer{
   .type = gpu::root_parameter_type::constant_buffer_view,

   .shader_register = material_cb_register,
   .visibility = gpu::root_shader_visibility::pixel,
};

constexpr std::array<gpu::static_sampler_desc, sampler_count> static_samplers =
   sampler_descriptions(gpu::root_shader_visibility::all);

constexpr std::array<gpu::static_sampler_desc, sampler_count> pixel_static_samplers =
   sampler_descriptions(gpu::root_shader_visibility::pixel);

const gpu::root_signature_desc mesh_desc{
   .parameters =
      {
         object_constant_buffer,
         material_constant_buffer,
         frame_constant_buffer,
         lights_constant_buffer,
      },

   .samplers = pixel_static_samplers,

   .flags = {.allow_input_assembler_input_layout = true},

   .debug_name = "mesh_root_signature",
};

const gpu::root_signature_desc terrain_desc{
   .parameters =
      {
         frame_constant_buffer,
         lights_constant_buffer,

         // terrain constant buffer
         gpu::root_parameter{
            .type = gpu::root_parameter_type::constant_buffer_view,

            .shader_register = terrain_cb_register,
            .visibility = gpu::root_shader_visibility::all,
         },

         // terrain patch data
         gpu::root_parameter{
            .type = gpu::root_parameter_type::shader_resource_view,

            .shader_register = terrain_patch_data_register,
            .visibility = gpu::root_shader_visibility::vertex,
         },
      },

   .samplers = pixel_static_samplers,

   .flags = {},

   .debug_name = "terrain_root_signature",
};

const gpu::root_signature_desc terrain_cut_mesh_desc{
   .parameters =
      {
         object_constant_buffer,
         frame_constant_buffer,
      },

   .flags = {.allow_input_assembler_input_layout = true},

   .debug_name = "terrain_cut_mesh_root_signature",
};

const gpu::root_signature_desc water_desc{
   .parameters =
      {
         frame_constant_buffer,

         // water constant buffer
         gpu::root_parameter{
            .type = gpu::root_parameter_type::constant_buffer_view,

            .shader_register = water_cb_register,
            .visibility = gpu::root_shader_visibility::all,
         },

         // water patches
         gpu::root_parameter{
            .type = gpu::root_parameter_type::shader_resource_view,

            .shader_register = water_patch_data_register,
            .visibility = gpu::root_shader_visibility::vertex,
         },
      },

   .samplers = pixel_static_samplers,

   .flags = {},

   .debug_name = "water_root_signature",
};

const gpu::root_signature_desc mesh_shadow_desc{
   .parameters =
      {
         object_constant_buffer,
         material_constant_buffer,
         frame_constant_buffer,
      },

   .samplers = pixel_static_samplers,

   .flags = {.allow_input_assembler_input_layout = true},

   .debug_name = "mesh_shadow_root_signature",
};

const gpu::root_signature_desc mesh_depth_prepass_desc{
   .parameters =
      {
         object_constant_buffer,
         material_constant_buffer,
         frame_constant_buffer,
      },

   .samplers = pixel_static_samplers,

   .flags = {.allow_input_assembler_input_layout = true},

   .debug_name = "mesh_depth_prepass_root_signature",
};

const gpu::root_signature_desc sky_mesh_desc{
   .parameters =
      {
         gpu::root_parameter{.type = gpu::root_parameter_type::constant_buffer_view,
                             .shader_register = sky_mesh_cb_register},
         material_constant_buffer,
         frame_constant_buffer,
      },

   .samplers = pixel_static_samplers,

   .flags = {.allow_input_assembler_input_layout = true},

   .debug_name = "sky_mesh_root_signature",
};

const gpu::root_signature_desc grid_overlay_desc{
   .parameters =
      {
         gpu::root_parameter{.type = gpu::root_parameter_type::_32bit_constants,
                             .shader_register = grid_overlay_cb_register,
                             .values_count = 9},
         frame_constant_buffer,
      },

   .flags = {.allow_input_assembler_input_layout = false},

   .debug_name = "grid_overlay_root_signature",
};

const gpu::root_signature_desc thumbnail_mesh_desc{
   .parameters =
      {
         material_constant_buffer,
         gpu::root_parameter{.type = gpu::root_parameter_type::_32bit_constants,
                             .shader_register = thumbnail_camera_cb_register,
                             .values_count = 3,
                             .visibility = gpu::root_shader_visibility::pixel},
         gpu::root_parameter{.type = gpu::root_parameter_type::constant_buffer_view,
                             .shader_register = thumbnail_camera_cb_register,
                             .visibility = gpu::root_shader_visibility::vertex},
      },

   .samplers = pixel_static_samplers,

   .flags = {.allow_input_assembler_input_layout = true},

   .debug_name = "thumbnail_mesh_root_signature",
};

const gpu::root_signature_desc thumbnail_downsample_mesh_desc{
   .parameters =
      {
         gpu::root_parameter{.type = gpu::root_parameter_type::_32bit_constants,
                             .shader_register = 0,
                             .values_count = 1,
                             .visibility = gpu::root_shader_visibility::pixel},
      },

   .samplers = pixel_static_samplers,

   .flags = {.allow_input_assembler_input_layout = false},

   .debug_name = "thumbnail_downsample_root_signature",
};

const gpu::root_signature_desc resample_env_map_desc{
   .parameters =
      {
         gpu::root_parameter{.type = gpu::root_parameter_type::_32bit_constants,
                             .shader_register = 0,
                             .values_count = 1,
                             .visibility = gpu::root_shader_visibility::pixel},
      },

   .samplers = pixel_static_samplers,

   .flags = {.allow_input_assembler_input_layout = false},

   .debug_name = "resample_env_map_root_signature",
};

const gpu::root_signature_desc mesh_wireframe_desc{
   .parameters =
      {
         object_constant_buffer,

         // wireframe constants
         gpu::root_parameter{
            .type = gpu::root_parameter_type::constant_buffer_view,

            .shader_register = meta_mesh_cb_register,
            .visibility = gpu::root_shader_visibility::pixel,
         },

         frame_constant_buffer,
      },

   .flags = {.allow_input_assembler_input_layout = true},

   .debug_name = "mesh_wireframe_root_signature",
};

const gpu::root_signature_desc meta_draw_desc{
   .parameters =
      {
         // instance data srv
         gpu::root_parameter{.type = gpu::root_parameter_type::shader_resource_view,
                             .shader_register = meta_draw_instance_data_register},

         frame_constant_buffer,
      },

   .flags = {.allow_input_assembler_input_layout = true},

   .debug_name = "meta_draw_root_signature",
};

const gpu::root_signature_desc ai_overlay_shape_desc{
   .parameters =
      {
         // instance data srv
         gpu::root_parameter{.type = gpu::root_parameter_type::shader_resource_view,
                             .shader_register = meta_draw_instance_data_register},

         frame_constant_buffer,
      },

   .flags = {.allow_input_assembler_input_layout = true},

   .debug_name = "ai_overlay_shape_root_signature",
};

const gpu::root_signature_desc ai_overlay_apply_desc{
   .parameters =
      {
         gpu::root_parameter{
            .type = gpu::root_parameter_type::_32bit_constants,
            .shader_register = 0,
            .values_count = 4,
         },
      },

   .flags = {.allow_input_assembler_input_layout = false},

   .debug_name = "ai_overlay_apply_root_signature",
};

const gpu::root_signature_desc tile_lights_clear_desc{
   .parameters =
      {
         // input cbv
         gpu::root_parameter{.type = gpu::root_parameter_type::constant_buffer_view,
                             .shader_register = 0},

         // tiles uav
         gpu::root_parameter{.type = gpu::root_parameter_type::unordered_access_view,
                             .shader_register = 0},
      },

   .debug_name = "tile_lights_clear_root_signature",
};

const gpu::root_signature_desc tile_lights_desc{
   .parameters =
      {
         // instance data srv
         gpu::root_parameter{.type = gpu::root_parameter_type::shader_resource_view,

                             .shader_register = 0},

         // tiles uav
         gpu::root_parameter{.type = gpu::root_parameter_type::unordered_access_view,

                             .shader_register = 0},

         // cbv
         gpu::root_parameter{.type = gpu::root_parameter_type::constant_buffer_view,

                             .shader_register = 0},
      },

   .flags = {.allow_input_assembler_input_layout = true},

   .debug_name = "tile_lights_root_signature",
};

const gpu::root_signature_desc depth_reduce_minmax_desc{
   .parameters =
      {
         // input cbv
         gpu::root_parameter{.type = gpu::root_parameter_type::_32bit_constants,
                             .shader_register = 0,
                             .values_count = 3},

         // output uav
         gpu::root_parameter{.type = gpu::root_parameter_type::unordered_access_view,
                             .shader_register = 0},
      },

   .samplers = static_samplers,

   .debug_name = "depth_reduce_minmax_root_signature",
};

const gpu::root_signature_desc imgui_desc{
   .parameters =
      {
         // texture
         gpu::root_parameter{.type = gpu::root_parameter_type::_32bit_constants,
                             .shader_register = 0,
                             .values_count = 1,
                             .visibility = gpu::root_shader_visibility::pixel},

         // inv viewport size
         gpu::root_parameter{.type = gpu::root_parameter_type::_32bit_constants,
                             .shader_register = 1,
                             .values_count = 2,
                             .visibility = gpu::root_shader_visibility::vertex},
      },

   .samplers = pixel_static_samplers,

   .flags = {.allow_input_assembler_input_layout = true},

   .debug_name = "dear_imgui_root_signature",
};

}

root_signature_library::root_signature_library(gpu::device& device)
{
   mesh = {device.create_root_signature(mesh_desc), device.direct_queue};
   terrain = {device.create_root_signature(terrain_desc), device.direct_queue};
   terrain_cut_mesh = {device.create_root_signature(terrain_cut_mesh_desc),
                       device.direct_queue};
   water = {device.create_root_signature(water_desc), device.direct_queue};
   mesh_shadow = {device.create_root_signature(mesh_shadow_desc), device.direct_queue};
   mesh_depth_prepass = {device.create_root_signature(mesh_depth_prepass_desc),
                         device.direct_queue};
   mesh_wireframe = {device.create_root_signature(mesh_wireframe_desc),
                     device.direct_queue};
   sky_mesh = {device.create_root_signature(sky_mesh_desc), device.direct_queue};
   grid_overlay = {device.create_root_signature(grid_overlay_desc), device.direct_queue};
   thumbnail_mesh = {device.create_root_signature(thumbnail_mesh_desc),
                     device.direct_queue};
   thumbnail_downsample = {device.create_root_signature(thumbnail_downsample_mesh_desc),
                           device.direct_queue};
   resample_env_map = {device.create_root_signature(resample_env_map_desc),
                       device.direct_queue};
   meta_draw = {device.create_root_signature(meta_draw_desc), device.direct_queue};

   ai_overlay_shape = {device.create_root_signature(ai_overlay_shape_desc),
                       device.direct_queue};
   ai_overlay_apply = {device.create_root_signature(ai_overlay_apply_desc),
                       device.direct_queue};

   tile_lights_clear = {device.create_root_signature(tile_lights_clear_desc),
                        device.direct_queue};
   tile_lights = {device.create_root_signature(tile_lights_desc), device.direct_queue};

   depth_reduce_minmax = {device.create_root_signature(depth_reduce_minmax_desc),
                          device.direct_queue};

   imgui = {device.create_root_signature(imgui_desc), device.direct_queue};
}

}
