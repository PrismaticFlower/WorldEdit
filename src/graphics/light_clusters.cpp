
#include "light_clusters.hpp"
#include "constant_buffers.hpp"
#include "cull_objects.hpp"

#include "math/align.hpp"
#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include "utility/enum_bitflags.hpp"
#include "utility/string_icompare.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <vector>

#include <imgui.h>

namespace we::graphics {

namespace {

struct light_textures {
   explicit light_textures(texture_manager& texture_manager)
      : _texture_manager{texture_manager}
   {
   }

   void trim()
   {
      for (auto it = _textures.begin(); it != _textures.end();) {
         if (not std::exchange(it->used, false)) {
            it = _textures.erase(it);
         }
         else {
            ++it;
         }
      }
   }

   auto try_get(const std::string_view name, const world_texture_dimension dimension)
      -> std::optional<uint32>
   {
      if (name.empty()) return std::nullopt;

      auto it = std::lower_bound(_textures.begin(), _textures.end(), name,
                                 [](const light_texture& texture,
                                    const std::string_view name) {
                                    return string::iless_than(texture.name, name);
                                 });

      if (it != _textures.end() and string::iequals(it->name, name)) {
         it->used = true;

         if (not it->texture) return std::nullopt;
         if (it->texture->dimension != dimension) return std::nullopt;

         return it->texture->srv.index;
      }
      else {
         light_texture texture{.name = lowercase_string{name}};

         texture.texture = _texture_manager.at_if(texture.name);

         if (not texture.texture) {
            texture.load_token = _texture_manager.acquire_load_token(texture.name);
         }

         _textures.insert(it, std::move(texture));
      }

      return std::nullopt;
   }

   void process_updated(const updated_textures& updated) noexcept
   {
      for (light_texture& texture : _textures) {
         if (auto new_texture = updated.check(texture.name); new_texture) {
            texture.load_token = nullptr;
            texture.texture = std::move(new_texture);
         }
      }
   }

private:
   struct light_texture {
      lowercase_string name;

      bool used = true;

      std::shared_ptr<const world_texture> texture;
      std::shared_ptr<const world_texture_load_token> load_token;
   };

   std::vector<light_texture> _textures;
   texture_manager& _texture_manager;
};

constexpr uint32 shadow_res = 2048;
constexpr uint32 cascade_count = 4;
constexpr uint32 light_tile_size = 8;
constexpr uint32 sun_cascade_count = 4;
constexpr uint32 max_onscreen_lights = 256;
constexpr uint32 tile_light_word_bits = 32;
constexpr uint32 tile_light_words = max_onscreen_lights / tile_light_word_bits;

enum class light_type : uint32 {
   directional_box,
   directional_sphere,
   directional_cylinder,
   point,
   spot
};

enum light_flags : uint32 {
   none = 0b0,
   is_dynamic = 0b1,
   has_texture = 0b10,
   texture_clamp = 0b100,
};

constexpr bool marked_as_enum_bitflag(light_flags)
{
   return true;
}

struct global_light {
   float3 directionWS;
   light_flags flags;
   float3 color;
   uint32 has_shadows;
   float4x4 texture_from_world;
   uint32 texture_index;
   uint32 texture_clamp;
   std::array<uint32, 2> pad;
};

static_assert(sizeof(global_light) == 112);

struct light_description {
   light_type type = light_type::point;

   light_flags flags;

   struct point_desc {
      float3 positionWS;
      float range;
      float3 color;
   };

   struct spot_desc {
      float3 positionWS;
      float range;
      float3 color;
      float3 directionWS;
      float spot_outer_param;
      float spot_inner_param;
   };

   struct directional_box_desc {
      float3 color;
      float3 directionWS;
      float4x4 world_from_region;
      float4x4 region_from_world;
      float3 size;
   };

   struct directional_sphere_desc {
      float3 color;
      float3 directionWS;
      float4x4 world_from_region;
      float4x4 region_from_world;
      float radius;
   };

   struct directional_cylinder_desc {
      float3 color;
      float3 directionWS;
      float4x4 world_from_region;
      float4x4 region_from_world;
      float radius;
      float height;
   };

   union {
      point_desc point = {};
      spot_desc spot;
      directional_box_desc directional_box;
      directional_sphere_desc directional_sphere;
      directional_cylinder_desc directional_cylinder;
   };

   uint32 texture_index;
   uint32 texture_clamp;
   float4x4 texture_from_world;
};

struct alignas(16) tiling_inputs {
   std::array<uint32, 2> tile_counts;
   std::array<uint32, 2> padding0{};

   float4x4 projection_from_world;
};

static_assert(sizeof(tiling_inputs) == 80);

struct alignas(16) gpu_light_description {
   float3 direction;
   light_type type;
   float3 position;
   float range;
   float3 color;
   float spot_outer_param;
   float spot_inner_param;
   light_flags flags;
   uint32 pad0;
};

static_assert(sizeof(gpu_light_description) == 64);

struct light_region_description {
   float4x4 region_from_world;
   float3 size;
   uint32 pad;
};

static_assert(sizeof(light_region_description) == 80);

struct light_texture_description {
   float4x4 texture_from_world;
   uint32 texture_index;
   uint32 texture_clamp;
   std::array<uint32, 2> pad;
};

static_assert(sizeof(light_texture_description) == 80);

struct alignas(16) light_constants {
   uint32 light_tiles_width;
   gpu::resource_view light_tiles_index;
   gpu::resource_view shadow_map_index;
   uint32 padding0;

   float3 sky_ambient_color;
   uint32 padding1;

   float3 ground_ambient_color;
   uint32 padding2;

   std::array<global_light, 2> global_lights;

   std::array<float4x4, 4> shadow_transforms;

   float2 shadow_resolution;
   float2 inv_shadow_resolution;

   std::array<gpu_light_description, max_onscreen_lights> lights;
   std::array<light_region_description, max_onscreen_lights> light_regions;
   std::array<light_texture_description, max_onscreen_lights> light_textures;
};

static_assert(sizeof(light_constants) == 57888);

struct alignas(16) light_proxy_instance {
   std::array<float4, 3> transform;
   uint32 light_index;
   std::array<uint32, 3> padding;
};

static_assert(sizeof(light_proxy_instance) == 64);

auto make_sphere_light_proxy_transform(const float3& position, const float radius)
   -> std::array<float4, 3>
{
   float4x4 light_transform = {{radius, 0.0f, 0.0f, 0.0f}, //
                               {0.0f, radius, 0.0f, 0.0f}, //
                               {0.0f, 0.0f, radius, 0.0f}, //
                               {0.0f, 0.0f, 0.0f, 1.0f}};
   light_transform[3] = {position, 1.0f};
   light_transform = transpose(
      light_transform); // transpose so we have good alignment for a float3x4 in HLSL

   return {light_transform[0], light_transform[1], light_transform[2]};
}

constexpr std::array<float3, 26> sphere_proxy_vertices = {
   {{1.160595f, 0.000000f, 0.000000f},    {0.670070f, 0.670070f, 0.670070f},
    {0.820664f, 0.000000f, 0.820664f},    {-0.000000f, 0.000000f, 1.160595f},
    {-0.670070f, 0.670070f, 0.670070f},   {-0.820664f, 0.000000f, 0.820664f},
    {-1.160595f, 0.000000f, 0.000000f},   {-0.670070f, 0.670070f, -0.670070f},
    {-0.820664f, 0.000000f, -0.820664f},  {-0.000000f, 0.000000f, -1.160595f},
    {0.670070f, 0.670070f, -0.670070f},   {0.820664f, 0.000000f, -0.820664f},
    {-0.000000f, -1.160595f, 0.000000f},  {-0.670070f, -0.670070f, -0.670070f},
    {-0.000000f, -0.820664f, -0.820664f}, {-0.000000f, 1.160595f, 0.000000f},
    {-0.000000f, 0.820664f, -0.820664f},  {-0.820664f, 0.820664f, 0.000000f},
    {-0.000000f, 0.820664f, 0.820664f},   {0.670070f, -0.670070f, -0.670070f},
    {0.820664f, -0.820664f, 0.000000f},   {0.670070f, -0.670070f, 0.670070f},
    {-0.670070f, -0.670070f, 0.670070f},  {-0.000000f, -0.820664f, 0.820664f},
    {-0.820664f, -0.820664f, 0.000000f},  {0.820664f, 0.820664f, 0.000000f}}};

constexpr std::array<uint16, 144> sphere_proxy_indices{
   0,  1,  2,  3,  4,  5,  6, 7,  8,  9,  10, 11, 12, 13, 14, 15, 10, 16,
   15, 7,  17, 4,  15, 17, 1, 15, 18, 12, 19, 20, 21, 12, 20, 22, 12, 23,
   9,  19, 14, 13, 9,  14, 7, 9,  8,  6,  13, 24, 22, 6,  24, 4,  6,  5,
   3,  22, 23, 21, 3,  23, 1, 3,  2,  0,  21, 20, 19, 0,  20, 10, 0,  11,
   0,  25, 1,  3,  18, 4,  6, 17, 7,  9,  16, 10, 12, 24, 13, 15, 25, 10,
   15, 16, 7,  4,  18, 15, 1, 25, 15, 12, 14, 19, 21, 23, 12, 22, 24, 12,
   9,  11, 19, 13, 8,  9,  7, 16, 9,  6,  8,  13, 22, 5,  6,  4,  17, 6,
   3,  5,  22, 21, 2,  3,  1, 18, 3,  0,  2,  21, 19, 11, 0,  10, 25, 0};

auto make_shadow_cascade_splits(const float near_clip, const float far_clip)
   -> std::array<float, cascade_count + 1>
{
   const float clip_ratio = far_clip / near_clip;
   const float clip_range = far_clip - near_clip;

   std::array<float, 5> cascade_splits{};

   for (int i = 0; i < cascade_splits.size(); ++i) {
      const float split = (near_clip * std::pow(clip_ratio, i / 4.0f));
      const float split_normalized = (split - near_clip) / clip_range;

      cascade_splits[i] = split_normalized;
   }

   return cascade_splits;
}

auto make_cascade_shadow_camera(const float3 light_direction,
                                const float near_split, const float far_split,
                                const frustum& view_frustum) -> shadow_ortho_camera
{
   auto view_frustum_corners = view_frustum.corners;

   for (int i = 0; i < 4; ++i) {
      float3 corner_ray = view_frustum_corners[i + 4] - view_frustum_corners[i];

      view_frustum_corners[i + 4] = view_frustum_corners[i] + (corner_ray * far_split);
      view_frustum_corners[i] += (corner_ray * near_split);
   }

   float3 view_frustum_center{0.0f, 0.0f, 0.0f};

   for (const auto& corner : view_frustum_corners) {
      view_frustum_center += corner;
   }

   view_frustum_center /= 8.0f;

   float radius = std::numeric_limits<float>::lowest();

   for (const auto& corner : view_frustum_corners) {
      radius = std::max(distance(corner, view_frustum_center), radius);
   }

   float3 bounds_max{radius, radius, radius};
   float3 bounds_min{-radius, -radius, -radius};
   float3 casecase_extents{bounds_max - bounds_min};

   float3 shadow_camera_position =
      view_frustum_center + light_direction * -bounds_min.z;

   shadow_ortho_camera shadow_camera;
   shadow_camera.set_projection(bounds_min.x, bounds_min.y, bounds_max.x,
                                bounds_max.y, 0.0f, casecase_extents.z);
   shadow_camera.look_at(shadow_camera_position, view_frustum_center,
                         float3{0.0f, 1.0f, 0.0f});

   auto shadow_view_projection = shadow_camera.projection_from_world();

   float4 shadow_origin = shadow_view_projection * float4{0.0f, 0.0f, 0.0f, 1.0f};
   shadow_origin /= shadow_origin.w;
   shadow_origin *= float{shadow_res} / 2.0f;

   float2 shadow_origin_xy = {shadow_origin.x, shadow_origin.y};

   float2 rounded_origin = round(shadow_origin_xy);
   float2 rounded_offset = rounded_origin - shadow_origin_xy;
   rounded_offset *= 2.0f / float{shadow_res};

   shadow_camera.set_stabilization(rounded_offset);

   return shadow_camera;
}

auto make_shadow_cascades(const quaternion light_rotation, const camera& camera,
                          const std::array<float, 2> scene_depth_min_max)
   -> std::array<shadow_ortho_camera, cascade_count>
{
   const frustum view_frustum{camera.world_from_projection(),
                              scene_depth_min_max[0], scene_depth_min_max[1]};

   const float3 light_direction =
      normalize(light_rotation * float3{0.0f, 0.0f, -1.0f});

   const std::array cascade_splits =
      make_shadow_cascade_splits(-unproject_depth_value(camera, scene_depth_min_max[1]),
                                 -unproject_depth_value(camera,
                                                        scene_depth_min_max[0]));

   std::array<shadow_ortho_camera, cascade_count> cameras;

   for (int i = 0; i < cascade_count; ++i) {
      cameras[i] = make_cascade_shadow_camera(light_direction, cascade_splits[i],
                                              cascade_splits[i + 1], view_frustum);
   }

   return cameras;
}

}

struct light_clusters::impl {
   impl(gpu::device& device, texture_manager& texture_manager,
        copy_command_list_pool& copy_command_list_pool, uint32 render_width,
        uint32 render_height)
      : _device{device}, _textures{texture_manager}
   {
      _tiling_inputs = {device.create_buffer({.size = sizeof(tiling_inputs),
                                              .debug_name =
                                                 "Lights Tiling Inputs"},
                                             gpu::heap_type::default_),
                        device};

      _lights_constants = {device.create_buffer({.size = sizeof(light_constants),
                                                 .debug_name =
                                                    "Lights Constant Buffer"},
                                                gpu::heap_type::default_),
                           device};

      _shadow_map = {device.create_texture(
                        {.dimension = gpu::texture_dimension::t_2d,
                         .flags = {.allow_depth_stencil = true},
                         .format = DXGI_FORMAT_D32_FLOAT,
                         .width = shadow_res,
                         .height = shadow_res,
                         .array_size = cascade_count,
                         .optimized_clear_value =
                            gpu::texture_clear_value{.format = DXGI_FORMAT_D32_FLOAT,
                                                     .depth_stencil = {.depth = 1.0f}}},
                        gpu::barrier_layout::direct_queue_shader_resource,
                        gpu::legacy_resource_state::pixel_shader_resource),
                     device};

      for (uint32 i = 0; i < cascade_count; ++i) {
         _shadow_map_dsv[i] = {device.create_depth_stencil_view(
                                  _shadow_map.get(),
                                  {.format = DXGI_FORMAT_D32_FLOAT,
                                   .dimension = gpu::dsv_dimension::texture2d_array,

                                   .texture2d_array = {.first_array_slice = i,
                                                       .array_size = 1}}),
                               device};
      }

      update_render_resolution(render_width, render_height, false);
      update_descriptors();

      init_proxy_geometry(device, copy_command_list_pool);
   }

   void update_render_resolution(uint32 width, uint32 height)
   {
      update_render_resolution(width, height, true);
   }

   void prepare_lights(const camera& view_camera, const frustum& view_frustum,
                       const world::world& world,
                       const world::light* optional_placement_light,
                       const world::entity_group* optional_entity_group,
                       const std::array<float, 2> scene_depth_min_max,
                       blocks& blocks, billboard_patches& billboard_patches,
                       const world::active_layers active_layers,
                       const world::active_entity_types active_entity_types,
                       gpu::copy_command_list& command_list,
                       dynamic_buffer_allocator& dynamic_buffer_allocator)
   {
      _scene_depth_min_max = scene_depth_min_max;
      _lights_allocated = 0;
      _light_proxy_count = 0;
      _has_sun_shadows = false;

      _textures.trim();

      add_world_lights(view_camera, view_frustum, world,
                       optional_placement_light, optional_entity_group);

      light_constants light_constants{.light_tiles_width = _tiles_width,
                                      .light_tiles_index = _lights_tiles_srv.get(),
                                      .shadow_map_index = _shadow_map_srv.get(),
                                      .sky_ambient_color = _ambient_sky_color,
                                      .ground_ambient_color = _ambient_ground_color,
                                      .global_lights = _global_lights,
                                      .shadow_resolution = {shadow_res, shadow_res},
                                      .inv_shadow_resolution = {1.0f / shadow_res,
                                                                1.0f / shadow_res}};

      std::array<light_proxy_instance, max_onscreen_lights> sphere_light_proxies{};

      std::array<gpu_light_description, max_onscreen_lights>& lights =
         light_constants.lights;

      // frustum cull lights
      for (std::size_t i = 0; i < _lights_allocated; ++i) {
         const uint32 light_index = _lights_order[i].light_index;
         const light_description& light = _lights[light_index];

         switch (light.type) {
         case light_type::point: {
            lights[light_index] = {.type = light_type::point,
                                   .position = light.point.positionWS,
                                   .range = light.point.range,
                                   .color = light.point.color,
                                   .flags = light.flags};

            sphere_light_proxies[_light_proxy_count++] =
               {.transform = make_sphere_light_proxy_transform(light.point.positionWS,
                                                               light.point.range),

                .light_index = light_index};
         } break;
         case light_type::spot: {
            lights[light_index] = {.direction = light.spot.directionWS,
                                   .type = light_type::spot,
                                   .position = light.spot.positionWS,
                                   .range = light.spot.range,
                                   .color = light.spot.color,
                                   .spot_outer_param = light.spot.spot_outer_param,
                                   .spot_inner_param = light.spot.spot_inner_param,
                                   .flags = light.flags};

            sphere_light_proxies[_light_proxy_count++] =
               {.transform = make_sphere_light_proxy_transform(light.spot.positionWS,
                                                               light.spot.range), // TODO: Cone light proxies.
                .light_index = light_index};
         } break;
         case light_type::directional_box: {
            lights[light_index] = {.direction = light.directional_box.directionWS,
                                   .type = light_type::directional_box,
                                   .color = light.directional_box.color,
                                   .flags = light.flags};
            light_constants.light_regions[light_index] = {
               .region_from_world = light.directional_box.region_from_world,
               .size = light.directional_box.size,
            };

            sphere_light_proxies[_light_proxy_count++] =
               {.transform = make_sphere_light_proxy_transform(
                   {light.directional_box.world_from_region[3].x,
                    light.directional_box.world_from_region[3].y,
                    light.directional_box.world_from_region[3].z},
                   length(light.directional_box.size)),

                .light_index = light_index};
         } break;
         case light_type::directional_sphere: {
            lights[light_index] = {.direction = light.directional_sphere.directionWS,
                                   .type = light_type::directional_sphere,
                                   .color = light.directional_sphere.color,
                                   .flags = light.flags};
            light_constants.light_regions[light_index] = {
               .region_from_world = light.directional_sphere.region_from_world,
               .size = {light.directional_sphere.radius, 0.0f, 0.0f},
            };

            sphere_light_proxies[_light_proxy_count++] =
               {.transform = make_sphere_light_proxy_transform(
                   {light.directional_sphere.world_from_region[3].x,
                    light.directional_sphere.world_from_region[3].y,
                    light.directional_sphere.world_from_region[3].z},
                   light.directional_sphere.radius),

                .light_index = light_index};
         } break;
         case light_type::directional_cylinder: {
            light_constants.light_regions[light_index] = {
               .region_from_world = light.directional_cylinder.region_from_world,
               .size = {light.directional_cylinder.radius,
                        light.directional_cylinder.height, 0.0f},
            };

            lights[light_index] = {.direction = light.directional_cylinder.directionWS,
                                   .type = light_type::directional_cylinder,
                                   .color = light.directional_cylinder.color,
                                   .flags = light.flags};

            sphere_light_proxies[_light_proxy_count++] =
               {.transform = make_sphere_light_proxy_transform(
                   {light.directional_sphere.world_from_region[3].x,
                    light.directional_sphere.world_from_region[3].y,
                    light.directional_sphere.world_from_region[3].z},
                   std::max(light.directional_cylinder.radius,
                            light.directional_cylinder.height)),

                .light_index = light_index};
         } break;
         }

         if (are_flags_set(light.flags, light_flags::has_texture)) {
            light_constants.light_textures[light_index] = {
               .texture_from_world = light.texture_from_world,
               .texture_index = light.texture_index,
               .texture_clamp = light.texture_clamp,
            };
         }
      }

      light_constants
         .shadow_transforms = {_sun_shadow_cascades[0].texture_from_world(),
                               _sun_shadow_cascades[1].texture_from_world(),
                               _sun_shadow_cascades[2].texture_from_world(),
                               _sun_shadow_cascades[3].texture_from_world()};

      // copy tile culling inputs
      {
         tiling_inputs tiling_inputs{.tile_counts = {_tiles_width, _tiles_height},
                                     .projection_from_world =
                                        view_camera.projection_from_world()};

         auto upload_buffer =
            dynamic_buffer_allocator.allocate(sizeof(tiling_inputs));

         std::memcpy(upload_buffer.cpu_address, &tiling_inputs, sizeof(tiling_inputs));

         command_list.copy_buffer_region(_tiling_inputs.get(), 0,
                                         upload_buffer.resource, upload_buffer.offset,
                                         sizeof(tiling_inputs));
      }

      {
         auto upload_buffer =
            dynamic_buffer_allocator.allocate(sizeof(light_constants));

         std::memcpy(upload_buffer.cpu_address, &light_constants,
                     sizeof(light_constants));

         command_list.copy_buffer_region(_lights_constants.get(), 0,
                                         upload_buffer.resource, upload_buffer.offset,
                                         sizeof(light_constants));
      }

      {
         auto upload_buffer =
            dynamic_buffer_allocator.allocate(sizeof(sphere_light_proxies));

         std::memcpy(upload_buffer.cpu_address, &sphere_light_proxies,
                     sizeof(sphere_light_proxies));

         _sphere_light_proxies_srv = upload_buffer.gpu_address;
      }

      if (_has_sun_shadows) {
         if (active_entity_types.objects) {
            for (int cascade_index = 0; cascade_index < cascade_count; ++cascade_index) {
               _sun_shadow_billboard_patches_view[cascade_index] =
                  billboard_patches.prepare_view(billboard_patches_prepare::shadow,
                                                 frustum{_sun_shadow_cascades[cascade_index]
                                                            .world_from_projection(),
                                                         1.0f, 0.0f},
                                                 dynamic_buffer_allocator);
            }
         }
         else {
            _sun_shadow_billboard_patches_view = {};
         }

         if (active_entity_types.blocks) {
            for (int cascade_index = 0; cascade_index < cascade_count; ++cascade_index) {
               _sun_shadow_blocks_view[cascade_index] = blocks.prepare_view(
                  blocks_draw::shadow, world.blocks, optional_entity_group,
                  frustum{_sun_shadow_cascades[cascade_index].world_from_projection(),
                          1.0f, 0.0f},
                  active_layers, dynamic_buffer_allocator);
            }
         }
         else {
            _sun_shadow_blocks_view = {};
         }
      }
      else {
         _sun_shadow_billboard_patches_view = {};
         _sun_shadow_blocks_view = {};
      }
   }

   void tile_lights(root_signature_library& root_signatures, pipeline_library& pipelines,
                    gpu::graphics_command_list& command_list, profiler& profiler)
   {
      profile_section profile{"Lights - Tile Lights", command_list, profiler,
                              profiler_queue::direct};

      // clear light tiles
      {
         const std::array<uint32, 2> tile_counts = {_tiles_width, _tiles_height};

         command_list.set_compute_root_signature(
            root_signatures.tile_lights_clear.get());
         command_list.set_compute_32bit_constants(rs::tile_lights_clear::tile_counts,
                                                  std::as_bytes(std::span{tile_counts}),
                                                  0);
         command_list.set_compute_uav(rs::tile_lights_clear::light_tiles_uav,
                                      _device.get_gpu_virtual_address(
                                         _lights_tiles.get()));

         command_list.set_pipeline_state(pipelines.tile_lights_clear.get());

         command_list.dispatch(math::align_up(_tiles_width, 32) / 32,
                               math::align_up(_tiles_height, 32) / 32, 1);
      }

      [[likely]] if (_device.supports_enhanced_barriers()) {
         command_list.deferred_barrier(
            gpu::buffer_barrier{.sync_before = gpu::barrier_sync::compute_shading,
                                .sync_after = gpu::barrier_sync::pixel_shading,
                                .access_before = gpu::barrier_access::unordered_access,
                                .access_after = gpu::barrier_access::unordered_access,
                                .resource = _lights_tiles.get()});
      }
      else {
         command_list.deferred_barrier(
            gpu::legacy_resource_uav_barrier{.resource = _lights_tiles.get()});
      }
      command_list.flush_barriers();

      command_list.set_graphics_root_signature(root_signatures.tile_lights.get());
      command_list.set_graphics_uav(rs::tile_lights::light_tiles_uav,
                                    _device.get_gpu_virtual_address(
                                       _lights_tiles.get()));
      command_list.set_graphics_cbv(rs::tile_lights::cbv, _tiling_inputs_cbv);

      command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

      command_list.rs_set_scissor_rects({0, 0, _tiles_width, _tiles_height});
      command_list.rs_set_viewports({0.0f, 0.0f, static_cast<float>(_tiles_width),
                                     static_cast<float>(_tiles_height)});

      command_list.om_set_render_targets(std::span<const gpu::rtv_handle>{});

      // draw sphere lights
      if (_light_proxy_count > 0) {
         command_list.set_graphics_srv(rs::tile_lights::instance_data_srv,
                                       _sphere_light_proxies_srv);

         command_list.set_pipeline_state(pipelines.tile_lights_spheres.get());

         command_list.ia_set_index_buffer(
            {.buffer_location =
                _device.get_gpu_virtual_address(_sphere_proxy_indices.get()),
             .size_in_bytes = sizeof(sphere_proxy_indices)});
         command_list.ia_set_vertex_buffers(
            0, gpu::vertex_buffer_view{.buffer_location = _device.get_gpu_virtual_address(
                                          _sphere_proxy_vertices.get()),
                                       .size_in_bytes = sizeof(sphere_proxy_vertices),
                                       .stride_in_bytes = sizeof(float3)});

         command_list.draw_indexed_instanced(static_cast<uint32>(
                                                sphere_proxy_indices.size()),
                                             _light_proxy_count, 0, 0, 0);
      }

      [[likely]] if (_device.supports_enhanced_barriers()) {
         command_list.deferred_barrier(
            gpu::buffer_barrier{.sync_before = gpu::barrier_sync::pixel_shading,
                                .sync_after = gpu::barrier_sync::pixel_shading,
                                .access_before = gpu::barrier_access::unordered_access,
                                .access_after = gpu::barrier_access::shader_resource,
                                .resource = _lights_tiles.get()});
      }
      else {
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _lights_tiles.get(),
            .state_before = gpu::legacy_resource_state::unordered_access,
            .state_after = gpu::legacy_resource_state::pixel_shader_resource});
      }
   }

   void draw_shadow_maps(const world_mesh_list& meshes, const blocks& blocks,
                         const billboard_patches& billboard_patches,
                         root_signature_library& root_signatures,
                         pipeline_library& pipelines,
                         gpu::graphics_command_list& command_list,
                         dynamic_buffer_allocator& dynamic_buffer_allocator,
                         profiler& profiler)
   {
      if (not _has_sun_shadows) return;

      profile_section profile{"Lights - Draw Shadow Maps", command_list,
                              profiler, profiler_queue::direct};

      [[likely]] if (_device.supports_enhanced_barriers()) {
         command_list.deferred_barrier(
            gpu::texture_barrier{.sync_before = gpu::barrier_sync::none,
                                 .sync_after = gpu::barrier_sync::depth_stencil,
                                 .access_before = gpu::barrier_access::no_access,
                                 .access_after = gpu::barrier_access::depth_stencil_write,
                                 .layout_before = gpu::barrier_layout::direct_queue_shader_resource,
                                 .layout_after = gpu::barrier_layout::depth_stencil_write,
                                 .resource = _shadow_map.get()});
      }
      else {
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _shadow_map.get(),
            .state_before = gpu::legacy_resource_state::pixel_shader_resource,
            .state_after = gpu::legacy_resource_state::depth_write});
      }
      command_list.flush_barriers();

      for (int cascade_index = 0; cascade_index < cascade_count; ++cascade_index) {
         auto& shadow_camera = _sun_shadow_cascades[cascade_index];

         gpu::dsv_handle depth_stencil_view = _shadow_map_dsv[cascade_index].get();
         gpu_virtual_address frame_cbv =
            dynamic_buffer_allocator
               .allocate_and_copy(
                  frame_constant_buffer{.projection_from_world =
                                           shadow_camera.projection_from_world(),
                                        .projection_from_view =
                                           shadow_camera.projection_from_view()})
               .gpu_address;

         command_list.clear_depth_stencil_view(depth_stencil_view, {}, 1.0f, 0x0);

         command_list.set_graphics_root_signature(root_signatures.mesh_shadow.get());
         command_list.set_graphics_cbv(rs::mesh_shadow::camera_cbv, frame_cbv);

         command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

         command_list.rs_set_scissor_rects({0, 0, shadow_res, shadow_res});
         command_list.rs_set_viewports({0, 0, shadow_res, shadow_res});

         command_list.om_set_render_targets(depth_stencil_view);

         frustum shadow_frustum{shadow_camera.world_from_projection(), 1.0f, 0.0f};

         const std::size_t max_visible_objects = std::max({
            meshes.opaque[mesh_opaque_flags::none].size(),
            meshes.opaque[mesh_opaque_flags::doublesided].size(),
            meshes.opaque[mesh_opaque_flags::alpha_cutout].size(),
            meshes
               .opaque[mesh_opaque_flags::alpha_cutout | mesh_opaque_flags::doublesided]
               .size(),
         });

         if (_shadow_render_list.size() < max_visible_objects) {
            _shadow_render_list.resize(max_visible_objects);
         }

         struct shadow_batch_flags {
            mesh_opaque_flags mesh;
            depth_prepass_pipeline_flags pipeline;
         };

         for (const shadow_batch_flags batch_flags : {
                 shadow_batch_flags{mesh_opaque_flags::none,
                                    depth_prepass_pipeline_flags::none},
                 shadow_batch_flags{mesh_opaque_flags::doublesided,
                                    depth_prepass_pipeline_flags::doublesided},
                 shadow_batch_flags{mesh_opaque_flags::alpha_cutout,
                                    depth_prepass_pipeline_flags::alpha_cutout},
                 shadow_batch_flags{mesh_opaque_flags::alpha_cutout |
                                       mesh_opaque_flags::doublesided,
                                    depth_prepass_pipeline_flags::alpha_cutout |
                                       depth_prepass_pipeline_flags::doublesided},
              }) {
            const std::span<uint16> visible_objects = cull_objects_shadow_cascade(
               shadow_frustum, meshes.opaque[batch_flags.mesh].bbox.min.x,
               meshes.opaque[batch_flags.mesh].bbox.min.y,
               meshes.opaque[batch_flags.mesh].bbox.min.z,
               meshes.opaque[batch_flags.mesh].bbox.max.x,
               meshes.opaque[batch_flags.mesh].bbox.max.y,
               meshes.opaque[batch_flags.mesh].bbox.max.z, _shadow_render_list);

            if (are_flags_set(batch_flags.mesh, mesh_opaque_flags::alpha_cutout)) {
               draw_meshes_alpha_cutout_shadow_map(
                  meshes.opaque[batch_flags.mesh], visible_objects,
                  pipelines.mesh_shadow[batch_flags.pipeline].get(), command_list);
            }
            else {
               draw_meshes_shadow_map(
                  meshes.opaque[batch_flags.mesh], visible_objects,
                  pipelines.mesh_shadow[batch_flags.pipeline].get(), command_list);
            }
         }

         blocks.draw(blocks_draw::shadow, _sun_shadow_blocks_view[cascade_index],
                     frame_cbv, _lights_constant_buffer_view, command_list,
                     root_signatures, pipelines);

         billboard_patches.draw(billboard_patches_draw::shadow,
                                _sun_shadow_billboard_patches_view[cascade_index],
                                frame_cbv, _lights_constant_buffer_view,
                                command_list, root_signatures, pipelines);
      }

      [[likely]] if (_device.supports_enhanced_barriers()) {
         command_list.deferred_barrier(
            gpu::texture_barrier{.sync_before = gpu::barrier_sync::depth_stencil,
                                 .sync_after = gpu::barrier_sync::pixel_shading,
                                 .access_before = gpu::barrier_access::depth_stencil_write,
                                 .access_after = gpu::barrier_access::shader_resource,
                                 .layout_before = gpu::barrier_layout::depth_stencil_write,
                                 .layout_after = gpu::barrier_layout::direct_queue_shader_resource,
                                 .resource = _shadow_map.get()});
      }
      else {
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _shadow_map.get(),
            .state_before = gpu::legacy_resource_state::depth_write,
            .state_after = gpu::legacy_resource_state::pixel_shader_resource});
      }
   }

   auto lights_constant_buffer_view() const noexcept -> gpu_virtual_address
   {
      return _lights_constant_buffer_view;
   }

   void process_updated_textures(const updated_textures& updated) noexcept
   {
      _textures.process_updated(updated);
   }

private:
   void update_render_resolution(uint32 width, uint32 height, bool recreate_descriptors)
   {
      const uint32 aligned_width = math::align_up(width, light_tile_size);
      const uint32 aligned_height = math::align_up(height, light_tile_size);
      const uint32 tiles_width = aligned_width / light_tile_size;
      const uint32 tiles_height = aligned_height / light_tile_size;
      const uint32 tiles_count = tiles_width * tiles_height;

      _lights_tiles = {_device.create_buffer({.size = sizeof(uint32) * 8 * tiles_count,
                                              .flags = {.allow_unordered_access = true},
                                              .debug_name = "Lights Tiles"},
                                             gpu::heap_type::default_),
                       _device};

      _tiles_count = tiles_count;
      _tiles_width = tiles_width;
      _tiles_height = tiles_height;
      _render_width = static_cast<float>(width);
      _render_height = static_cast<float>(height);

      if (recreate_descriptors) update_descriptors();
   }

   void update_descriptors()
   {
      _lights_constant_buffer_view =
         _device.get_gpu_virtual_address(_lights_constants.get());

      _lights_tiles_srv = {_device.create_shader_resource_view(
                              _lights_tiles.get(),
                              {.buffer = {.first_element = 0,
                                          .number_elements = _tiles_count,
                                          .structure_byte_stride = sizeof(uint32) * 8}}),
                           _device};

      _shadow_map_srv = {_device.create_shader_resource_view(_shadow_map.get(),
                                                             {.format = DXGI_FORMAT_R32_FLOAT}),
                         _device};

      _tiling_inputs_cbv = _device.get_gpu_virtual_address(_tiling_inputs.get());
   }

   void init_proxy_geometry(gpu::device& device,
                            copy_command_list_pool& copy_command_list_pool)
   {
      pooled_copy_command_list command_list =
         copy_command_list_pool.aquire_and_reset();

      _sphere_proxy_indices = {device.create_buffer({.size = sizeof(sphere_proxy_indices),
                                                     .debug_name =
                                                        "Light Proxy Indices"},
                                                    gpu::heap_type::default_),
                               device};
      _sphere_proxy_vertices =
         {device.create_buffer({.size = sizeof(sphere_proxy_vertices),
                                .debug_name = "Light Proxy Vertices"},
                               gpu::heap_type::default_),
          device};

      const auto upload_buffer_size =
         sizeof(sphere_proxy_indices) + sizeof(sphere_proxy_vertices);

      gpu::unique_resource_handle upload_buffer =
         {device.create_buffer({.size = upload_buffer_size,
                                .debug_name = "Light Proxy Upload Buffer"},
                               gpu::heap_type::upload),
          device};

      std::byte* const upload_buffer_ptr =
         static_cast<std::byte*>(device.map(upload_buffer.get(), 0, {}));

      const auto sphere_proxy_indices_offset = 0;
      const auto sphere_proxy_vertices_offset = sizeof(sphere_proxy_indices);

      std::memcpy(upload_buffer_ptr + sphere_proxy_indices_offset,
                  &sphere_proxy_indices, sizeof(sphere_proxy_indices));

      std::memcpy(upload_buffer_ptr + sphere_proxy_vertices_offset,
                  &sphere_proxy_vertices, sizeof(sphere_proxy_vertices));

      device.unmap(upload_buffer.get(), 0, {0, upload_buffer_size});

      command_list->copy_buffer_region(_sphere_proxy_indices.get(), 0,
                                       upload_buffer.get(), sphere_proxy_indices_offset,
                                       sizeof(sphere_proxy_indices));

      command_list->copy_buffer_region(_sphere_proxy_vertices.get(), 0,
                                       upload_buffer.get(), sphere_proxy_vertices_offset,
                                       sizeof(sphere_proxy_vertices));
      command_list->close();

      device.background_copy_queue.execute_command_lists(command_list.get());
   }

   void draw_meshes_shadow_map(const world_opaque_mesh_list& meshes,
                               const std::span<const uint16> render_list,
                               gpu::pipeline_handle pipeline,
                               gpu::graphics_command_list& command_list) const
   {
      command_list.set_pipeline_state(pipeline);

      for (const uint16 i : render_list) {
         command_list.set_graphics_cbv(rs::mesh_shadow::object_cbv,
                                       meshes.gpu_constants[i]);

         command_list.ia_set_index_buffer(meshes.mesh[i].index_buffer_view);
         command_list.ia_set_vertex_buffers(0, meshes.mesh[i].vertex_buffer_views);

         command_list.draw_indexed_instanced(meshes.mesh[i].index_count, 1,
                                             meshes.mesh[i].start_index,
                                             meshes.mesh[i].start_vertex, 0);
      }
   }

   void draw_meshes_alpha_cutout_shadow_map(const world_opaque_mesh_list& meshes,
                                            const std::span<const uint16> render_list,
                                            gpu::pipeline_handle pipeline,
                                            gpu::graphics_command_list& command_list) const
   {
      command_list.set_pipeline_state(pipeline);

      for (const uint16 i : render_list) {
         command_list.set_graphics_cbv(rs::mesh_shadow::object_cbv,
                                       meshes.gpu_constants[i]);
         command_list.set_graphics_cbv(rs::mesh_shadow::material_cbv,
                                       meshes.material_constant_buffer[i]);

         command_list.ia_set_index_buffer(meshes.mesh[i].index_buffer_view);
         command_list.ia_set_vertex_buffers(0, meshes.mesh[i].vertex_buffer_views);

         command_list.draw_indexed_instanced(meshes.mesh[i].index_count, 1,
                                             meshes.mesh[i].start_index,
                                             meshes.mesh[i].start_vertex, 0);
      }
   }

   void add_world_lights(const camera& view_camera, const frustum& view_frustum,
                         const world::world& world,
                         const world::light* optional_placement_light,
                         const world::entity_group* optional_entity_group)
   {
      _ambient_sky_color = world.global_lights.ambient_sky_color;
      _ambient_ground_color = world.global_lights.ambient_ground_color;

      if (world.global_lights.global_light_1.has_index()) {
         const world::light& light =
            world.lights[world.global_lights.global_light_1.index()];

         _global_lights[0] = {
            .directionWS = normalize(light.rotation * float3{0.0f, 0.0f, -1.0f}),
            .flags = not light.static_ ? light_flags::is_dynamic : light_flags::none,
            .color = light.color,
            .has_shadows = light.shadow_caster,
         };

         _has_sun_shadows = light.shadow_caster;

         if (_has_sun_shadows) {
            _sun_shadow_cascades = make_shadow_cascades(light.rotation, view_camera,
                                                        _scene_depth_min_max);
         }

         if (not light.texture.empty()) {
            std::optional<uint32> texture =
               _textures.try_get(light.texture, world_texture_dimension::_2d);

            if (texture) {
               _global_lights[0].flags |= light_flags::has_texture;
               _global_lights[0].texture_index = *texture;
               _global_lights[0].texture_clamp =
                  light.texture_addressing == world::texture_addressing::clamp;

               const float2 inv_texture_tiling = 1.0f / light.directional_texture_tiling;
               const float2 texture_offset =
                  light.directional_texture_offset * inv_texture_tiling;
               const float3& directionWS = _global_lights[0].directionWS;

               const float3 column0 =
                  (float3{1.0f, 0.0f, 0.0f} - directionWS * directionWS.x) *
                  inv_texture_tiling.x;
               const float3 column1 =
                  (float3{0.0f, 0.0f, 1.0f} - directionWS * directionWS.z) *
                  inv_texture_tiling.y;

               const float4x4 texture_from_world = {
                  {column0.x, column1.x, 0.0f, 0.0f},
                  {column0.y, column1.y, 0.0f, 0.0f},
                  {column0.z, column1.z, 1.0f, 0.0f},
                  {texture_offset.x, texture_offset.y, 0.0f, 1.0f},
               };

               _global_lights[0].texture_from_world = texture_from_world;
            }
         }
      }

      if (world.global_lights.global_light_2.has_index()) {
         const world::light& light =
            world.lights[world.global_lights.global_light_2.index()];

         _global_lights[1] = {
            .directionWS = normalize(light.rotation * float3{0.0f, 0.0f, -1.0f}),
            .flags = not light.static_ ? light_flags::is_dynamic : light_flags::none,
            .color = light.color,
            .has_shadows = false,
         };

         if (not light.texture.empty()) {
            std::optional<uint32> texture =
               _textures.try_get(light.texture, world_texture_dimension::_2d);

            if (texture) {
               _global_lights[1].flags |= light_flags::has_texture;
               _global_lights[1].texture_index = *texture;
               _global_lights[1].texture_clamp =
                  light.texture_addressing == world::texture_addressing::clamp;

               const float2 inv_texture_tiling = 1.0f / light.directional_texture_tiling;
               const float2 texture_offset =
                  light.directional_texture_offset * inv_texture_tiling;
               const float3& directionWS = _global_lights[1].directionWS;

               const float3 column0 =
                  (float3{1.0f, 0.0f, 0.0f} - directionWS * directionWS.x) *
                  inv_texture_tiling.x;
               const float3 column1 =
                  (float3{0.0f, 0.0f, 1.0f} - directionWS * directionWS.z) *
                  inv_texture_tiling.y;

               const float4x4 texture_from_world = {
                  {column0.x, column1.x, 0.0f, 0.0f},
                  {column0.y, column1.y, 0.0f, 0.0f},
                  {column0.z, column1.z, 1.0f, 0.0f},
                  {texture_offset.x, texture_offset.y, 0.0f, 1.0f},
               };

               _global_lights[1].texture_from_world = texture_from_world;
            }
         }
      }

      std::array<std::span<const world::light>, 2> light_arrays =
         {optional_placement_light ? std::span{optional_placement_light, 1}
                                   : std::span<const world::light>{},
          world.lights};

      for (const std::span<const world::light> lights : light_arrays) {
         for (const world::light& light : lights) {
            if (not light.texture.empty()) {
               _textures.try_get(light.texture, world_texture_dimension::_2d); // The dimension doesn't matter here, this is just to ref the texture.
            }

            const light_flags base_flags =
               not light.static_ ? light_flags::is_dynamic : light_flags::none;

            switch (light.light_type) {
            case world::light_type::directional: {
               // Directional lights don't go in the main light list.
            } break;
            case world::light_type::point: {
               if (not intersects(view_frustum, light.position, light.range)) {
                  continue;
               }

               const float light_distance =
                  distance(view_camera.position(), light.position) - light.range;

               if (light_description* added_light = try_add_light(light_distance);
                   added_light) {
                  *added_light = {.type = light_type::point,
                                  .flags = base_flags,

                                  .point = {
                                     .positionWS = light.position,
                                     .range = light.range,
                                     .color = light.color,
                                  }};

                  if (not light.texture.empty()) {
                     std::optional<uint32> texture =
                        _textures.try_get(light.texture, world_texture_dimension::cube);

                     if (texture) {
                        added_light->flags |= light_flags::has_texture;
                        added_light->texture_index = *texture;
                        added_light->texture_clamp = light.texture_addressing ==
                                                     world::texture_addressing::clamp;

                        const float inv_range = 1.0f / light.range;

                        added_light->texture_from_world = {
                           {inv_range, 0.0f, 0.0f, 0.0f},
                           {0.0f, inv_range, 0.0f, 0.0f},
                           {0.0f, 0.0f, inv_range, 0.0f},
                           {-light.position * inv_range, 1.0f},
                        };
                     }
                  }
               }
            } break;
            case world::light_type::spot: {
               const float tan_half_outer_cone_angle =
                  std::tan(light.outer_cone_angle * 0.5f);
               const float outer_cone_radius = light.range * tan_half_outer_cone_angle;
               const float3 light_directionWS =
                  normalize(light.rotation * float3{0.0f, 0.0f, 1.0f});
               const float3 cone_baseWS =
                  light.position + light_directionWS * light.range;
               const float3 e = outer_cone_radius *
                                sqrt(1.0f - light_directionWS * light_directionWS);

               const math::bounding_box bbox{.min = min(cone_baseWS - e, light.position),
                                             .max = max(cone_baseWS + e, light.position)};

               if (not intersects(view_frustum, bbox)) {
                  continue;
               }

               const float light_distance_conservative =
                  distance(view_camera.position(), light.position) - light.range;

               if (light_description* added_light =
                      try_add_light(light_distance_conservative);
                   added_light) {
                  const float cos_outer_cone_angle =
                     std::cos(light.outer_cone_angle / 2.0f);
                  const float cos_inner_cone_angle =
                     std::cos(light.inner_cone_angle / 2.0f);

                  *added_light = {.type = light_type::spot,
                                  .flags = base_flags,

                                  .spot = {
                                     .positionWS = light.position,
                                     .range = light.range,
                                     .color = light.color,
                                     .directionWS = -light_directionWS,
                                     .spot_outer_param = cos_outer_cone_angle,
                                     .spot_inner_param =
                                        1.0f / (cos_inner_cone_angle - cos_outer_cone_angle),
                                  }};

                  if (not light.texture.empty()) {
                     std::optional<uint32> texture =
                        _textures.try_get(light.texture, world_texture_dimension::_2d);

                     if (texture) {
                        added_light->flags |= light_flags::has_texture;
                        added_light->texture_index = *texture;
                        added_light->texture_clamp = light.texture_addressing ==
                                                     world::texture_addressing::clamp;

                        const float4x4 world_from_light =
                           std::abs(dot(light_directionWS, {0.0f, 1.0f, 0.0f})) <= 0.99f
                              ? make_direction_transform(light_directionWS,
                                                         {0.0f, 1.0f, 0.0f},
                                                         light.position)
                              : make_direction_transform(light_directionWS,
                                                         {1.0f, 0.0f, 0.0f},
                                                         light.position);
                        const float4x4 light_from_world =
                           inverse_rotation_translation(world_from_light);

                        float4x4 texture_from_light;

                        texture_from_light[0].x =
                           (1.0f / tan_half_outer_cone_angle) * 0.5f;
                        texture_from_light[1].y = texture_from_light[0].x;
                        texture_from_light[2] = {0.5f, 0.5f, 0.0f, 1.0f};
                        texture_from_light[3] = {0.0f, 0.0f, 0.0f, 0.0f};

                        added_light->texture_from_world =
                           texture_from_light * light_from_world;
                     }
                  }
               }
            } break;
            case world::light_type::directional_region_box:
            case world::light_type::directional_region_sphere:
            case world::light_type::directional_region_cylinder: {
               float4x4 world_from_region = to_matrix(light.region_rotation);
               world_from_region[3] = {light.position, 1.0f};

               float4x4 region_from_world = transpose(world_from_region);
               region_from_world[3] = {float3x3{region_from_world} * -light.position,
                                       1.0f};

               light_description* added_light = nullptr;

               switch (light.light_type) {
               case world::light_type::directional_region_box: {
                  math::bounding_box bbox{.min = -light.region_size,
                                          .max = light.region_size};

                  bbox = light.region_rotation * bbox + light.position;

                  if (not intersects(view_frustum, bbox)) {
                     continue;
                  }

                  const float3 camera_positionRS =
                     region_from_world * view_camera.position();
                  const float3 q = abs(camera_positionRS) - light.region_size;

                  const float light_distance =
                     length(max(q, float3{0.0f, 0.0f, 0.0f})) -
                     std::min(std::max(std::max(q.x, q.y), q.z), 0.0f);

                  added_light = try_add_light(light_distance);

                  if (added_light) {
                     *added_light = {.type = light_type::directional_box,
                                     .flags = base_flags,

                                     .directional_box = {
                                        .color = light.color,
                                        .directionWS = normalize(
                                           light.rotation * float3{0.0f, 0.0f, -1.0f}),
                                        .world_from_region = world_from_region,
                                        .region_from_world = region_from_world,
                                        .size = light.region_size,
                                     }};
                  }
               } break;
               case world::light_type::directional_region_sphere: {
                  const float sphere_radius = length(light.region_size);

                  if (not intersects(view_frustum, light.position, sphere_radius)) {
                     continue;
                  }

                  const float light_distance =
                     distance(view_camera.position(), light.position) - light.range;

                  added_light = try_add_light(light_distance);

                  if (added_light) {
                     *added_light = {.type = light_type::directional_sphere,
                                     .flags = base_flags,

                                     .directional_sphere = {
                                        .color = light.color,
                                        .directionWS = normalize(
                                           light.rotation * float3{0.0f, 0.0f, -1.0f}),
                                        .world_from_region = world_from_region,
                                        .region_from_world = region_from_world,
                                        .radius = light.region_size.x,
                                     }};
                  }
               } break;
               case world::light_type::directional_region_cylinder: {
                  const float radius =
                     length(float2{light.region_size.x, light.region_size.z});
                  const float height = light.region_size.y;

                  math::bounding_box bbox{.min = {-radius, -height, -radius},
                                          .max = {radius, height, radius}};

                  bbox = light.region_rotation * bbox + light.position;

                  if (not intersects(view_frustum, bbox)) {
                     continue;
                  }

                  const float3 camera_positionRS =
                     region_from_world * view_camera.position();

                  const float cap_distance =
                     std::max(std::abs(camera_positionRS.y) - height, 0.0f);
                  const float edge_distance =
                     std::max(length(float2{camera_positionRS.x,
                                            camera_positionRS.z}) -
                                 radius,
                              0.0f);
                  const float light_distance = std::max(cap_distance, edge_distance);

                  added_light = try_add_light(light_distance);

                  if (added_light) {
                     *added_light = {.type = light_type::directional_cylinder,
                                     .flags = base_flags,

                                     .directional_cylinder = {
                                        .color = light.color,
                                        .directionWS = normalize(
                                           light.rotation * float3{0.0f, 0.0f, -1.0f}),
                                        .world_from_region = world_from_region,
                                        .region_from_world = region_from_world,
                                        .radius = radius,
                                        .height = height,
                                     }};
                  }
               } break;
               default:
                  break;
               }

               if (added_light and not light.texture.empty()) {
                  std::optional<uint32> texture =
                     _textures.try_get(light.texture, world_texture_dimension::_2d);

                  if (texture) {
                     added_light->flags |= light_flags::has_texture;
                     added_light->texture_index = *texture;
                     added_light->texture_clamp = light.texture_addressing ==
                                                  world::texture_addressing::clamp;

                     const float2 inv_texture_tiling =
                        1.0f / light.directional_texture_tiling;
                     const float2 texture_offset =
                        light.directional_texture_offset * inv_texture_tiling;
                     float3 directionWS;

                     switch (light.light_type) {
                     case world::light_type::directional_region_box: {
                        directionWS = added_light->directional_box.directionWS;
                     } break;
                     case world::light_type::directional_region_sphere: {
                        directionWS = added_light->directional_sphere.directionWS;
                     } break;
                     case world::light_type::directional_region_cylinder: {
                        directionWS = added_light->directional_cylinder.directionWS;
                     } break;
                     default:
                        break;
                     }

                     const float3 column0 =
                        (float3{1.0f, 0.0f, 0.0f} - directionWS * directionWS.x) *
                        inv_texture_tiling.x;
                     const float3 column1 =
                        (float3{0.0f, 0.0f, 1.0f} - directionWS * directionWS.z) *
                        inv_texture_tiling.y;

                     const float4x4 texture_from_world = {
                        {column0.x, column1.x, 0.0f, 0.0f},
                        {column0.y, column1.y, 0.0f, 0.0f},
                        {column0.z, column1.z, 1.0f, 0.0f},
                        {texture_offset.x, texture_offset.y, 0.0f, 1.0f},
                     };

                     added_light->texture_from_world = texture_from_world;
                  }
               }
            } break;
            }
         }
      }

      if (optional_entity_group) {
         const quaternion& group_rotation = optional_entity_group->rotation;
         const float3& group_position = optional_entity_group->position;

         for (const world::light& light : optional_entity_group->lights) {
            if (not light.texture.empty()) {
               _textures.try_get(light.texture, world_texture_dimension::_2d); // The dimension doesn't matter here, this is just to ref the texture.
            }

            const light_flags base_flags =
               not light.static_ ? light_flags::is_dynamic : light_flags::none;

            const float3 light_positionWS =
               group_rotation * light.position + group_position;

            switch (light.light_type) {
            case world::light_type::directional: {
               // Directional lights don't go in the main light list.
            } break;
            case world::light_type::point: {
               if (not intersects(view_frustum, light_positionWS, light.range)) {
                  continue;
               }

               const float light_distance =
                  distance(view_camera.position(), light_positionWS) - light.range;

               if (light_description* added_light = try_add_light(light_distance);
                   added_light) {
                  *added_light = {.type = light_type::point,
                                  .flags = base_flags,

                                  .point = {
                                     .positionWS = light_positionWS,
                                     .range = light.range,
                                     .color = light.color,
                                  }};

                  if (not light.texture.empty()) {
                     std::optional<uint32> texture =
                        _textures.try_get(light.texture, world_texture_dimension::cube);

                     if (texture) {
                        added_light->flags |= light_flags::has_texture;
                        added_light->texture_index = *texture;
                        added_light->texture_clamp = light.texture_addressing ==
                                                     world::texture_addressing::clamp;

                        const float inv_range = 1.0f / light.range;

                        added_light->texture_from_world = {
                           {inv_range, 0.0f, 0.0f, 0.0f},
                           {0.0f, inv_range, 0.0f, 0.0f},
                           {0.0f, 0.0f, inv_range, 0.0f},
                           {-light_positionWS * inv_range, 1.0f},
                        };
                     }
                  }
               }
            } break;
            case world::light_type::spot: {
               const float tan_half_outer_cone_angle =
                  std::tan(light.outer_cone_angle * 0.5f);
               const float outer_cone_radius = light.range * tan_half_outer_cone_angle;
               const float3 light_directionWS = normalize(
                  group_rotation * light.rotation * float3{0.0f, 0.0f, 1.0f});
               const float3 cone_baseWS =
                  light_positionWS + light_directionWS * light.range;
               const float3 e = outer_cone_radius *
                                sqrt(1.0f - light_directionWS * light_directionWS);

               const math::bounding_box bbox{.min = min(cone_baseWS - e, light_positionWS),
                                             .max = max(cone_baseWS + e,
                                                        light_positionWS)};

               if (not intersects(view_frustum, bbox)) {
                  continue;
               }

               const float light_distance_conservative =
                  distance(view_camera.position(), light_positionWS) - light.range;

               if (light_description* added_light =
                      try_add_light(light_distance_conservative);
                   added_light) {
                  const float cos_outer_cone_angle =
                     std::cos(light.outer_cone_angle / 2.0f);
                  const float cos_inner_cone_angle =
                     std::cos(light.inner_cone_angle / 2.0f);

                  *added_light = {.type = light_type::spot,
                                  .flags = base_flags,

                                  .spot = {
                                     .positionWS = light_positionWS,
                                     .range = light.range,
                                     .color = light.color,
                                     .directionWS = -light_directionWS,
                                     .spot_outer_param = cos_outer_cone_angle,
                                     .spot_inner_param =
                                        1.0f / (cos_inner_cone_angle - cos_outer_cone_angle),
                                  }};

                  if (not light.texture.empty()) {
                     std::optional<uint32> texture =
                        _textures.try_get(light.texture, world_texture_dimension::_2d);

                     if (texture) {
                        added_light->flags |= light_flags::has_texture;
                        added_light->texture_index = *texture;
                        added_light->texture_clamp = light.texture_addressing ==
                                                     world::texture_addressing::clamp;

                        const float4x4 world_from_light =
                           std::abs(dot(light_directionWS, {0.0f, 1.0f, 0.0f})) <= 0.99f
                              ? make_direction_transform(light_directionWS,
                                                         {0.0f, 1.0f, 0.0f},
                                                         light_positionWS)
                              : make_direction_transform(light_directionWS,
                                                         {1.0f, 0.0f, 0.0f},
                                                         light_positionWS);
                        const float4x4 light_from_world =
                           inverse_rotation_translation(world_from_light);

                        float4x4 texture_from_light;

                        texture_from_light[0].x =
                           (1.0f / tan_half_outer_cone_angle) * 0.5f;
                        texture_from_light[1].y = texture_from_light[0].x;
                        texture_from_light[2] = {0.5f, 0.5f, 0.0f, 1.0f};
                        texture_from_light[3] = {0.0f, 0.0f, 0.0f, 0.0f};

                        added_light->texture_from_world =
                           texture_from_light * light_from_world;
                     }
                  }
               }
            } break;
            case world::light_type::directional_region_box:
            case world::light_type::directional_region_sphere:
            case world::light_type::directional_region_cylinder: {
               const quaternion light_region_rotation =
                  group_rotation * light.region_rotation;

               float4x4 world_from_region = to_matrix(light_region_rotation);
               world_from_region[3] = {light.position, 1.0f};

               float4x4 region_from_world = transpose(world_from_region);
               region_from_world[3] = {float3x3{region_from_world} * -light.position,
                                       1.0f};

               light_description* added_light = nullptr;

               switch (light.light_type) {
               case world::light_type::directional_region_box: {
                  math::bounding_box bbox{.min = -light.region_size,
                                          .max = light.region_size};

                  bbox = light_region_rotation * bbox + light_positionWS;

                  if (not intersects(view_frustum, bbox)) {
                     continue;
                  }

                  const float3 camera_positionRS =
                     region_from_world * view_camera.position();
                  const float3 q = abs(camera_positionRS) - light.region_size;

                  const float light_distance =
                     length(max(q, float3{0.0f, 0.0f, 0.0f})) -
                     std::min(std::max(std::max(q.x, q.y), q.z), 0.0f);

                  added_light = try_add_light(light_distance);

                  if (added_light) {
                     *added_light = {.type = light_type::directional_box,
                                     .flags = base_flags,

                                     .directional_box = {
                                        .color = light.color,
                                        .directionWS =
                                           normalize(group_rotation * light.rotation *
                                                     float3{0.0f, 0.0f, 1.0f}),
                                        .world_from_region = world_from_region,
                                        .region_from_world = region_from_world,
                                        .size = light.region_size,
                                     }};
                  }
               } break;
               case world::light_type::directional_region_sphere: {
                  const float sphere_radius = length(light.region_size);

                  if (not intersects(view_frustum, light_positionWS, sphere_radius)) {
                     continue;
                  }

                  const float light_distance =
                     distance(view_camera.position(), light_positionWS) - light.range;

                  added_light = try_add_light(light_distance);

                  if (added_light) {
                     *added_light = {.type = light_type::directional_sphere,
                                     .flags = base_flags,

                                     .directional_sphere = {
                                        .color = light.color,
                                        .directionWS =
                                           normalize(group_rotation * light.rotation *
                                                     float3{0.0f, 0.0f, 1.0f}),
                                        .world_from_region = world_from_region,
                                        .region_from_world = region_from_world,
                                        .radius = light.region_size.x,
                                     }};
                  }
               } break;
               case world::light_type::directional_region_cylinder: {
                  const float radius =
                     length(float2{light.region_size.x, light.region_size.z});
                  const float height = light.region_size.y;

                  math::bounding_box bbox{.min = {-radius, -height, -radius},
                                          .max = {radius, height, radius}};

                  bbox = light_region_rotation * bbox + light_positionWS;

                  if (not intersects(view_frustum, bbox)) {
                     continue;
                  }

                  const float3 camera_positionRS =
                     region_from_world * view_camera.position();

                  const float cap_distance =
                     std::max(std::abs(camera_positionRS.y) - height, 0.0f);
                  const float edge_distance =
                     std::max(length(float2{camera_positionRS.x,
                                            camera_positionRS.z}) -
                                 radius,
                              0.0f);
                  const float light_distance = std::max(cap_distance, edge_distance);

                  added_light = try_add_light(light_distance);

                  if (added_light) {
                     *added_light = {.type = light_type::directional_cylinder,
                                     .flags = base_flags,

                                     .directional_cylinder = {
                                        .color = light.color,
                                        .directionWS =
                                           normalize(group_rotation * light.rotation *
                                                     float3{0.0f, 0.0f, 1.0f}),
                                        .world_from_region = world_from_region,
                                        .region_from_world = region_from_world,
                                        .radius = radius,
                                        .height = height,
                                     }};
                  }
               } break;
               default:
                  break;
               }

               if (added_light and not light.texture.empty()) {
                  std::optional<uint32> texture =
                     _textures.try_get(light.texture, world_texture_dimension::_2d);

                  if (texture) {
                     added_light->flags |= light_flags::has_texture;
                     added_light->texture_index = *texture;
                     added_light->texture_clamp = light.texture_addressing ==
                                                  world::texture_addressing::clamp;

                     const float2 inv_texture_tiling =
                        1.0f / light.directional_texture_tiling;
                     const float2 texture_offset =
                        light.directional_texture_offset * inv_texture_tiling;
                     float3 directionWS;

                     switch (light.light_type) {
                     case world::light_type::directional_region_box: {
                        directionWS = added_light->directional_box.directionWS;
                     } break;
                     case world::light_type::directional_region_sphere: {
                        directionWS = added_light->directional_sphere.directionWS;
                     } break;
                     case world::light_type::directional_region_cylinder: {
                        directionWS = added_light->directional_cylinder.directionWS;
                     } break;
                     default:
                        break;
                     }

                     const float3 column0 =
                        float3{1.0f, 0.0f, 0.0f} -
                        directionWS * directionWS.x * inv_texture_tiling.x;
                     const float3 column1 =
                        float3{0.0f, 0.0f, 1.0f} -
                        directionWS * directionWS.z * inv_texture_tiling.y;

                     const float4x4 texture_from_world = {
                        {column0.x, column1.x, 0.0f, 0.0f},
                        {column0.y, column1.y, 0.0f, 0.0f},
                        {column0.z, column1.z, 1.0f, 0.0f},
                        {texture_offset.x, texture_offset.y, 0.0f, 1.0f},
                     };

                     added_light->texture_from_world = texture_from_world;
                  }
               }
            } break;
            }
         }
      }
   }

   auto try_add_light(float distance) noexcept -> light_description*
   {
      light_entry* const lights_order_end = _lights_order.data() + _lights_allocated;

      auto slot =
         std::lower_bound(_lights_order.data(), lights_order_end, distance,
                          [](const light_entry& entry, const float distance) {
                             return entry.distance < distance;
                          });

      uint32 light_index;

      if (slot == lights_order_end) {
         if (_lights_allocated < max_onscreen_lights) {
            light_index = _lights_allocated;
            slot = lights_order_end;

            _lights_allocated += 1;
         }
         else {
            return nullptr;
         }
      }
      else {
         if (_lights_allocated == max_onscreen_lights) {
            light_index = _lights_order[_lights_allocated - 1].light_index;
         }
         else {
            light_index = _lights_allocated;

            _lights_allocated += 1;
         }

         std::memmove(slot + 1, slot, sizeof(light_entry) * (lights_order_end - slot));
      }

      *slot = {.distance = distance, .light_index = light_index};

      return &_lights[light_index];
   }

   gpu::device& _device;

   gpu::unique_resource_handle _tiling_inputs;

   gpu::unique_resource_handle _lights_constants;
   gpu::unique_resource_handle _lights_tiles;

   gpu::unique_resource_handle _sphere_proxy_indices;
   gpu::unique_resource_handle _sphere_proxy_vertices;

   gpu::unique_resource_handle _shadow_map;
   std::array<gpu::unique_dsv_handle, 4> _shadow_map_dsv;

   gpu_virtual_address _tiling_inputs_cbv;

   gpu_virtual_address _lights_constant_buffer_view;
   gpu::unique_resource_view _lights_tiles_srv;
   gpu::unique_resource_view _lights_region_list_srv;
   gpu::unique_resource_view _shadow_map_srv;

   uint32 _tiles_count = 0;
   uint32 _tiles_width = 0;
   uint32 _tiles_height = 0;
   float _render_width = 0.0f;
   float _render_height = 0.0f;

   uint32 _light_proxy_count = 0;
   gpu_virtual_address _sphere_light_proxies_srv = 0;

   std::array<float, 2> _scene_depth_min_max = {0.0f, 1.0f};

   float3 _ambient_sky_color;
   float3 _ambient_ground_color;

   std::array<global_light, 2> _global_lights;

   struct light_entry {
      float distance = FLT_MAX;
      uint32 light_index = 0;
   };

   std::array<light_entry, max_onscreen_lights> _lights_order;
   std::array<light_description, max_onscreen_lights> _lights;

   uint32 _lights_allocated = 0;

   struct light_texture {
      lowercase_string name;

      std::shared_ptr<const world_texture> texture;
      std::shared_ptr<const world_texture_load_token> load_token;
   };

   light_textures _textures;

   bool _has_sun_shadows = false;

   std::array<shadow_ortho_camera, sun_cascade_count> _sun_shadow_cascades;
   std::array<blocks::view, sun_cascade_count> _sun_shadow_blocks_view;
   std::array<billboard_patches::view, sun_cascade_count> _sun_shadow_billboard_patches_view;
   std::vector<uint16> _shadow_render_list;
};

light_clusters::light_clusters(gpu::device& device, texture_manager& texture_manager,
                               copy_command_list_pool& copy_command_list_pool,
                               uint32 render_width, uint32 render_height)
   : _impl{std::make_unique<impl>(device, texture_manager, copy_command_list_pool,
                                  render_width, render_height)}
{
}

light_clusters::~light_clusters() = default;

void light_clusters::update_render_resolution(uint32 width, uint32 height)
{
   _impl->update_render_resolution(width, height);
}

void light_clusters::prepare_lights(
   const camera& view_camera, const frustum& view_frustum,
   const world::world& world, const world::light* optional_placement_light,
   const world::entity_group* optional_entity_group,
   const std::array<float, 2> scene_depth_min_max, blocks& blocks,
   billboard_patches& billboard_patches, const world::active_layers active_layers,
   const world::active_entity_types active_entity_types,
   gpu::copy_command_list& command_list,
   dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   _impl->prepare_lights(view_camera, view_frustum, world, optional_placement_light,
                         optional_entity_group, scene_depth_min_max, blocks,
                         billboard_patches, active_layers, active_entity_types,
                         command_list, dynamic_buffer_allocator);
}

void light_clusters::tile_lights(root_signature_library& root_signatures,
                                 pipeline_library& pipelines,
                                 gpu::graphics_command_list& command_list,
                                 profiler& profiler)
{
   _impl->tile_lights(root_signatures, pipelines, command_list, profiler);
}

void light_clusters::draw_shadow_maps(
   const world_mesh_list& meshes, const blocks& blocks,
   const billboard_patches& billboard_patches, root_signature_library& root_signatures,
   pipeline_library& pipelines, gpu::graphics_command_list& command_list,
   dynamic_buffer_allocator& dynamic_buffer_allocator, profiler& profiler)
{
   _impl->draw_shadow_maps(meshes, blocks, billboard_patches, root_signatures, pipelines,
                           command_list, dynamic_buffer_allocator, profiler);
}

auto light_clusters::lights_constant_buffer_view() const noexcept -> gpu_virtual_address
{
   return _impl->lights_constant_buffer_view();
}

void light_clusters::process_updated_textures(const updated_textures& updated) noexcept
{
   _impl->process_updated_textures(updated);
}

}
