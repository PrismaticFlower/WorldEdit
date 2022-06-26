
#include "root_signature_library.hpp"
#include "hresult_error.hpp"

#include <algorithm>

#include <boost/container/static_vector.hpp>

namespace we::graphics {

namespace {
constexpr uint32 mesh_register_space = 0;
constexpr uint32 material_register_space = 1;
constexpr uint32 terrain_register_space = 2;
constexpr uint32 lights_register_space = 3;
constexpr uint32 lights_tile_register_space = 4;
constexpr uint32 bindless_srv_space = 1000;

const gpu::static_sampler_desc trilinear_sampler{.filter = gpu::filter::trilinear,
                                                 .address = gpu::address_mode::wrap};

const gpu::static_sampler_desc bilinear_sampler{.filter = gpu::filter::bilinear,
                                                .address = gpu::address_mode::wrap};

const gpu::static_sampler_desc shadow_sampler{.filter = gpu::filter::bilinear,
                                              .address = gpu::address_mode::clamp,
                                              .comparison =
                                                 gpu::comparison_mode::less_equal};

const gpu::root_parameter_descriptor_table bindless_srv_table{
   .ranges =
      {
         {.type = gpu::descriptor_range_type::srv,
          .count = UINT_MAX,
          .base_shader_register = 0,
          .register_space = bindless_srv_space + 0,
          .offset_in_descriptors_from_table_start = 0},

         {.type = gpu::descriptor_range_type::srv,
          .count = UINT_MAX,
          .base_shader_register = 0,
          .register_space = bindless_srv_space + 1,
          .offset_in_descriptors_from_table_start = 0},

         {.type = gpu::descriptor_range_type::srv,
          .count = UINT_MAX,
          .base_shader_register = 0,
          .register_space = bindless_srv_space + 2,
          .offset_in_descriptors_from_table_start = 0},

         {.type = gpu::descriptor_range_type::srv,
          .count = UINT_MAX,
          .base_shader_register = 0,
          .register_space = bindless_srv_space + 3,
          .offset_in_descriptors_from_table_start = 0},

         {.type = gpu::descriptor_range_type::srv,
          .count = UINT_MAX,
          .base_shader_register = 0,
          .register_space = bindless_srv_space + 4,
          .offset_in_descriptors_from_table_start = 0},

         {.type = gpu::descriptor_range_type::srv,
          .count = UINT_MAX,
          .base_shader_register = 0,
          .register_space = bindless_srv_space + 5,
          .offset_in_descriptors_from_table_start = 0},
      },
   .visibility = gpu::shader_visibility::pixel,
};

const gpu::root_parameter_descriptor_table lights_input_descriptor_table{
   .ranges =
      {
         {.type = gpu::descriptor_range_type::cbv,
          .count = 1,
          .base_shader_register = 0,
          .register_space = lights_register_space,
          .offset_in_descriptors_from_table_start = 0},

         {.type = gpu::descriptor_range_type::srv,
          .count = 1,
          .base_shader_register = 1,
          .register_space = lights_register_space,
          .offset_in_descriptors_from_table_start = gpu::descriptor_range_offset_append},

         {.type = gpu::descriptor_range_type::cbv,
          .count = 1,
          .base_shader_register = 2,
          .register_space = lights_register_space,
          .offset_in_descriptors_from_table_start = gpu::descriptor_range_offset_append},

         {.type = gpu::descriptor_range_type::srv,
          .count = 1,
          .base_shader_register = 3,
          .register_space = lights_register_space,
          .offset_in_descriptors_from_table_start = gpu::descriptor_range_offset_append},

         {.type = gpu::descriptor_range_type::srv,
          .count = 1,
          .base_shader_register = 4,
          .register_space = lights_register_space,
          .offset_in_descriptors_from_table_start = gpu::descriptor_range_offset_append},
      },
   .visibility = gpu::shader_visibility::pixel,
};

const gpu::root_signature_desc mesh_desc{
   .name = "mesh_root_signature",

   .parameters =
      {// per-object constants
       gpu::root_parameter_cbv{
          .shader_register = 1,
          .register_space = mesh_register_space,
          .visibility = gpu::shader_visibility::vertex,
       },

       // material constants
       gpu::root_parameter_descriptor_table{
          .ranges =
             {
                {.type = gpu::descriptor_range_type::cbv,
                 .count = 1,
                 .base_shader_register = 0,
                 .register_space = material_register_space,
                 .offset_in_descriptors_from_table_start = 0},
             },
          .visibility = gpu::shader_visibility::pixel,
       },

       // camera descriptors
       gpu::root_parameter_descriptor_table{
          .ranges =
             {
                {.type = gpu::descriptor_range_type::cbv,
                 .count = 1,
                 .base_shader_register = 0,
                 .register_space = mesh_register_space,
                 .offset_in_descriptors_from_table_start = 0},
             },
          .visibility = gpu::shader_visibility::all,
       },

       // lights descriptors
       lights_input_descriptor_table,

       // bindless descriptors
       bindless_srv_table

      },

   .samplers =
      {
         {.sampler = trilinear_sampler,
          .shader_register = 0,
          .register_space = 0,
          .visibility = gpu::shader_visibility::pixel},

         {.sampler = shadow_sampler,
          .shader_register = 2,
          .register_space = lights_register_space,
          .visibility = gpu::shader_visibility::pixel},
      },

   .flags = {.allow_input_assembler_input_layout = true},
};

const gpu::root_signature_desc terrain_desc{
   .name = "terrain_root_signature",

   .parameters =
      {
         // camera descriptor
         gpu::root_parameter_descriptor_table{
            .ranges =
               {
                  {.type = gpu::descriptor_range_type::cbv,
                   .count = 1,
                   .base_shader_register = 0,
                   .register_space = mesh_register_space,
                   .offset_in_descriptors_from_table_start = 0},
               },
            .visibility = gpu::shader_visibility::all,
         },

         // lights descriptors
         lights_input_descriptor_table,

         // terrain descriptors
         gpu::root_parameter_descriptor_table{
            .ranges =
               {
                  {.type = gpu::descriptor_range_type::cbv,
                   .count = 1,
                   .base_shader_register = 0,
                   .register_space = terrain_register_space,
                   .offset_in_descriptors_from_table_start = 0},

                  {.type = gpu::descriptor_range_type::srv,
                   .count = 2,
                   .base_shader_register = 0,
                   .register_space = terrain_register_space,
                   .offset_in_descriptors_from_table_start = 1},
               },
            .visibility = gpu::shader_visibility::all,
         },

         // terrain patch data
         gpu::root_parameter_srv{
            .shader_register = 2,
            .register_space = terrain_register_space,
            .visibility = gpu::shader_visibility::vertex,
         },

         // material descriptors
         gpu::root_parameter_descriptor_table{
            .ranges =
               {
                  {.type = gpu::descriptor_range_type::srv,
                   .count = 16,
                   .base_shader_register = 0,
                   .register_space = material_register_space,
                   .offset_in_descriptors_from_table_start = 0},
               },
            .visibility = gpu::shader_visibility::pixel,
         },
      },

   .samplers =
      {
         {.sampler = bilinear_sampler,
          .shader_register = 0,
          .register_space = 0,
          .visibility = gpu::shader_visibility::pixel},

         {.sampler = trilinear_sampler,
          .shader_register = 1,
          .register_space = 0,
          .visibility = gpu::shader_visibility::pixel},

         {.sampler = shadow_sampler,
          .shader_register = 2,
          .register_space = lights_register_space,
          .visibility = gpu::shader_visibility::pixel},
      },

   .flags = {},
};

const gpu::root_signature_desc meta_mesh_desc{
   .name = "meta_mesh_root_signature",

   .parameters =
      {
         // per-object constants
         gpu::root_parameter_cbv{
            .shader_register = 1,
            .register_space = mesh_register_space,
            .visibility = gpu::shader_visibility::vertex,
         },

         // color constant (should this be a root constant?)
         gpu::root_parameter_cbv{
            .shader_register = 0,
            .register_space = mesh_register_space,
            .visibility = gpu::shader_visibility::pixel,
         },

         // camera descriptors
         gpu::root_parameter_descriptor_table{
            .ranges =
               {
                  {.type = gpu::descriptor_range_type::cbv,
                   .count = 1,
                   .base_shader_register = 0,
                   .register_space = mesh_register_space,
                   .offset_in_descriptors_from_table_start = 0},
               },
            .visibility = gpu::shader_visibility::vertex,
         },
      },

   .flags = {.allow_input_assembler_input_layout = true},
};

const gpu::root_signature_desc meta_mesh_wireframe_desc{
   .name = "meta_mesh_wireframe_root_signature",

   .parameters =
      {
         // per-object constants
         gpu::root_parameter_cbv{
            .shader_register = 1,
            .register_space = mesh_register_space,
            .visibility = gpu::shader_visibility::vertex,
         },

         // wireframe constants
         gpu::root_parameter_cbv{
            .shader_register = 1,
            .register_space = 0,
            .visibility = gpu::shader_visibility::pixel,
         },

         // camera descriptors
         gpu::root_parameter_descriptor_table{
            .ranges =
               {
                  {.type = gpu::descriptor_range_type::cbv,
                   .count = 1,
                   .base_shader_register = 0,
                   .register_space = mesh_register_space,
                   .offset_in_descriptors_from_table_start = 0},
               },
            .visibility = gpu::shader_visibility::all,
         },
      },

   .flags = {.allow_input_assembler_input_layout = true},
};

const gpu::root_signature_desc meta_line_desc{
   .name = "meta_line_root_signature",

   .parameters =
      {
         // color constant (should this be a root constant?)
         gpu::root_parameter_cbv{
            .shader_register = 0,
            .register_space = mesh_register_space,
            .visibility = gpu::shader_visibility::pixel,
         },

         // camera descriptors
         gpu::root_parameter_descriptor_table{
            .ranges =
               {
                  {.type = gpu::descriptor_range_type::cbv,
                   .count = 1,
                   .base_shader_register = 0,
                   .register_space = mesh_register_space,
                   .offset_in_descriptors_from_table_start = 0},
               },
            .visibility = gpu::shader_visibility::vertex,
         },
      },

   .flags = {.allow_input_assembler_input_layout = true},
};

const gpu::root_signature_desc mesh_shadow_desc{
   .name = "mesh_shadow_root_signature",

   .parameters =
      {
         // transform cbv
         gpu::root_parameter_cbv{
            .shader_register = 0,
            .register_space = mesh_register_space,
            .visibility = gpu::shader_visibility::vertex,
         },

         // camera cbv
         gpu::root_parameter_cbv{
            .shader_register = 1,
            .register_space = mesh_register_space,
            .visibility = gpu::shader_visibility::vertex,
         },
      },

   .flags = {.allow_input_assembler_input_layout = true},
};

const gpu::root_signature_desc mesh_depth_prepass_desc{
   .name = "mesh_depth_prepass_root_signature",

   .parameters =
      {// per-object constants
       gpu::root_parameter_cbv{
          .shader_register = 1,
          .register_space = mesh_register_space,
          .visibility = gpu::shader_visibility::vertex,
       },

       // material constants
       gpu::root_parameter_descriptor_table{
          .ranges =
             {
                {.type = gpu::descriptor_range_type::cbv,
                 .count = 1,
                 .base_shader_register = 0,
                 .register_space = material_register_space,
                 .offset_in_descriptors_from_table_start = 0},
             },
          .visibility = gpu::shader_visibility::pixel,
       },

       // camera descriptors
       gpu::root_parameter_descriptor_table{
          .ranges =
             {
                {.type = gpu::descriptor_range_type::cbv,
                 .count = 1,
                 .base_shader_register = 0,
                 .register_space = mesh_register_space,
                 .offset_in_descriptors_from_table_start = 0},
             },
          .visibility = gpu::shader_visibility::vertex,
       },

       // bindless descriptors
       bindless_srv_table},

   .samplers =
      {
         {.sampler = trilinear_sampler,
          .shader_register = 0,
          .register_space = 0,
          .visibility = gpu::shader_visibility::pixel},
      },

   .flags = {.allow_input_assembler_input_layout = true},
};

const gpu::root_signature_desc mesh_wireframe_desc{
   .name = "mesh_wireframe_root_signature",

   .parameters =
      {
         // per-object constants
         gpu::root_parameter_cbv{
            .shader_register = 1,
            .register_space = mesh_register_space,
            .visibility = gpu::shader_visibility::vertex,
         },

         // wireframe constants
         gpu::root_parameter_cbv{
            .shader_register = 1,
            .register_space = 0,
            .visibility = gpu::shader_visibility::pixel,
         },

         // camera descriptors
         gpu::root_parameter_descriptor_table{
            .ranges =
               {
                  {.type = gpu::descriptor_range_type::cbv,
                   .count = 1,
                   .base_shader_register = 0,
                   .register_space = mesh_register_space,
                   .offset_in_descriptors_from_table_start = 0},
               },
            .visibility = gpu::shader_visibility::all,
         },
      },

   .flags = {.allow_input_assembler_input_layout = true},
};

const gpu::root_signature_desc
   tile_lights_clear_desc{.name = "tile_lights_clear_root_signature",

                          .parameters = {
                             // input cbv
                             gpu::root_parameter_cbv{.shader_register = 0},

                             // tiles uav
                             gpu::root_parameter_uav{.shader_register = 0},
                          }};

const gpu::root_signature_desc tile_lights_desc{
   .name = "tile_lights_root_signature",

   .parameters =
      {
         // instance data srv
         gpu::root_parameter_srv{.shader_register = 0},

         // descriptors
         gpu::root_parameter_descriptor_table{
            .ranges = {{.type = gpu::descriptor_range_type::cbv,
                        .count = 1,
                        .base_shader_register = 0,
                        .offset_in_descriptors_from_table_start = 0},

                       {.type = gpu::descriptor_range_type::uav,
                        .count = 1,
                        .base_shader_register = 0,
                        .offset_in_descriptors_from_table_start =
                           gpu::descriptor_range_offset_append}},
         },
      },

   .flags = {.allow_input_assembler_input_layout = true},
};

}

root_signature_library::root_signature_library(gpu::device& device)
{
   mesh = device.create_root_signature(mesh_desc);
   terrain = device.create_root_signature(terrain_desc);
   meta_mesh = device.create_root_signature(meta_mesh_desc);
   meta_mesh_wireframe = device.create_root_signature(meta_mesh_wireframe_desc);
   meta_line = device.create_root_signature(meta_line_desc);
   mesh_shadow = device.create_root_signature(mesh_shadow_desc);
   mesh_depth_prepass = device.create_root_signature(mesh_depth_prepass_desc);
   mesh_wireframe = device.create_root_signature(mesh_wireframe_desc);

   tile_lights_clear = device.create_root_signature(tile_lights_clear_desc);
   tile_lights = device.create_root_signature(tile_lights_desc);
}

}
