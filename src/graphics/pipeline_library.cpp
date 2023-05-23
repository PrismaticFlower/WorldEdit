
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "shader_library.hpp"

#include <array>
#include <stdexcept>

#include <fmt/core.h>

using namespace std::literals;

namespace we::graphics {

namespace {

constexpr gpu::blend_state_desc blend_disabled = {
   .render_target = {gpu::render_target_blend::disabled},
};

constexpr gpu::blend_state_desc blend_premult_alpha = {
   .render_target = {gpu::render_target_blend::premult_alpha_blend},
};

constexpr gpu::blend_state_desc blend_alpha = {
   .render_target = {gpu::render_target_blend::alpha_belnd},
};

constexpr gpu::blend_state_desc blend_additive = {
   .render_target = {gpu::render_target_blend::additive_blend},
};

constexpr gpu::rasterizer_state_desc rasterizer_cull_none = {
   .cull_mode = gpu::cull_mode::none,
};

constexpr gpu::rasterizer_state_desc rasterizer_cull_backfacing = {
   .cull_mode = gpu::cull_mode::back,
};

constexpr gpu::rasterizer_state_desc rasterizer_cull_frontfacing = {
   .cull_mode = gpu::cull_mode::front,
};

constexpr gpu::rasterizer_state_desc rasterizer_line_antialiased = {
   .cull_mode = gpu::cull_mode::none,
   .antialiased_lines = true,
};

constexpr gpu::depth_stencil_state_desc depth_stencil_enabled = {
   .depth_test_enabled = true,
   .depth_test_func = gpu::comparison_func::less_equal,
};

constexpr gpu::depth_stencil_state_desc depth_stencil_readonly_equal = {
   .depth_test_enabled = true,
   .depth_test_func = gpu::comparison_func::equal,
   .write_depth = false,
};

constexpr gpu::depth_stencil_state_desc depth_stencil_readonly_less_equal = {
   .depth_test_enabled = true,
   .depth_test_func = gpu::comparison_func::less_equal,
   .write_depth = false,
};

constexpr gpu::depth_stencil_state_desc depth_stencil_disabled = {
   .depth_test_enabled = false,
};

constexpr std::array mesh_input_layout = {
   gpu::input_element_desc{.semantic_name = "POSITION",
                           .format = DXGI_FORMAT_R32G32B32_FLOAT,
                           .input_slot = 0},
   gpu::input_element_desc{.semantic_name = "NORMAL",
                           .format = DXGI_FORMAT_R16G16B16A16_SNORM,
                           .input_slot = 1},
   gpu::input_element_desc{.semantic_name = "TANGENT",
                           .format = DXGI_FORMAT_R16G16B16A16_SNORM,
                           .input_slot = 1},
   gpu::input_element_desc{.semantic_name = "BITANGENT",
                           .format = DXGI_FORMAT_R16G16B16A16_SNORM,
                           .input_slot = 1},
   gpu::input_element_desc{.semantic_name = "TEXCOORD",
                           .format = DXGI_FORMAT_R32G32_FLOAT,
                           .input_slot = 1},
   gpu::input_element_desc{.semantic_name = "COLOR",
                           .format = DXGI_FORMAT_B8G8R8A8_UNORM,
                           .input_slot = 1},
};

constexpr std::array mesh_input_layout_position_only = {
   mesh_input_layout[0],
};

constexpr std::array meta_draw_input_layout = {
   gpu::input_element_desc{.semantic_name = "POSITION",
                           .format = DXGI_FORMAT_R32G32B32_FLOAT},
};

constexpr std::array meta_draw_primitive_input_layout = {
   gpu::input_element_desc{.semantic_name = "POSITION",
                           .format = DXGI_FORMAT_R32G32B32_FLOAT},
   gpu::input_element_desc{.semantic_name = "COLOR", .format = DXGI_FORMAT_B8G8R8A8_UNORM},
};

constexpr std::array imgui_input_layout = {
   gpu::input_element_desc{.semantic_name = "POSITION", .format = DXGI_FORMAT_R32G32_FLOAT},
   gpu::input_element_desc{.semantic_name = "TEXCOORDS", .format = DXGI_FORMAT_R32G32_FLOAT},
   gpu::input_element_desc{.semantic_name = "COLOR", .format = DXGI_FORMAT_R8G8B8A8_UNORM},
};

auto create_material_pipelines(gpu::device& device, const std::string_view name_base,
                               const shader_library& shader_library,
                               const root_signature_library& root_signature_library)
   -> material_pipelines
{
   material_pipelines pipelines;

   for (auto i = 0u; i < pipelines.size(); ++i) {
      const auto flags = static_cast<material_pipeline_flags>(i);

      const bool transparent =
         are_flags_set(flags, material_pipeline_flags::transparent);
      const bool additive = are_flags_set(flags, material_pipeline_flags::additive);
      const bool doublesided =
         are_flags_set(flags, material_pipeline_flags::doublesided);

      const gpu::blend_state_desc blend_state = [&] {
         if (additive) {
            return blend_additive;
         }
         else if (transparent) {
            return blend_premult_alpha;
         }

         return blend_disabled;
      }();

      const gpu::rasterizer_state_desc rasterizer_state =
         doublesided ? rasterizer_cull_none : rasterizer_cull_backfacing;

      const gpu::depth_stencil_state_desc depth_stencil_state =
         transparent ? depth_stencil_readonly_less_equal : depth_stencil_readonly_equal;

      const std::string pipeline_name =
         fmt::format("{}{}{}{}", name_base, transparent ? "_transparent"sv : ""sv,
                     additive ? "_additive"sv : ""sv,
                     doublesided ? "_doublesided"sv : ""sv);

      pipelines[i] = {device.create_graphics_pipeline(
                         {.root_signature = root_signature_library.mesh.get(),

                          .vs_bytecode = shader_library["meshVS"sv],
                          .ps_bytecode = shader_library["mesh_normalPS"sv],

                          .blend_state = blend_state,
                          .rasterizer_state = rasterizer_state,
                          .depth_stencil_state = depth_stencil_state,
                          .input_layout = mesh_input_layout,

                          .render_target_count = 1,
                          .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
                          .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

                          .debug_name = pipeline_name}),
                      device.direct_queue};
   }

   return pipelines;
}

auto create_depth_prepass_pipelines(gpu::device& device, const std::string_view name_base,
                                    const shader_library& shader_library,
                                    const root_signature_library& root_signature_library)
   -> depth_prepass_pipelines
{
   depth_prepass_pipelines pipelines;

   for (auto i = 0u; i < pipelines.size(); ++i) {
      const auto flags = static_cast<depth_prepass_pipeline_flags>(i);

      const bool alpha_cutout =
         are_flags_set(flags, depth_prepass_pipeline_flags::alpha_cutout);

      const bool doublesided =
         are_flags_set(flags, depth_prepass_pipeline_flags::doublesided);

      const std::string pipeline_name =
         fmt::format("{}{}{}", name_base, alpha_cutout ? "_alpha_cutout"sv : ""sv,
                     doublesided ? "_doublesided"sv : ""sv);

      pipelines[i] =
         {device.create_graphics_pipeline(
             {.root_signature = root_signature_library.mesh_depth_prepass.get(),

              .vs_bytecode = alpha_cutout ? shader_library["meshVS"sv]
                                          : shader_library["mesh_depth_prepassVS"sv],
              .ps_bytecode = alpha_cutout ? shader_library["mesh_depth_cutoutPS"sv]
                                          : std::span<const std::byte>{},

              .rasterizer_state = doublesided ? rasterizer_cull_none : rasterizer_cull_backfacing,
              .depth_stencil_state = depth_stencil_enabled,
              .input_layout =
                 alpha_cutout
                    ? std::span<const gpu::input_element_desc>{mesh_input_layout}
                    : std::span<const gpu::input_element_desc>{mesh_input_layout_position_only},

              .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

              .debug_name = pipeline_name}),
          device.direct_queue};
   }

   return pipelines;
}

auto create_shadow_pipelines(gpu::device& device, const std::string_view name_base,
                             const shader_library& shader_library,
                             const root_signature_library& root_signature_library)
   -> depth_prepass_pipelines
{
   depth_prepass_pipelines pipelines;

   for (auto i = 0u; i < pipelines.size(); ++i) {
      const auto flags = static_cast<depth_prepass_pipeline_flags>(i);

      const bool alpha_cutout =
         are_flags_set(flags, depth_prepass_pipeline_flags::alpha_cutout);

      const bool doublesided =
         are_flags_set(flags, depth_prepass_pipeline_flags::doublesided);

      const std::string pipeline_name =
         fmt::format("{}{}{}", name_base, alpha_cutout ? "_alpha_cutout"sv : ""sv,
                     doublesided ? "_doublesided"sv : ""sv);

      pipelines[i] =
         {device.create_graphics_pipeline(
             {.root_signature = root_signature_library.mesh_shadow.get(),

              .vs_bytecode = alpha_cutout ? shader_library["mesh_shadow_cutoutVS"sv]
                                          : shader_library["mesh_shadowVS"sv],
              .ps_bytecode = alpha_cutout ? shader_library["mesh_shadow_cutoutPS"sv]
                                          : std::span<const std::byte>{},

              .rasterizer_state = doublesided ? rasterizer_cull_none : rasterizer_cull_backfacing,
              .depth_stencil_state = depth_stencil_enabled,
              .input_layout =
                 alpha_cutout
                    ? std::span<const gpu::input_element_desc>{mesh_input_layout}
                    : std::span<const gpu::input_element_desc>{mesh_input_layout_position_only},

              .dsv_format = DXGI_FORMAT_D32_FLOAT,

              .debug_name = pipeline_name}),
          device.direct_queue};
   }

   return pipelines;
}

}

pipeline_library::pipeline_library(gpu::device& device,
                                   const shader_library& shader_library,
                                   const root_signature_library& root_signature_library)

{
   reload(device, shader_library, root_signature_library);
}

void pipeline_library::reload(gpu::device& device, const shader_library& shader_library,
                              const root_signature_library& root_signature_library)
{
   mesh_basic = {device.create_graphics_pipeline(
                    {.root_signature = root_signature_library.mesh.get(),

                     .vs_bytecode = shader_library["meshVS"sv],
                     .ps_bytecode = shader_library["mesh_basicPS"sv],

                     .rasterizer_state = rasterizer_cull_backfacing,
                     .depth_stencil_state = depth_stencil_enabled,
                     .input_layout = mesh_input_layout,

                     .render_target_count = 1,
                     .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
                     .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

                     .debug_name = "mesh_basic"sv}),
                 device.direct_queue};

   mesh_basic_lighting = {device.create_graphics_pipeline(
                             {.root_signature = root_signature_library.mesh.get(),

                              .vs_bytecode = shader_library["meshVS"sv],
                              .ps_bytecode = shader_library["mesh_basic_lightingPS"sv],

                              .rasterizer_state = rasterizer_cull_backfacing,
                              .depth_stencil_state = depth_stencil_enabled,

                              .input_layout = mesh_input_layout,

                              .render_target_count = 1,
                              .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
                              .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

                              .debug_name = "mesh_basic_lighting"sv}),
                          device.direct_queue};

   mesh_shadow = create_shadow_pipelines(device, "mesh_shadow"sv,
                                         shader_library, root_signature_library);

   mesh_depth_prepass =
      create_depth_prepass_pipelines(device, "mesh_depth_prepass"sv,
                                     shader_library, root_signature_library);

   mesh_normal = create_material_pipelines(device, "mesh_normal"sv, shader_library,
                                           root_signature_library);

   const bool supports_shader_barycentrics = device.supports_shader_barycentrics();

   mesh_wireframe =
      {device.create_graphics_pipeline(
          {.root_signature = root_signature_library.mesh_wireframe.get(),

           .vs_bytecode = shader_library["mesh_wireframeVS"sv],
           .ps_bytecode = supports_shader_barycentrics
                             ? shader_library["mesh_wireframePS"sv]
                             : shader_library["mesh_wireframe_GS_fallbackPS"sv],
           .gs_bytecode = supports_shader_barycentrics
                             ? std::span<const std::byte>{}
                             : shader_library["mesh_wireframe_GS_fallbackGS"sv],

           .blend_state = blend_alpha,
           .rasterizer_state = rasterizer_cull_backfacing,
           .depth_stencil_state = depth_stencil_readonly_less_equal,
           .input_layout = mesh_input_layout_position_only,

           .render_target_count = 1,
           .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
           .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

           .debug_name = "mesh_wireframe"sv}),
       device.direct_queue};

   mesh_wireframe_doublesided =
      {device.create_graphics_pipeline(
          {.root_signature = root_signature_library.mesh_wireframe.get(),

           .vs_bytecode = shader_library["mesh_wireframeVS"sv],
           .ps_bytecode = supports_shader_barycentrics
                             ? shader_library["mesh_wireframePS"sv]
                             : shader_library["mesh_wireframe_GS_fallbackPS"sv],
           .gs_bytecode = supports_shader_barycentrics
                             ? std::span<const std::byte>{}
                             : shader_library["mesh_wireframe_GS_fallbackGS"sv],

           .blend_state = blend_alpha,
           .rasterizer_state = rasterizer_cull_none,
           .depth_stencil_state = depth_stencil_readonly_less_equal,
           .input_layout = mesh_input_layout_position_only,

           .render_target_count = 1,
           .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
           .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

           .debug_name = "mesh_wireframe_doublesided"sv}),
       device.direct_queue};

   terrain_depth_prepass = {device.create_graphics_pipeline(
                               {.root_signature = root_signature_library.terrain.get(),

                                .vs_bytecode = shader_library["terrain_patchVS"sv],

                                .rasterizer_state = rasterizer_cull_backfacing,
                                .depth_stencil_state = depth_stencil_enabled,

                                .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

                                .debug_name = "terrain_depth_prepass"sv}),
                            device.direct_queue};

   terrain_basic = {device.create_graphics_pipeline(
                       {.root_signature = root_signature_library.terrain.get(),

                        .vs_bytecode = shader_library["terrain_patchVS"sv],
                        .ps_bytecode = shader_library["terrain_basicPS"sv],

                        .rasterizer_state = rasterizer_cull_backfacing,
                        .depth_stencil_state = depth_stencil_readonly_equal,

                        .render_target_count = 1,
                        .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
                        .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

                        .debug_name = "terrain_basic"sv}),
                    device.direct_queue};

   terrain_lighting = {device.create_graphics_pipeline(
                          {.root_signature = root_signature_library.terrain.get(),

                           .vs_bytecode = shader_library["terrain_patchVS"sv],
                           .ps_bytecode = shader_library["terrain_lightingPS"sv],

                           .rasterizer_state = rasterizer_cull_backfacing,
                           .depth_stencil_state = depth_stencil_readonly_equal,

                           .render_target_count = 1,
                           .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
                           .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

                           .debug_name = "terrain_lighting"sv}),
                       device.direct_queue};

   terrain_normal = {device.create_graphics_pipeline(
                        {.root_signature = root_signature_library.terrain.get(),

                         .vs_bytecode = shader_library["terrain_patchVS"sv],
                         .ps_bytecode = shader_library["terrain_normalPS"sv],

                         .rasterizer_state = rasterizer_cull_backfacing,
                         .depth_stencil_state = depth_stencil_readonly_equal,

                         .render_target_count = 1,
                         .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
                         .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

                         .debug_name = "terrain_normal"sv}),
                     device.direct_queue};

   sky_mesh = {device.create_graphics_pipeline(
                  {.root_signature = root_signature_library.sky_mesh.get(),

                   .vs_bytecode = shader_library["sky_meshVS"sv],
                   .ps_bytecode = shader_library["sky_meshPS"sv],

                   .blend_state = blend_premult_alpha,
                   .rasterizer_state = rasterizer_cull_backfacing,
                   .depth_stencil_state = depth_stencil_readonly_less_equal,
                   .input_layout = mesh_input_layout,

                   .render_target_count = 1,
                   .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
                   .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

                   .debug_name = "sky_mesh"sv}),
               device.direct_queue};

   meta_draw_shape = {device.create_graphics_pipeline(
                         {.root_signature = root_signature_library.meta_draw.get(),

                          .vs_bytecode = shader_library["meta_draw_shapeVS"sv],
                          .ps_bytecode = shader_library["meta_drawPS"sv],

                          .blend_state = blend_additive,
                          .rasterizer_state = rasterizer_cull_backfacing,
                          .depth_stencil_state = depth_stencil_readonly_less_equal,
                          .input_layout = meta_draw_input_layout,

                          .render_target_count = 1,
                          .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
                          .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

                          .debug_name = "meta_draw_shape"sv}),
                      device.direct_queue};

   meta_draw_shape_outlined =
      {device.create_graphics_pipeline(
          {.root_signature = root_signature_library.meta_draw.get(),

           .vs_bytecode = shader_library["meta_draw_shape_outlinedVS"sv],
           .ps_bytecode = supports_shader_barycentrics
                             ? shader_library["meta_draw_outlinedPS"sv]
                             : shader_library["meta_draw_outlined_GS_fallbackPS"sv],
           .gs_bytecode =
              supports_shader_barycentrics
                 ? std::span<const std::byte>{}
                 : shader_library["meta_draw_outlined_GS_fallbackGS"],

           .rasterizer_state = rasterizer_cull_backfacing,
           .depth_stencil_state = depth_stencil_enabled,
           .input_layout = meta_draw_input_layout,

           .render_target_count = 1,
           .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
           .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

           .debug_name = "meta_draw_shape_outlined"sv}),
       device.direct_queue};

   meta_draw_shape_wireframe =
      {device.create_graphics_pipeline(
          {.root_signature = root_signature_library.meta_draw.get(),

           .vs_bytecode = shader_library["meta_draw_shapeVS"sv],
           .ps_bytecode = supports_shader_barycentrics
                             ? shader_library["meta_draw_wireframePS"sv]
                             : shader_library["meta_draw_wireframe_GS_fallbackPS"sv],
           .gs_bytecode = supports_shader_barycentrics
                             ? std::span<const std::byte>{}
                             : shader_library["meta_draw_wireframe_GS_fallbackGS"sv],

           .blend_state = blend_alpha,
           .rasterizer_state = rasterizer_cull_backfacing,
           .depth_stencil_state = depth_stencil_readonly_less_equal,
           .input_layout = meta_draw_input_layout,

           .render_target_count = 1,
           .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
           .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

           .debug_name = "meta_draw_shape_wireframe"sv}),
       device.direct_queue};

   meta_draw_sphere = {device.create_graphics_pipeline(
                          {.root_signature = root_signature_library.meta_draw.get(),

                           .vs_bytecode = shader_library["meta_draw_sphereVS"sv],
                           .ps_bytecode = shader_library["meta_drawPS"sv],

                           .blend_state = blend_additive,
                           .rasterizer_state = rasterizer_cull_backfacing,
                           .depth_stencil_state = depth_stencil_readonly_less_equal,
                           .input_layout = meta_draw_input_layout,

                           .render_target_count = 1,
                           .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
                           .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

                           .debug_name = "meta_draw_sphere"sv}),
                       device.direct_queue};

   meta_draw_sphere_wireframe =
      {device.create_graphics_pipeline(
          {.root_signature = root_signature_library.meta_draw.get(),

           .vs_bytecode = shader_library["meta_draw_sphereVS"sv],
           .ps_bytecode = supports_shader_barycentrics
                             ? shader_library["meta_draw_wireframePS"sv]
                             : shader_library["meta_draw_wireframe_GS_fallbackPS"sv],
           .gs_bytecode = supports_shader_barycentrics
                             ? std::span<const std::byte>{}
                             : shader_library["meta_draw_wireframe_GS_fallbackGS"sv],

           .blend_state = blend_alpha,
           .rasterizer_state = rasterizer_cull_backfacing,
           .depth_stencil_state = depth_stencil_readonly_less_equal,
           .input_layout = meta_draw_input_layout,

           .render_target_count = 1,
           .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
           .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

           .debug_name = "meta_draw_sphere_wireframe"sv}),
       device.direct_queue};

   meta_draw_line_solid = {device.create_graphics_pipeline(
                              {.root_signature =
                                  root_signature_library.meta_draw.get(),

                               .vs_bytecode = shader_library["meta_draw_lineVS"sv],
                               .ps_bytecode = shader_library["meta_draw_linePS"sv],

                               .blend_state = blend_alpha,
                               .rasterizer_state = rasterizer_cull_none,
                               .depth_stencil_state = depth_stencil_readonly_less_equal,
                               .primitive_type = gpu::primitive_type::triangle,

                               .render_target_count = 1,
                               .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
                               .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

                               .debug_name = "meta_draw_line_solid"sv}),
                           device.direct_queue};

   meta_draw_line_overlay = {device.create_graphics_pipeline(
                                {.root_signature =
                                    root_signature_library.meta_draw.get(),

                                 .vs_bytecode = shader_library["meta_draw_lineVS"sv],
                                 .ps_bytecode = shader_library["meta_draw_linePS"sv],

                                 .blend_state = blend_alpha,
                                 .rasterizer_state = rasterizer_cull_none,
                                 .depth_stencil_state = depth_stencil_disabled,
                                 .primitive_type = gpu::primitive_type::triangle,

                                 .render_target_count = 1,
                                 .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
                                 .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

                                 .debug_name = "meta_draw_line_overlay"sv}),
                             device.direct_queue};

   meta_draw_triangle = {device.create_graphics_pipeline(
                            {.root_signature = root_signature_library.meta_draw.get(),

                             .vs_bytecode = shader_library["meta_draw_primitiveVS"sv],
                             .ps_bytecode = shader_library["meta_drawPS"sv],

                             .blend_state = blend_additive,
                             .rasterizer_state = rasterizer_cull_backfacing,
                             .depth_stencil_state = depth_stencil_readonly_less_equal,
                             .input_layout = meta_draw_primitive_input_layout,

                             .render_target_count = 1,
                             .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
                             .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

                             .debug_name = "meta_draw_triangle"sv}),
                         device.direct_queue};

   meta_draw_triangle_wireframe =
      {device.create_graphics_pipeline(
          {.root_signature = root_signature_library.meta_draw.get(),

           .vs_bytecode = shader_library["meta_draw_primitiveVS"sv],
           .ps_bytecode = supports_shader_barycentrics
                             ? shader_library["meta_draw_wireframePS"sv]
                             : shader_library["meta_draw_wireframe_GS_fallbackPS"sv],
           .gs_bytecode = supports_shader_barycentrics
                             ? std::span<const std::byte>{}
                             : shader_library["meta_draw_wireframe_GS_fallbackGS"sv],

           .blend_state = blend_alpha,
           .rasterizer_state = rasterizer_cull_backfacing,
           .depth_stencil_state = depth_stencil_readonly_less_equal,
           .input_layout = meta_draw_primitive_input_layout,

           .render_target_count = 1,
           .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},
           .dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT,

           .debug_name = "meta_draw_triangle_wireframe"sv}),
       device.direct_queue};

   tile_lights_clear = {device.create_compute_pipeline(
                           {.root_signature =
                               root_signature_library.tile_lights_clear.get(),
                            .cs_bytecode = shader_library["tile_lights_clearCS"sv],

                            .debug_name = "tile_lights_clear"sv}),
                        device.direct_queue};

   tile_lights_spheres =
      {device.create_graphics_pipeline(
          {.root_signature = root_signature_library.tile_lights.get(),

           .vs_bytecode = shader_library["tile_lightsVS"sv],
           .ps_bytecode = shader_library["tile_lightsPS"sv],

           .rasterizer_state =
              {
                 .cull_mode = gpu::cull_mode::front,
                 .conservative_raster = device.supports_conservative_rasterization(),
              },
           .depth_stencil_state = depth_stencil_disabled,
           .input_layout = meta_draw_input_layout,

           .debug_name = "tile_lights_spheres"sv}),
       device.direct_queue};

   depth_reduce_minmax = {device.create_compute_pipeline(
                             {.root_signature =
                                 root_signature_library.depth_reduce_minmax.get(),
                              .cs_bytecode = shader_library["depth_reduce_minmaxCS"sv],

                              .debug_name = "depth_reduce_minmax"sv}),
                          device.direct_queue};

   imgui = {device.create_graphics_pipeline(
               {.root_signature = root_signature_library.imgui.get(),

                .vs_bytecode = shader_library["imguiVS"sv],
                .ps_bytecode = shader_library["imguiPS"sv],

                .blend_state = blend_alpha,
                .rasterizer_state = rasterizer_cull_none,
                .depth_stencil_state = depth_stencil_disabled,
                .input_layout = imgui_input_layout,

                .render_target_count = 1,
                .rtv_formats = {DXGI_FORMAT_B8G8R8A8_UNORM_SRGB},

                .debug_name = "imgui"sv}),
            device.direct_queue};
}
}
