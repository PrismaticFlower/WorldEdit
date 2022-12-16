
#include "root_signature_library.hpp"

namespace we::graphics {

namespace {

constexpr uint32 frame_cb_register = 0;
constexpr uint32 lights_cb_register = 1;
constexpr uint32 object_cb_register = 2;
constexpr uint32 material_cb_register = 3;
constexpr uint32 terrain_cb_register = 4;
constexpr uint32 meta_mesh_cb_register = 5;

constexpr uint32 terrain_patch_data_register = 0;

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

const gpu::root_signature_desc mesh_desc{
   .parameters =
      {
         object_constant_buffer,
         material_constant_buffer,
         frame_constant_buffer,
         lights_constant_buffer,
      },

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

   .flags = {},

   .debug_name = "terrain_root_signature",
};

const gpu::root_signature_desc meta_mesh_desc{
   .parameters =
      {
         object_constant_buffer,

         // color constant (should this be a root constant?)
         gpu::root_parameter{
            .type = gpu::root_parameter_type::constant_buffer_view,

            .shader_register = meta_mesh_cb_register,
            .visibility = gpu::root_shader_visibility::pixel,
         },

         frame_constant_buffer,
      },

   .flags = {.allow_input_assembler_input_layout = true},

   .debug_name = "meta_mesh_root_signature",
};

const gpu::root_signature_desc meta_mesh_wireframe_desc{
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

   .debug_name = "meta_mesh_wireframe_root_signature",
};

const gpu::root_signature_desc meta_line_desc{
   .parameters =
      {
         // color constant (should this be a root constant?)
         gpu::root_parameter{
            .type = gpu::root_parameter_type::constant_buffer_view,

            .shader_register = meta_mesh_cb_register,
            .visibility = gpu::root_shader_visibility::pixel,
         },

         // frame constant buffer
         frame_constant_buffer,
      },

   .flags = {.allow_input_assembler_input_layout = true},

   .debug_name = "meta_line_root_signature",
};

const gpu::root_signature_desc mesh_shadow_desc{
   .parameters =
      {
         object_constant_buffer,
         frame_constant_buffer,
      },

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

   .flags = {.allow_input_assembler_input_layout = true},

   .debug_name = "mesh_depth_prepass_root_signature",
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

}

root_signature_library::root_signature_library(gpu::device& device)
{
   mesh = {device.create_root_signature(mesh_desc), device.direct_queue};
   terrain = {device.create_root_signature(terrain_desc), device.direct_queue};
   meta_mesh = {device.create_root_signature(meta_mesh_desc), device.direct_queue};
   meta_mesh_wireframe = {device.create_root_signature(meta_mesh_wireframe_desc),
                          device.direct_queue};
   meta_line = {device.create_root_signature(meta_line_desc), device.direct_queue};
   mesh_shadow = {device.create_root_signature(mesh_shadow_desc), device.direct_queue};
   mesh_depth_prepass = {device.create_root_signature(mesh_depth_prepass_desc),
                         device.direct_queue};
   mesh_wireframe = {device.create_root_signature(mesh_wireframe_desc),
                     device.direct_queue};

   tile_lights_clear = {device.create_root_signature(tile_lights_clear_desc),
                        device.direct_queue};
   tile_lights = {device.create_root_signature(tile_lights_desc), device.direct_queue};
}

}
