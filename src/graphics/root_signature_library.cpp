
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

const gpu::static_sampler_desc
   trilinear_sampler{.filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                     .address_u = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                     .address_v = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                     .address_w = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                     .mip_lod_bias = 0.0f,
                     .max_anisotropy = 0,
                     .comparison_func = D3D12_COMPARISON_FUNC_ALWAYS,
                     .border_color = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
                     .min_lod = 0.0f,
                     .max_lod = D3D12_FLOAT32_MAX};

const gpu::static_sampler_desc
   bilinear_sampler{.filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
                    .address_u = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                    .address_v = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                    .address_w = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                    .mip_lod_bias = 0.0f,
                    .max_anisotropy = 0,
                    .comparison_func = D3D12_COMPARISON_FUNC_ALWAYS,
                    .border_color = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
                    .min_lod = 0.0f,
                    .max_lod = D3D12_FLOAT32_MAX};

const gpu::static_sampler_desc
   shadow_sampler{.filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
                  .address_u = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                  .address_v = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                  .address_w = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                  .mip_lod_bias = 0.0f,
                  .max_anisotropy = 0,
                  .comparison_func = D3D12_COMPARISON_FUNC_LESS_EQUAL,
                  .border_color = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
                  .min_lod = 0.0f,
                  .max_lod = D3D12_FLOAT32_MAX};

const gpu::root_signature_desc object_mesh_desc{
   .name = "mesh_root_signature",

   .parameters =
      {
         // per-object constants
         gpu::root_parameter_cbv{
            .shader_register = 1,
            .register_space = mesh_register_space,
            .flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
         },

         // per-object material descriptors
         gpu::root_parameter_descriptor_table{
            .ranges =
               {
                  {.type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                   .count = 2,
                   .base_shader_register = 0,
                   .register_space = material_register_space,
                   .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                   .offset_in_descriptors_from_table_start = 0},
               },
            .visibility = D3D12_SHADER_VISIBILITY_PIXEL,
         },

         // camera descriptors
         gpu::root_parameter_descriptor_table{
            .ranges =
               {
                  {.type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                   .count = 1,
                   .base_shader_register = 0,
                   .register_space = mesh_register_space,
                   .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                   .offset_in_descriptors_from_table_start = 0},
               },
            .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
         },

         // lights descriptors
         gpu::root_parameter_descriptor_table{
            .ranges =
               {
                  {.type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                   .count = 1,
                   .base_shader_register = 0,
                   .register_space = lights_register_space,
                   .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                   .offset_in_descriptors_from_table_start = 0},

                  {.type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                   .count = 2,
                   .base_shader_register = 0,
                   .register_space = lights_register_space,
                   .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                   .offset_in_descriptors_from_table_start = 1},
               },
            .visibility = D3D12_SHADER_VISIBILITY_PIXEL,
         },
      },

   .samplers =
      {
         {.sampler = trilinear_sampler,
          .shader_register = 0,
          .register_space = 0,
          .visibility = D3D12_SHADER_VISIBILITY_PIXEL},

         {.sampler = shadow_sampler,
          .shader_register = 2,
          .register_space = lights_register_space,
          .visibility = D3D12_SHADER_VISIBILITY_PIXEL},
      },

   .flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT};

const gpu::root_signature_desc
   terrain_desc{.name = "terrain_root_signature",

                .parameters =
                   {
                      // camera descriptor
                      gpu::root_parameter_descriptor_table{
                         .ranges =
                            {
                               {.type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                                .count = 1,
                                .base_shader_register = 0,
                                .register_space = mesh_register_space,
                                .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                                .offset_in_descriptors_from_table_start = 0},
                            },
                         .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
                      },

                      // lights descriptors
                      gpu::root_parameter_descriptor_table{
                         .ranges =
                            {
                               {.type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                                .count = 1,
                                .base_shader_register = 0,
                                .register_space = lights_register_space,
                                .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                                .offset_in_descriptors_from_table_start = 0},

                               {.type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                                .count = 2,
                                .base_shader_register = 0,
                                .register_space = lights_register_space,
                                .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                                .offset_in_descriptors_from_table_start = 1},
                            },
                         .visibility = D3D12_SHADER_VISIBILITY_PIXEL,
                      },

                      // terrain descriptors
                      gpu::root_parameter_descriptor_table{
                         .ranges =
                            {
                               {.type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                                .count = 1,
                                .base_shader_register = 0,
                                .register_space = terrain_register_space,
                                .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                                .offset_in_descriptors_from_table_start = 0},

                               {.type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                                .count = 2,
                                .base_shader_register = 0,
                                .register_space = terrain_register_space,
                                .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                                .offset_in_descriptors_from_table_start = 1},
                            },
                         .visibility = D3D12_SHADER_VISIBILITY_ALL,
                      },

                      // terrain patch data
                      gpu::root_parameter_srv{
                         .shader_register = 2,
                         .register_space = terrain_register_space,
                         .flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
                         .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
                      },

                      // material descriptors
                      gpu::root_parameter_descriptor_table{
                         .ranges =
                            {
                               {.type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                                .count = 16,
                                .base_shader_register = 0,
                                .register_space = material_register_space,
                                .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                                .offset_in_descriptors_from_table_start = 0},
                            },
                         .visibility = D3D12_SHADER_VISIBILITY_PIXEL,
                      },
                   },

                .samplers =
                   {
                      {.sampler = bilinear_sampler,
                       .shader_register = 0,
                       .register_space = 0,
                       .visibility = D3D12_SHADER_VISIBILITY_PIXEL},

                      {.sampler = trilinear_sampler,
                       .shader_register = 1,
                       .register_space = 0,
                       .visibility = D3D12_SHADER_VISIBILITY_PIXEL},

                      {.sampler = shadow_sampler,
                       .shader_register = 2,
                       .register_space = lights_register_space,
                       .visibility = D3D12_SHADER_VISIBILITY_PIXEL},
                   },

                .flags = D3D12_ROOT_SIGNATURE_FLAG_NONE};

const gpu::root_signature_desc meta_object_desc{
   .name = "meta_object_root_signature",

   .parameters =
      {
         // per-object constants
         gpu::root_parameter_cbv{
            .shader_register = 1,
            .register_space = mesh_register_space,
            .flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
         },

         // color constant (should this be a root constant?)
         gpu::root_parameter_cbv{
            .shader_register = 0,
            .register_space = mesh_register_space,
            .flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            .visibility = D3D12_SHADER_VISIBILITY_PIXEL,
         },

         // camera descriptors
         gpu::root_parameter_descriptor_table{
            .ranges =
               {
                  {.type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                   .count = 1,
                   .base_shader_register = 0,
                   .register_space = mesh_register_space,
                   .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                   .offset_in_descriptors_from_table_start = 0},
               },
            .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
         },
      },

   .flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT};

const gpu::root_signature_desc meta_line_desc{
   .name = "meta_line_root_signature",

   .parameters =
      {
         // color constant (should this be a root constant?)
         gpu::root_parameter_cbv{
            .shader_register = 0,
            .register_space = mesh_register_space,
            .flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            .visibility = D3D12_SHADER_VISIBILITY_PIXEL,
         },

         // camera descriptors
         gpu::root_parameter_descriptor_table{
            .ranges =
               {
                  {.type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                   .count = 1,
                   .base_shader_register = 0,
                   .register_space = mesh_register_space,
                   .flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
                   .offset_in_descriptors_from_table_start = 0},
               },
            .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
         },
      },

   .flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT};

const gpu::root_signature_desc shadow_mesh_desc{
   .name = "shadow_mesh_root_signature",

   .parameters =
      {
         // transform cbv
         gpu::root_parameter_cbv{
            .shader_register = 0,
            .register_space = mesh_register_space,
            .flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
         },

         // camera cbv
         gpu::root_parameter_cbv{
            .shader_register = 1,
            .register_space = mesh_register_space,
            .flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
            .visibility = D3D12_SHADER_VISIBILITY_VERTEX,
         },
      },

   .flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT};

}

root_signature_library::root_signature_library(gpu::device& device)
{
   object_mesh = device.create_root_signature(object_mesh_desc);
   terrain = device.create_root_signature(terrain_desc);
   meta_object_mesh = device.create_root_signature(meta_object_desc);
   meta_line = device.create_root_signature(meta_line_desc);
   depth_only_mesh = device.create_root_signature(shadow_mesh_desc);
}

}
