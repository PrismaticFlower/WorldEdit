
#include "light_clusters.hpp"
#include "cull_objects.hpp"
#include "math/align.hpp"
#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <cmath>

namespace we::graphics {

namespace {

constexpr auto shadow_res = 2048;
constexpr auto cascade_count = light_clusters::sun_cascade_count;
constexpr auto light_tile_size = 8;
constexpr auto max_lights = 256;
constexpr auto tile_light_word_bits = 32;
constexpr auto tile_light_words = max_lights / tile_light_word_bits;
constexpr auto max_regional_lights = 256;

enum class light_type : uint32 { directional, point, spot };
enum class directional_region_type : uint32 { none, box, sphere, cylinder };

struct alignas(16) light_tile_clear_inputs {
   std::array<uint32, 2> tile_counts;
   std::array<uint32, 2> padding0{};

   std::array<uint32, 8> clear_value;
};

static_assert(sizeof(light_tile_clear_inputs) == 48);

struct alignas(16) tiling_inputs {
   std::array<uint32, 2> tile_counts;
   std::array<uint32, 2> padding0{};

   float4x4 view_projection_matrix;
};

static_assert(sizeof(tiling_inputs) == 80);

struct alignas(16) light_description {
   float3 direction;
   light_type type;
   float3 position;
   float range;
   float3 color;
   float spot_outer_param;
   float spot_inner_param;
   directional_region_type region_type;
   uint32 directional_region_index;
   uint32 shadow_caster;
};

static_assert(sizeof(light_description) == 64);

struct alignas(16) light_constants {
   uint32 light_tiles_width;
   gpu::resource_view light_tiles_index;
   gpu::resource_view light_region_list_index;
   gpu::resource_view shadow_map_index;

   float3 sky_ambient_color;
   uint32 padding1;

   float3 ground_ambient_color;
   uint32 padding2;

   std::array<float4x4, 4> shadow_transforms;

   float2 shadow_resolution;
   float2 inv_shadow_resolution;

   std::array<light_description, max_lights> lights;
};

static_assert(sizeof(light_constants) == 16704);

struct light_region_description {
   float4x4 inverse_transform;
   float3 position;
   directional_region_type type;
   float3 size;
   uint32 padding;
};

static_assert(sizeof(light_region_description) == 96);

struct alignas(16) light_proxy_instance {
   std::array<float4, 3> transform;
   uint32 light_index;
   std::array<uint32, 3> padding;
};

static_assert(sizeof(light_proxy_instance) == 64);

auto make_sphere_light_proxy_transform(const world::light& light, const float radius)
   -> std::array<float4, 3>
{
   float4x4 light_transform = {{radius, 0.0f, 0.0f, 0.0f}, //
                               {0.0f, radius, 0.0f, 0.0f}, //
                               {0.0f, 0.0f, radius, 0.0f}, //
                               {0.0f, 0.0f, 0.0f, 1.0f}};
   light_transform[3] = {light.position, 1.0f};
   light_transform = transpose(
      light_transform); // transpose so we have good alignment for a float3x4 in HLSL

   return {light_transform[0], light_transform[1], light_transform[2]};
}

struct upload_data {
   upload_data(dynamic_buffer_allocator& dynamic_buffer_allocator)
   {
      {
         auto allocation = dynamic_buffer_allocator.allocate(sizeof(light_constants));

         constants = reinterpret_cast<light_constants*>(allocation.cpu_address);
         constants_offset = allocation.offset;
      }

      {
         auto allocation = dynamic_buffer_allocator.allocate(
            sizeof(std::array<light_region_description, max_regional_lights>));

         regional_lights_descriptions =
            reinterpret_cast<light_region_description*>(allocation.cpu_address);
         regional_lights_descriptions_offset = allocation.offset;
      }

      {
         auto allocation = dynamic_buffer_allocator.allocate(
            sizeof(std::array<light_proxy_instance, max_lights>));

         sphere_light_proxies =
            reinterpret_cast<light_proxy_instance*>(allocation.cpu_address);
         sphere_light_proxies_gpu = allocation.gpu_address;
      }
   }

   light_constants* constants;
   light_region_description* regional_lights_descriptions;
   light_proxy_instance* sphere_light_proxies;

   uint64 constants_offset;
   uint64 regional_lights_descriptions_offset;
   gpu_virtual_address sphere_light_proxies_gpu;
};

template<typename T>
void upload_to(const T& v, T* const to)
{
   std::memcpy(to, &v, sizeof(T));
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

auto make_shadow_cascade_splits(const camera& camera)
   -> std::array<float, cascade_count + 1>
{
   const float clip_ratio = camera.far_clip() / camera.near_clip();
   const float clip_range = camera.far_clip() - camera.near_clip();

   std::array<float, 5> cascade_splits{};

   for (int i = 0; i < cascade_splits.size(); ++i) {
      const float split = (camera.near_clip() * std::pow(clip_ratio, i / 4.0f));
      const float split_normalized = (split - camera.near_clip()) / clip_range;

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

   auto shadow_view_projection = shadow_camera.view_projection_matrix();

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
   const frustum view_frustum{camera.inv_view_projection_matrix(),
                              scene_depth_min_max[0], scene_depth_min_max[1]};

   const float3 light_direction =
      normalize(light_rotation * float3{0.0f, 0.0f, -1.0f});

   const std::array cascade_splits = make_shadow_cascade_splits(camera);

   std::array<shadow_ortho_camera, cascade_count> cameras;

   for (int i = 0; i < cascade_count; ++i) {
      cameras[i] = make_cascade_shadow_camera(light_direction, cascade_splits[i],
                                              cascade_splits[i + 1], view_frustum);
   }

   return cameras;
}

}

light_clusters::light_clusters(gpu::device& device,
                               copy_command_list_pool& copy_command_list_pool,
                               uint32 render_width, uint32 render_height)
   : _device{device}
{
   _tiling_inputs = {device.create_buffer({.size = sizeof(tiling_inputs),
                                           .debug_name =
                                              "Lights Tiling Inputs"},
                                          gpu::heap_type::default_),
                     device.direct_queue};

   _lights_constants = {device.create_buffer({.size = sizeof(light_constants),
                                              .debug_name =
                                                 "Lights Constant Buffer"},
                                             gpu::heap_type::default_),
                        device.direct_queue};

   _lights_region_list =
      {device.create_buffer({.size = sizeof(light_region_description) * max_regional_lights,
                             .debug_name = "Lights Region List"},
                            gpu::heap_type::default_),
       device.direct_queue};

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
                     gpu::barrier_layout::direct_queue_shader_resource),
                  device.direct_queue};

   for (uint32 i = 0; i < cascade_count; ++i) {
      _shadow_map_dsv[i] = {device.create_depth_stencil_view(
                               _shadow_map.get(),
                               {.format = DXGI_FORMAT_D32_FLOAT,
                                .dimension = gpu::dsv_dimension::texture2d_array,

                                .texture2d_array = {.first_array_slice = i,
                                                    .array_size = 1}}),
                            device.direct_queue};
   }

   update_render_resolution(render_width, render_height, false);
   update_descriptors();

   init_proxy_geometry(device, copy_command_list_pool);
}

void light_clusters::update_render_resolution(uint32 width, uint32 height)
{
   update_render_resolution(width, height, true);
}

void light_clusters::update_render_resolution(uint32 width, uint32 height,
                                              bool recreate_descriptors)
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
                    _device.direct_queue};

   _tiles_count = tiles_count;
   _tiles_width = tiles_width;
   _tiles_height = tiles_height;
   _render_width = static_cast<float>(width);
   _render_height = static_cast<float>(height);

   if (recreate_descriptors) update_descriptors();
}

void light_clusters::update_descriptors()
{
   _lights_constant_buffer_view =
      _device.get_gpu_virtual_address(_lights_constants.get());

   _lights_tiles_srv = {_device.create_shader_resource_view(
                           _lights_tiles.get(),
                           {.buffer = {.first_element = 0,
                                       .number_elements = _tiles_count,
                                       .structure_byte_stride = sizeof(uint32) * 8}}),
                        _device.direct_queue};

   _lights_region_list_srv = {_device.create_shader_resource_view(
                                 _lights_region_list.get(),
                                 {.buffer = {.first_element = 0,
                                             .number_elements = max_regional_lights,
                                             .structure_byte_stride =
                                                sizeof(light_region_description)}}),
                              _device.direct_queue};

   _shadow_map_srv = {_device.create_shader_resource_view(_shadow_map.get(),
                                                          {.format = DXGI_FORMAT_R32_FLOAT}),
                      _device.direct_queue};

   _tiling_inputs_cbv = _device.get_gpu_virtual_address(_tiling_inputs.get());
}

void light_clusters::prepare_lights(const camera& view_camera,
                                    const frustum& view_frustum,
                                    const world::world& world,
                                    const std::array<float, 2> scene_depth_min_max,
                                    gpu::copy_command_list& command_list,
                                    dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   upload_data upload_data{dynamic_buffer_allocator};

   static_assert((sizeof(_tiles_start_value) / sizeof(uint32)) == tile_light_words);

   _tiles_start_value = {};
   _light_count = 0;
   _sphere_light_proxies_srv = upload_data.sphere_light_proxies_gpu;
   uint32 regional_lights_count = 0;

   // Upload light constants.
   {
      upload_to(_tiles_width, &upload_data.constants->light_tiles_width);
      upload_to(_lights_tiles_srv.get(), &upload_data.constants->light_tiles_index);
      upload_to(_lights_region_list_srv.get(),
                &upload_data.constants->light_region_list_index);
      upload_to(_shadow_map_srv.get(), &upload_data.constants->shadow_map_index);
      upload_to(world.lighting_settings.ambient_sky_color,
                &upload_data.constants->sky_ambient_color);
      upload_to(world.lighting_settings.ambient_ground_color,
                &upload_data.constants->ground_ambient_color);
      upload_to({shadow_res, shadow_res}, &upload_data.constants->shadow_resolution);
      upload_to({1.0f / shadow_res, 1.0f / shadow_res},
                &upload_data.constants->inv_shadow_resolution);
   }

   // frustum cull lights
   for (auto& light : world.lights) {
      if (_light_count >= max_lights) break;

      const uint32 light_index = _light_count;

      switch (light.light_type) {
      case world::light_type::directional: {
         const float3 light_direction =
            normalize(light.rotation * float3{0.0f, 0.0f, -1.0f});

         if (light.shadow_caster) {
            _sun_shadow_cascades =
               make_shadow_cascades(light.rotation, view_camera, scene_depth_min_max);
         }

         upload_to({.direction = light_direction,
                    .type = light_type::directional,
                    .color = light.color,
                    .region_type = directional_region_type::none,
                    .shadow_caster = light.shadow_caster},
                   &upload_data.constants->lights[light_index]);

         _light_count += 1;

         // Set the directional light as part of the tile clear pass.
         _tiles_start_value[light_index / tile_light_word_bits] |=
            (1u << (light_index % tile_light_word_bits));
         break;
      }
      case world::light_type::point: {
         if (not intersects(view_frustum, light.position, light.range)) {
            continue;
         }

         upload_to({.type = light_type::point,
                    .position = light.position,
                    .range = light.range,
                    .color = light.color},
                   &upload_data.constants->lights[light_index]);

         upload_to({.transform = make_sphere_light_proxy_transform(light, light.range),

                    .light_index = light_index},
                   upload_data.sphere_light_proxies + light_index);

         _light_count += 1;

         break;
      }
      case world::light_type::spot: {
         const float3 light_direction =
            normalize(light.rotation * float3{0.0f, 0.0f, -1.0f});

         const float half_range = light.range / 2.0f;
         const float cone_radius = half_range * std::tan(light.outer_cone_angle);

         const float light_bounds_radius = std::max(cone_radius, half_range);
         const float3 light_centre =
            light.position - (light_direction * (half_range));

         // TODO: Cone-frustum intersection (but how much of a problem is it?)
         if (not intersects(view_frustum, light_centre, light_bounds_radius)) {
            continue;
         }

         upload_to({.direction = light_direction,
                    .type = light_type::spot,
                    .position = light.position,
                    .range = light.range,
                    .color = light.color,
                    .spot_outer_param = std::cos(light.outer_cone_angle / 2.0f),
                    .spot_inner_param =
                       1.0f / (std::cos(light.inner_cone_angle / 2.0f) -
                               std::cos(light.outer_cone_angle / 2.0f))},
                   &upload_data.constants->lights[light_index]);

         upload_to({.transform = make_sphere_light_proxy_transform(light, light.range), // TODO: Cone light proxies.
                    .light_index = light_index},
                   upload_data.sphere_light_proxies + light_index);

         _light_count += 1;

         break;
      }
      case world::light_type::directional_region_box:
      case world::light_type::directional_region_sphere:
      case world::light_type::directional_region_cylinder: {
         const float3 light_direction =
            normalize(light.rotation * float3{0.0f, 0.0f, -1.0f});

         if (regional_lights_count == max_regional_lights) {
            break;
         }

         const quaternion region_rotation_inverse = conjugate(light.region_rotation);
         float4x4 inverse_region_transform = to_matrix(region_rotation_inverse);
         inverse_region_transform[3] = {region_rotation_inverse * -light.position, 1.0f};

         inverse_region_transform = transpose(inverse_region_transform);

         const uint32 region_description_index = regional_lights_count;

         switch (light.light_type) {
         case world::light_type::directional_region_box: {
            upload_to({.inverse_transform = inverse_region_transform,
                       .position = light.position,
                       .type = directional_region_type::box,
                       .size = light.region_size},
                      upload_data.regional_lights_descriptions + region_description_index);

            upload_to({.direction = light_direction,
                       .type = light_type::directional,
                       .color = light.color,
                       .region_type = directional_region_type::box,
                       .directional_region_index = region_description_index},
                      &upload_data.constants->lights[light_index]);

            upload_to({.transform = make_sphere_light_proxy_transform(
                          light,
                          std::max({light.region_size.x, light.region_size.y,
                                    light.region_size.z})),

                       .light_index = light_index},
                      upload_data.sphere_light_proxies + light_index);

            _light_count += 1;
            regional_lights_count += 1;

            break;
         }
         case world::light_type::directional_region_sphere: {
            const float sphere_radius = length(light.region_size);

            upload_to({.inverse_transform = inverse_region_transform,
                       .position = light.position,
                       .type = directional_region_type::sphere,
                       .size = float3{sphere_radius, sphere_radius, sphere_radius}},
                      upload_data.regional_lights_descriptions + region_description_index);

            upload_to({.direction = light_direction,
                       .type = light_type::directional,
                       .color = light.color,
                       .region_type = directional_region_type::sphere,
                       .directional_region_index = region_description_index},
                      &upload_data.constants->lights[light_index]);

            upload_to({.transform = make_sphere_light_proxy_transform(light, sphere_radius),

                       .light_index = light_index},
                      upload_data.sphere_light_proxies + light_index);

            _light_count += 1;
            regional_lights_count += 1;

            break;
         }
         case world::light_type::directional_region_cylinder: {
            const float radius =
               length(float2{light.region_size.x, light.region_size.z});
            upload_to({.inverse_transform = inverse_region_transform,
                       .position = light.position,
                       .type = directional_region_type::cylinder,
                       .size = float3{radius, light.region_size.y, radius}},
                      upload_data.regional_lights_descriptions + region_description_index);

            upload_to({.direction = light_direction,
                       .type = light_type::directional,
                       .color = light.color,
                       .region_type = directional_region_type::cylinder,
                       .directional_region_index = region_description_index},
                      &upload_data.constants->lights[light_index]);

            upload_to({.transform = make_sphere_light_proxy_transform(
                          light, std::max(radius, light.region_size.y)),

                       .light_index = light_index},
                      upload_data.sphere_light_proxies + light_index);

            _light_count += 1;
            regional_lights_count += 1;

            break;
         }
         }

         break;
      }
      }
   }

   // Upload shadow transform.
   {
      upload_to(_sun_shadow_cascades[0].texture_matrix(),
                &upload_data.constants->shadow_transforms[0]);
      upload_to(_sun_shadow_cascades[1].texture_matrix(),
                &upload_data.constants->shadow_transforms[1]);
      upload_to(_sun_shadow_cascades[2].texture_matrix(),
                &upload_data.constants->shadow_transforms[2]);
      upload_to(_sun_shadow_cascades[3].texture_matrix(),
                &upload_data.constants->shadow_transforms[3]);
   }

   // copy tile culling inputs
   {
      tiling_inputs tiling_inputs{.tile_counts = {_tiles_width, _tiles_height},
                                  .view_projection_matrix =
                                     view_camera.view_projection_matrix()};

      auto upload_buffer = dynamic_buffer_allocator.allocate(sizeof(tiling_inputs));

      std::memcpy(upload_buffer.cpu_address, &tiling_inputs, sizeof(tiling_inputs));

      command_list.copy_buffer_region(_tiling_inputs.get(), 0,
                                      dynamic_buffer_allocator.resource(),
                                      upload_buffer.offset, sizeof(tiling_inputs));
   }

   command_list.copy_buffer_region(_lights_constants.get(), 0,
                                   dynamic_buffer_allocator.resource(),
                                   upload_data.constants_offset,
                                   sizeof(light_constants));

   command_list.copy_buffer_region(_lights_region_list.get(), 0,
                                   dynamic_buffer_allocator.resource(),
                                   upload_data.regional_lights_descriptions_offset,
                                   sizeof(light_region_description) *
                                      regional_lights_count);
}

void light_clusters::tile_lights(root_signature_library& root_signatures,
                                 pipeline_library& pipelines,
                                 gpu::graphics_command_list& command_list,
                                 dynamic_buffer_allocator& dynamic_buffer_allocator,
                                 profiler& profiler)
{
   profile_section profile{"Lights - Tile Lights", command_list, profiler,
                           profiler_queue::direct};

   // clear light tiles
   {
      command_list.set_compute_root_signature(root_signatures.tile_lights_clear.get());
      command_list.set_compute_cbv(rs::tile_lights_clear::input_cbv,
                                   dynamic_buffer_allocator
                                      .allocate_and_copy(light_tile_clear_inputs{
                                         .tile_counts = {_tiles_width, _tiles_height},
                                         .clear_value = _tiles_start_value})
                                      .gpu_address);
      command_list.set_compute_uav(rs::tile_lights_clear::light_tiles_uav,
                                   _device.get_gpu_virtual_address(_lights_tiles.get()));

      command_list.set_pipeline_state(pipelines.tile_lights_clear.get());

      command_list.dispatch(math::align_up(_tiles_width, 32) / 32,
                            math::align_up(_tiles_height, 32) / 32, 1);
   }

   command_list.deferred_barrier(
      gpu::buffer_barrier{.sync_before = gpu::barrier_sync::compute_shading,
                          .sync_after = gpu::barrier_sync::pixel_shading,
                          .access_before = gpu::barrier_access::unordered_access,
                          .access_after = gpu::barrier_access::unordered_access,
                          .resource = _lights_tiles.get()});
   command_list.flush_barriers();

   command_list.set_graphics_root_signature(root_signatures.tile_lights.get());
   command_list.set_graphics_uav(rs::tile_lights::light_tiles_uav,
                                 _device.get_gpu_virtual_address(_lights_tiles.get()));
   command_list.set_graphics_cbv(rs::tile_lights::cbv, _tiling_inputs_cbv);

   command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

   command_list.rs_set_scissor_rects({0, 0, _tiles_width, _tiles_height});
   command_list.rs_set_viewports({0.0f, 0.0f, static_cast<float>(_tiles_width),
                                  static_cast<float>(_tiles_height)});

   command_list.om_set_render_targets(std::span<const gpu::rtv_handle>{});

   // draw sphere lights
   {
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
                                          _light_count, 0, 0, 0);
   }

   command_list.deferred_barrier(
      gpu::buffer_barrier{.sync_before = gpu::barrier_sync::pixel_shading,
                          .sync_after = gpu::barrier_sync::pixel_shading,
                          .access_before = gpu::barrier_access::unordered_access,
                          .access_after = gpu::barrier_access::shader_resource,
                          .resource = _lights_tiles.get()});
}

void light_clusters::draw_shadow_maps(
   const world_mesh_list& meshes, root_signature_library& root_signatures,
   pipeline_library& pipelines, gpu::graphics_command_list& command_list,
   dynamic_buffer_allocator& dynamic_buffer_allocator, profiler& profiler)
{
   profile_section profile{"Lights - Draw Shadow Maps", command_list, profiler,
                           profiler_queue::direct};

   command_list.deferred_barrier(
      gpu::texture_barrier{.sync_before = gpu::barrier_sync::none,
                           .sync_after = gpu::barrier_sync::depth_stencil,
                           .access_before = gpu::barrier_access::no_access,
                           .access_after = gpu::barrier_access::depth_stencil_write,
                           .layout_before = gpu::barrier_layout::direct_queue_shader_resource,
                           .layout_after = gpu::barrier_layout::depth_stencil_write,
                           .resource = _shadow_map.get()});
   command_list.flush_barriers();

   for (int cascade_index = 0; cascade_index < cascade_count; ++cascade_index) {
      auto& shadow_camera = _sun_shadow_cascades[cascade_index];

      frustum shadow_frustum{shadow_camera.inv_view_projection_matrix()};

      cull_objects_shadow_cascade_avx2(shadow_frustum, meshes.bbox.min.x,
                                       meshes.bbox.min.y, meshes.bbox.min.z,
                                       meshes.bbox.max.x, meshes.bbox.max.y,
                                       meshes.bbox.max.z, meshes.pipeline_flags,
                                       _shadow_render_list);

      gpu::dsv_handle depth_stencil_view = _shadow_map_dsv[cascade_index].get();

      command_list.clear_depth_stencil_view(depth_stencil_view, {}, 1.0f, 0x0);

      command_list.set_graphics_root_signature(root_signatures.mesh_shadow.get());
      command_list.set_graphics_cbv(rs::mesh_shadow::camera_cbv,
                                    dynamic_buffer_allocator
                                       .allocate_and_copy(
                                          shadow_camera.view_projection_matrix())
                                       .gpu_address);

      command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

      command_list.rs_set_scissor_rects({0, 0, shadow_res, shadow_res});
      command_list.rs_set_viewports({0, 0, shadow_res, shadow_res});

      command_list.om_set_render_targets(depth_stencil_view);

      material_pipeline_flags pipeline_flags = material_pipeline_flags::none;

      command_list.set_pipeline_state(pipelines.mesh_shadow.get());

      for (const uint16 i : _shadow_render_list) {
         if (std::exchange(pipeline_flags, meshes.pipeline_flags[i]) !=
             meshes.pipeline_flags[i]) {

            if (are_flags_set(pipeline_flags, material_pipeline_flags::alpha_cutout)) {
               command_list.set_pipeline_state(
                  pipelines.mesh_shadow_alpha_cutout.get());
            }
            else {
               command_list.set_pipeline_state(pipelines.mesh_shadow.get());
            }
         }

         command_list.set_graphics_cbv(rs::mesh_shadow::object_cbv,
                                       meshes.gpu_constants[i]);

         if (are_flags_set(meshes.pipeline_flags[i],
                           material_pipeline_flags::alpha_cutout)) {
            command_list.set_graphics_cbv(rs::mesh_shadow::material_cbv,
                                          meshes.material_constant_buffer[i]);
         }

         command_list.ia_set_index_buffer(meshes.mesh[i].index_buffer_view);
         command_list.ia_set_vertex_buffers(0, meshes.mesh[i].vertex_buffer_views);

         command_list.draw_indexed_instanced(meshes.mesh[i].index_count, 1,
                                             meshes.mesh[i].start_index,
                                             meshes.mesh[i].start_vertex, 0);
      }
   }

   command_list.deferred_barrier(
      gpu::texture_barrier{.sync_before = gpu::barrier_sync::depth_stencil,
                           .sync_after = gpu::barrier_sync::pixel_shading,
                           .access_before = gpu::barrier_access::depth_stencil_write,
                           .access_after = gpu::barrier_access::shader_resource,
                           .layout_before = gpu::barrier_layout::depth_stencil_write,
                           .layout_after = gpu::barrier_layout::direct_queue_shader_resource,
                           .resource = _shadow_map.get()});
}

auto light_clusters::lights_constant_buffer_view() const noexcept -> gpu_virtual_address
{
   return _lights_constant_buffer_view;
}

void light_clusters::init_proxy_geometry(gpu::device& device,
                                         copy_command_list_pool& copy_command_list_pool)
{
   pooled_copy_command_list command_list = copy_command_list_pool.aquire_and_reset();

   _sphere_proxy_indices = {device.create_buffer({.size = sizeof(sphere_proxy_indices),
                                                  .debug_name =
                                                     "Light Proxy Indices"},
                                                 gpu::heap_type::default_),
                            device.direct_queue};
   _sphere_proxy_vertices = {device.create_buffer({.size = sizeof(sphere_proxy_vertices),
                                                   .debug_name =
                                                      "Light Proxy Vertices"},
                                                  gpu::heap_type::default_),
                             device.direct_queue};

   const auto upload_buffer_size =
      sizeof(sphere_proxy_indices) + sizeof(sphere_proxy_vertices);

   gpu::unique_resource_handle upload_buffer =
      {device.create_buffer({.size = upload_buffer_size,
                             .debug_name = "Light Proxy Upload Buffer"},
                            gpu::heap_type::upload),
       device.background_copy_queue};

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
}
