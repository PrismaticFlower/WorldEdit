
#include "light_clusters.hpp"
#include "gpu/barrier_helpers.hpp"
#include "math/align.hpp"
#include "world/world_utilities.hpp"

#include <cmath>

#include <boost/algorithm/string.hpp>
#include <boost/container/static_vector.hpp>

#include <d3dx12.h>

namespace we::graphics {

namespace {

constexpr auto shadow_res = 2048;
constexpr auto cascade_count = 4;
constexpr auto light_tile_size = 8;
constexpr auto light_description_size = 64;
constexpr auto max_lights = 256;
constexpr auto tile_light_word_bits = 32;
constexpr auto tile_light_words = max_lights / tile_light_word_bits;
constexpr auto max_regional_lights = 256;

enum class light_type : uint32 { directional, point, spot };
enum class directional_region_type : uint32 { none, box, sphere, cylinder };

struct alignas(256) light_tile_clear_inputs {
   std::array<uint32, 2> tile_counts;
   std::array<uint32, 2> padding0{};

   std::array<uint32, 8> clear_value;
};

static_assert(sizeof(light_tile_clear_inputs) == 256);

struct alignas(256) tiling_inputs {
   std::array<uint32, 2> tile_counts;
   std::array<uint32, 2> padding0{};

   float4x4 view_projection_matrix;
};

static_assert(sizeof(tiling_inputs) == 256);

struct alignas(256) light_constants {
   uint32 light_tiles_width = 0;
   std::array<uint32, 3> padding0;
   float3 sky_ambient_color;
   uint32 padding1;
   float3 ground_ambient_color;
   uint32 padding2;
   std::array<float4x4, 4> shadow_transforms;
   float2 shadow_resolution;
   float2 inv_shadow_resolution;
};

static_assert(sizeof(light_constants) == 512);

struct light_description {
   float3 direction;
   light_type type;
   float3 position;
   float range;
   float3 color;
   float spot_outer_param;
   float spot_inner_param;
   directional_region_type region_type;
   uint32 directional_region_index;
   uint32 padding;
};

static_assert(sizeof(light_description) == light_description_size);

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
   float4x4 light_transform = {radius, 0.0f,   0.0f,   0.0f, //
                               0.0f,   radius, 0.0f,   0.0f, //
                               0.0f,   0.0f,   radius, 0.0f, //
                               0.0f,   0.0f,   0.0f,   1.0f};
   light_transform[3] = {light.position, 1.0f};
   light_transform = glm::transpose(
      light_transform); // transpose so we have good alignment for a float3x4 in HLSL

   return {light_transform[0], light_transform[1], light_transform[2]};
}

auto make_sphere_light_proxy_transform(const world::region& light, const float radius)
   -> std::array<float4, 3>
{
   float4x4 light_transform = {radius, 0.0f,   0.0f,   0.0f, //
                               0.0f,   radius, 0.0f,   0.0f, //
                               0.0f,   0.0f,   radius, 0.0f, //
                               0.0f,   0.0f,   0.0f,   1.0f};
   light_transform[3] = {light.position, 1.0f};
   light_transform = glm::transpose(
      light_transform); // transpose so we have good alignment for a float3x4 in HLSL

   return {light_transform[0], light_transform[1], light_transform[2]};
}

struct upload_data {
   upload_data(gpu::dynamic_buffer_allocator& dynamic_buffer_allocator)
   {
      {
         auto allocation = dynamic_buffer_allocator.allocate(
            sizeof(std::array<light_description, max_lights>));

         lights = reinterpret_cast<light_description*>(allocation.cpu_address);
         lights_gpu = allocation.gpu_address;
      }

      {
         auto allocation = dynamic_buffer_allocator.allocate(
            sizeof(std::array<light_region_description, max_regional_lights>));

         regional_lights_descriptions =
            reinterpret_cast<light_region_description*>(allocation.cpu_address);
         regional_lights_descriptions_gpu = allocation.gpu_address;
      }

      {
         auto allocation = dynamic_buffer_allocator.allocate(
            sizeof(std::array<light_proxy_instance, max_lights>));

         sphere_light_proxies =
            reinterpret_cast<light_proxy_instance*>(allocation.cpu_address);
         sphere_light_proxies_gpu = allocation.gpu_address;
      }
   }

   light_description* lights;
   light_region_description* regional_lights_descriptions;
   light_proxy_instance* sphere_light_proxies;

   gpu::virtual_address lights_gpu;
   gpu::virtual_address regional_lights_descriptions_gpu;
   gpu::virtual_address sphere_light_proxies_gpu;
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

}

light_clusters::light_clusters(gpu::device& gpu_device, uint32 render_width,
                               uint32 render_height)
   : _gpu_device{&gpu_device}
{
   _tiling_inputs = gpu_device.create_buffer({.size = sizeof(tiling_inputs)},
                                             D3D12_HEAP_TYPE_DEFAULT,
                                             D3D12_RESOURCE_STATE_COMMON);

   _lights_constants = gpu_device.create_buffer({.size = sizeof(light_constants)},
                                                D3D12_HEAP_TYPE_DEFAULT,
                                                D3D12_RESOURCE_STATE_COMMON);

   _lights_list =
      gpu_device.create_buffer({.size = sizeof(light_description) * max_lights},
                               D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);

   _lights_region_list =
      gpu_device.create_buffer({.size = sizeof(light_region_description) *
                                        max_regional_lights},
                               D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);

   _shadow_map = gpu_device.create_texture(
      {.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
       .flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
       .format = DXGI_FORMAT_D32_FLOAT,
       .width = shadow_res,
       .height = shadow_res,
       .array_size = cascade_count,
       .optimized_clear_value =
          D3D12_CLEAR_VALUE{.Format = DXGI_FORMAT_D32_FLOAT,
                            .DepthStencil = {.Depth = 1.0f, .Stencil = 0x0}}},
      D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

   _shadow_map_dsv =
      gpu_device.allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, cascade_count);

   for (UINT i = 0; i < cascade_count; ++i) {
      const D3D12_DEPTH_STENCIL_VIEW_DESC desc{.Format = _shadow_map.format(),
                                               .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY,
                                               .Texture2DArray = {.FirstArraySlice = i,
                                                                  .ArraySize = 1}};

      _gpu_device->device_d3d->CreateDepthStencilView(_shadow_map.view_resource(),
                                                      &desc, _shadow_map_dsv[i].cpu);
   }

   update_render_resolution(render_width, render_height, false);
   update_descriptors();

   init_proxy_geometry(gpu_device);
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

   _lights_tiles =
      _gpu_device->create_buffer({.size = sizeof(uint32) * 8 * tiles_count,
                                  .flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS},
                                 D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);

   _tiles_count = tiles_count;
   _tiles_width = tiles_width;
   _tiles_height = tiles_height;
   _render_width = static_cast<float>(width);
   _render_height = static_cast<float>(height);

   if (recreate_descriptors) update_descriptors();
}

void light_clusters::update_descriptors()
{
   // _resource_views = _gpu_device->create_resource_view_set(std::array{
   // gpu::resource_view_desc{
   //    .resource = *_lights_constant_buffer.resource(),
   //    .view_desc =
   //       gpu::constant_buffer_view{.buffer_location =
   //                                    _lights_constant_buffer.gpu_virtual_address(),
   //                                 .size = _lights_constant_buffer.size()}},
   //
   // gpu::resource_view_desc{.resource = *_regional_lights_buffer.resource(),
   //                         .view_desc =
   //                            gpu::shader_resource_view_desc{
   //                               .type_description =
   //                                  gpu::buffer_srv{.first_element = 0,
   //                                                  .number_elements = max_regional_lights,
   //                                                  .structure_byte_stride = sizeof(
   //                                                     light_region_description)}}},
   //
   // gpu::resource_view_desc{.resource = *_shadow_map.resource(),
   //                         .view_desc = gpu::shader_resource_view_desc{
   //                            .format = DXGI_FORMAT_R32_FLOAT,
   //                            .type_description = gpu::texture2d_srv{}}}});

   // Above ICEs, convoluted workaround below

   std::array resource_view_descs{
      gpu::resource_view_desc{.resource = _lights_constants.resource()},

      gpu::resource_view_desc{.resource = _lights_tiles.resource()},

      gpu::resource_view_desc{.resource = _lights_list.resource()},

      gpu::resource_view_desc{.resource = _lights_region_list.resource()},

      gpu::resource_view_desc{.resource = _shadow_map.resource()}};

   resource_view_descs[0].view_desc =
      gpu::constant_buffer_view{.buffer_location =
                                   _lights_constants.gpu_virtual_address(),
                                .size = _lights_constants.size()};

   resource_view_descs[1].view_desc = gpu::shader_resource_view_desc{
      .type_description =
         gpu::buffer_srv{.first_element = 0,
                         .number_elements = _tiles_count,
                         .structure_byte_stride = sizeof(uint32) * 8}};

   resource_view_descs[2].view_desc =
      gpu::constant_buffer_view{.buffer_location = _lights_list.gpu_virtual_address(),
                                .size = _lights_list.size()};

   resource_view_descs[3].view_desc = gpu::shader_resource_view_desc{
      .type_description = gpu::buffer_srv{.first_element = 0,
                                          .number_elements = max_regional_lights,
                                          .structure_byte_stride =
                                             sizeof(light_region_description)}};

   resource_view_descs[4].view_desc =
      gpu::shader_resource_view_desc{.format = DXGI_FORMAT_R32_FLOAT,
                                     .type_description = gpu::texture2d_array_srv{
                                        .array_size = cascade_count}};

   _resource_views = _gpu_device->create_resource_view_set(resource_view_descs);

   std::array tile_culling_resource_views_descs{
      gpu::resource_view_desc{.resource = _tiling_inputs.resource()},

      gpu::resource_view_desc{.resource = _lights_tiles.resource()}};

   tile_culling_resource_views_descs[0].view_desc =
      gpu::constant_buffer_view{.buffer_location = _tiling_inputs.gpu_virtual_address(),
                                .size = _tiling_inputs.size()};

   tile_culling_resource_views_descs[1].view_desc = gpu::unordered_access_view_desc{
      .type_description =
         gpu::buffer_uav{.first_element = 0,
                         .number_elements = _tiles_count,
                         .structure_byte_stride = sizeof(uint32) * 8}};

   _tiling_resource_views =
      _gpu_device->create_resource_view_set(tile_culling_resource_views_descs);
}

void light_clusters::update_lights(
   const camera& view_camera, const frustrum& view_frustrum,
   const world::world& world, root_signature_library& root_signatures,
   pipeline_library& pipelines, gpu::graphics_command_list& command_list,
   gpu::dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   upload_data upload_data{dynamic_buffer_allocator};

   std::array<uint32, tile_light_words> tiles_start_value{};
   uint32 light_count = 0;
   uint32 regional_lights_count = 0;

   // frustrum cull lights
   for (auto& light : world.lights) {
      if (light_count >= max_lights) break;

      const uint32 light_index = light_count;

      switch (light.light_type) {
      case world::light_type::directional: {
         const float3 light_direction =
            glm::normalize(light.rotation * float3{0.0f, 0.0f, -1.0f});

         if (light.directional_region) {
            if (regional_lights_count == max_regional_lights) {
               break;
            }

            const world::region* const region =
               world::find_region_by_description(world, *light.directional_region);

            if (not region) break;

            const quaternion region_rotation_inverse =
               glm::conjugate(region->rotation);
            float4x4 inverse_region_transform{region_rotation_inverse};
            inverse_region_transform[3] = {region_rotation_inverse * -region->position,
                                           1.0f};

            inverse_region_transform = glm::transpose(inverse_region_transform);

            const uint32 region_description_index = regional_lights_count;

            switch (region->shape) {
            case world::region_shape::box: {
               upload_to({.inverse_transform = inverse_region_transform,
                          .position = region->position,
                          .type = directional_region_type::box,
                          .size = region->size},
                         upload_data.regional_lights_descriptions +
                            region_description_index);

               upload_to({.direction = light_direction,
                          .type = light_type::directional,
                          .color = light.color,
                          .region_type = directional_region_type::box,
                          .directional_region_index = region_description_index},
                         upload_data.lights + light_index);

               upload_to({.transform = make_sphere_light_proxy_transform(
                             *region, std::max({region->size.x, region->size.y,
                                                region->size.z})),

                          .light_index = light_index},
                         upload_data.sphere_light_proxies + light_index);

               light_count += 1;
               regional_lights_count += 1;

               break;
            }
            case world::region_shape::sphere: {
               upload_to({.inverse_transform = inverse_region_transform,
                          .position = region->position,
                          .type = directional_region_type::sphere,
                          .size = float3{glm::length(region->size)}},
                         upload_data.regional_lights_descriptions +
                            region_description_index);

               upload_to({.direction = light_direction,
                          .type = light_type::directional,
                          .color = light.color,
                          .region_type = directional_region_type::sphere,
                          .directional_region_index = region_description_index},
                         upload_data.lights + light_index);

               upload_to({.transform = make_sphere_light_proxy_transform(
                             *region, glm::length(region->size)),

                          .light_index = light_index},
                         upload_data.sphere_light_proxies + light_index);

               light_count += 1;
               regional_lights_count += 1;

               break;
            }
            case world::region_shape::cylinder: {
               const float radius =
                  glm::length(float2{region->size.x, region->size.z});

               upload_to({.inverse_transform = inverse_region_transform,
                          .position = region->position,
                          .type = directional_region_type::cylinder,
                          .size = float3{radius, region->size.y, radius}},
                         upload_data.regional_lights_descriptions +
                            region_description_index);

               upload_to({.direction = light_direction,
                          .type = light_type::directional,
                          .color = light.color,
                          .region_type = directional_region_type::cylinder,
                          .directional_region_index = region_description_index},
                         upload_data.lights + light_index);

               upload_to({.transform = make_sphere_light_proxy_transform(
                             *region, std::max(radius, region->size.y)),

                          .light_index = light_index},
                         upload_data.sphere_light_proxies + light_index);

               light_count += 1;
               regional_lights_count += 1;

               break;
            }
            }
         }
         else {
            upload_to({.direction = light_direction,
                       .type = light_type::directional,
                       .color = light.color,
                       .region_type = directional_region_type::none},
                      upload_data.lights + light_index);

            light_count += 1;

            // Set the directional light as part of the tile clear pass.
            tiles_start_value[light_index / tile_light_word_bits] |=
               (1u << (light_index % tile_light_word_bits));
         }

         break;
      }
      case world::light_type::point: {
         if (not intersects(view_frustrum, light.position, light.range)) {
            continue;
         }

         upload_to({.type = light_type::point,
                    .position = light.position,
                    .range = light.range,
                    .color = light.color},
                   upload_data.lights + light_index);

         upload_to({.transform = make_sphere_light_proxy_transform(light, light.range),

                    .light_index = light_index},
                   upload_data.sphere_light_proxies + light_index);

         light_count += 1;

         break;
      }
      case world::light_type::spot: {
         const float3 light_direction =
            glm::normalize(light.rotation * float3{0.0f, 0.0f, -1.0f});

         const float half_range = light.range / 2.0f;
         const float cone_radius = half_range * std::tan(light.outer_cone_angle);

         const float light_bounds_radius = std::max(cone_radius, half_range);
         const float3 light_centre =
            light.position - (light_direction * (half_range));

         // TODO: Cone-frustrum intersection (but how much of a problem is it?)
         if (not intersects(view_frustrum, light_centre, light_bounds_radius)) {
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
                   upload_data.lights + light_index);

         upload_to({.transform = make_sphere_light_proxy_transform(light, light.range), // TODO: Cone light proxies.

                    .light_index = light_index},
                   upload_data.sphere_light_proxies + light_index);

         light_count += 1;

         break;
      }
      }
   }

   // copy tile culling inputs
   {
      tiling_inputs tiling_inputs{.tile_counts = {_tiles_width, _tiles_height},
                                  .view_projection_matrix =
                                     view_camera.view_projection_matrix()};

      auto upload_buffer = dynamic_buffer_allocator.allocate(sizeof(tiling_inputs));

      std::memcpy(upload_buffer.cpu_address, &tiling_inputs, sizeof(tiling_inputs));

      command_list.copy_buffer_region(*_tiling_inputs.resource(), 0,
                                      *dynamic_buffer_allocator.view_resource(),
                                      upload_buffer.gpu_address -
                                         dynamic_buffer_allocator.gpu_base_address(),
                                      sizeof(tiling_inputs));
   }

   // copy global light constants
   {
      light_constants light_constants{
         .light_tiles_width = _tiles_width,
         .sky_ambient_color = world.lighting_settings.ambient_sky_color,
         .ground_ambient_color = world.lighting_settings.ambient_ground_color,
         .shadow_transforms = _shadow_cascade_transforms,
         .shadow_resolution = {shadow_res, shadow_res},
         .inv_shadow_resolution = {1.0f / shadow_res, 1.0f / shadow_res}};

      auto upload_buffer = dynamic_buffer_allocator.allocate(sizeof(light_constants));

      std::memcpy(upload_buffer.cpu_address, &light_constants, sizeof(light_constants));

      command_list.copy_buffer_region(*_lights_constants.resource(), 0,
                                      *dynamic_buffer_allocator.view_resource(),
                                      upload_buffer.gpu_address -
                                         dynamic_buffer_allocator.gpu_base_address(),
                                      sizeof(light_constants));
   }

   command_list.copy_buffer_region(*_lights_list.resource(), 0,
                                   *dynamic_buffer_allocator.view_resource(),
                                   upload_data.lights_gpu -
                                      dynamic_buffer_allocator.gpu_base_address(),
                                   sizeof(light_description) * light_count);

   command_list.copy_buffer_region(*_lights_region_list.resource(), 0,
                                   *dynamic_buffer_allocator.view_resource(),
                                   upload_data.regional_lights_descriptions_gpu -
                                      dynamic_buffer_allocator.gpu_base_address(),
                                   sizeof(light_region_description) *
                                      regional_lights_count);

   command_list.deferred_resource_barrier(std::array{
      gpu::transition_barrier(*_tiling_inputs.resource(), D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
      gpu::transition_barrier(*_lights_constants.resource(), D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
      gpu::transition_barrier(*_lights_list.resource(), D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
      gpu::transition_barrier(*_lights_region_list.resource(), D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
      gpu::transition_barrier(*_lights_tiles.resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                              D3D12_RESOURCE_STATE_UNORDERED_ACCESS)});

   command_list.flush_deferred_resource_barriers();

   // clear light tiles
   {
      command_list.set_compute_root_signature(*root_signatures.tile_lights_clear);
      command_list.set_compute_root_constant_buffer(
         rs::tile_lights_clear::input_cbv,
         light_tile_clear_inputs{.tile_counts = {_tiles_width, _tiles_height},
                                 .clear_value = tiles_start_value});
      command_list.set_compute_root_unordered_access_view(rs::tile_lights_clear::light_tiles_uav,
                                                          _lights_tiles.gpu_virtual_address());

      command_list.set_pipeline_state(*pipelines.tile_lights_clear);

      command_list.dispatch(math::align_up(_tiles_width, 32) / 32,
                            math::align_up(_tiles_height, 32) / 32, 1);
   }

   command_list.resource_barrier(gpu::uav_barrier(*_lights_tiles.resource()));

   command_list.set_graphics_root_signature(*root_signatures.tile_lights);
   command_list.set_graphics_root_descriptor_table(rs::tile_lights::descriptor_table,
                                                   _tiling_resource_views.descriptors());

   command_list.ia_set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   command_list.rs_set_scissor_rects(
      {0, 0, to_int32(_tiles_width), to_int32(_tiles_height)});
   command_list.rs_set_viewports(
      {0.0f, 0.0f, to_float(_tiles_width), to_float(_tiles_height), 0.0f, 1.0f});

   command_list.om_set_render_targets({}, false, std::nullopt);

   // draw sphere lights
   {
      command_list.set_graphics_root_shader_resource_view(rs::tile_lights::instance_data_srv,
                                                          upload_data.sphere_light_proxies_gpu);

      command_list.set_pipeline_state(*pipelines.tile_lights_spheres);

      command_list.ia_set_index_buffer(
         {.BufferLocation = _sphere_proxy_indices.gpu_virtual_address(),
          .SizeInBytes = sizeof(sphere_proxy_indices),
          .Format = DXGI_FORMAT_R16_UINT});
      command_list.ia_set_vertex_buffers(0, {.BufferLocation =
                                                _sphere_proxy_vertices.gpu_virtual_address(),
                                             .SizeInBytes = sizeof(sphere_proxy_vertices),
                                             .StrideInBytes = sizeof(float3)});

      command_list.draw_indexed_instanced(to_uint32(sphere_proxy_indices.size()),
                                          light_count, 0, 0, 0);
   }

   command_list.deferred_resource_barrier(std::array{
      gpu::transition_barrier(*_lights_tiles.resource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                              D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE)});
}

namespace {
class shadow_camera : public camera {
public:
   shadow_camera() noexcept
   {
      update();
   }

   void look_at(const float3 eye, const float3 at, const float3 up) noexcept
   {
      _view_matrix = glm::lookAtLH(eye, at, up);
      _world_matrix = glm::inverse(_view_matrix);

      update();
   }

   void set_projection(const float min_x, const float min_y, const float max_x,
                       const float max_y, const float min_z, const float max_z)
   {
      _projection_matrix = {1.0f, 0.0f, 0.0f, 0.0f, //
                            0.0f, 1.0f, 0.0f, 0.0f, //
                            0.0f, 0.0f, 1.0f, 0.0f, //
                            0.0f, 0.0f, 0.0f, 1.0f};

      _near_clip = min_z;
      _far_clip = max_z;

      const auto inv_x_range = 1.0f / (min_x - max_x);
      const auto inv_y_range = 1.0f / (max_y - min_y);
      const auto inv_z_range = 1.0f / (max_z - min_z);

      _projection_matrix[0][0] = inv_x_range + inv_x_range;
      _projection_matrix[1][1] = inv_y_range + inv_y_range;
      _projection_matrix[2][2] = inv_z_range;

      _projection_matrix[3][0] = -(max_x + min_x) * inv_x_range;
      _projection_matrix[3][1] = -(max_y + min_y) * inv_y_range;
      _projection_matrix[3][2] = -min_z * inv_z_range;

      update();
   }

   void set_stabilization(const float2 stabilization)
   {
      _stabilization = stabilization;

      update();
   }

   auto texture_matrix() const noexcept -> const float4x4&
   {
      return _texture_matrix;
   }

private:
   void update() noexcept
   {
      _view_projection_matrix = _projection_matrix * _view_matrix;

      _view_projection_matrix[3].x += _stabilization.x;
      _view_projection_matrix[3].y += _stabilization.y;

      _inv_view_projection_matrix = glm::inverse(double4x4{_view_projection_matrix});

      constexpr float4x4 bias_matrix{0.5f, 0.0f,  0.0f, 0.0f, //
                                     0.0f, -0.5f, 0.0f, 0.0f, //
                                     0.0f, 0.0f,  1.0f, 0.0f, //
                                     0.5f, 0.5f,  0.0f, 1.0f};

      _texture_matrix = bias_matrix * _view_projection_matrix;
   }

   float2 _stabilization = {0.0f, 0.0f};
   float4x4 _texture_matrix;
};

auto sun_rotation(const world::world& world) -> quaternion
{
   for (auto& light : world.lights) {
      if (boost::iequals(light.name, world.lighting_settings.global_lights[0])) {
         return light.rotation;
      }
   }

   return quaternion{1.0f, 0.0f, 0.0f, 0.0f};
}

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
                                const frustrum& view_frustrum) -> shadow_camera
{
   auto view_frustrum_corners = view_frustrum.corners;

   for (int i = 0; i < 4; ++i) {
      float3 corner_ray = view_frustrum_corners[i + 4] - view_frustrum_corners[i];

      view_frustrum_corners[i + 4] =
         view_frustrum_corners[i] + (corner_ray * far_split);
      view_frustrum_corners[i] += (corner_ray * near_split);
   }

   float3 view_frustrum_center{0.0f, 0.0f, 0.0f};

   for (const auto& corner : view_frustrum_corners) {
      view_frustrum_center += corner;
   }

   view_frustrum_center /= 8.0f;

   float radius = std::numeric_limits<float>::lowest();

   for (const auto& corner : view_frustrum_corners) {
      radius = glm::max(glm::distance(corner, view_frustrum_center), radius);
   }

   float3 bounds_max{radius};
   float3 bounds_min{-radius};
   float3 casecase_extents{bounds_max - bounds_min};

   float3 shadow_camera_position =
      view_frustrum_center + light_direction * -bounds_min.z;

   shadow_camera shadow_camera;
   shadow_camera.set_projection(bounds_min.x, bounds_min.y, bounds_max.x,
                                bounds_max.y, 0.0f, casecase_extents.z);
   shadow_camera.look_at(shadow_camera_position, view_frustrum_center,
                         float3{0.0f, 1.0f, 0.0f});

   auto shadow_view_projection = shadow_camera.view_projection_matrix();

   float4 shadow_origin = shadow_view_projection * float4{0.0f, 0.0f, 0.0f, 1.0f};
   shadow_origin /= shadow_origin.w;
   shadow_origin *= float{shadow_res} / 2.0f;

   float2 rounded_origin = glm::round(shadow_origin);
   float2 rounded_offset = rounded_origin - float2{shadow_origin};
   rounded_offset *= 2.0f / float{shadow_res};

   shadow_camera.set_stabilization(rounded_offset);

   return shadow_camera;
}

auto make_shadow_cascades(const quaternion light_rotation, const camera& camera,
                          const frustrum& view_frustrum)
   -> std::array<shadow_camera, cascade_count>
{
   const float3 light_direction =
      glm::normalize(light_rotation * float3{0.0f, 0.0f, -1.0f});

   const std::array cascade_splits = make_shadow_cascade_splits(camera);

   std::array<shadow_camera, cascade_count> cameras;

   for (int i = 0; i < cascade_count; ++i) {
      cameras[i] = make_cascade_shadow_camera(light_direction, cascade_splits[i],
                                              cascade_splits[i + 1], view_frustrum);
   }

   return cameras;
}
}

void light_clusters::TEMP_render_shadow_maps(
   const camera& view_camera, const frustrum& view_frustrum,
   const world_mesh_list& meshes, const world::world& world,
   root_signature_library& root_signatures, pipeline_library& pipelines,
   gpu::graphics_command_list& command_list,
   [[maybe_unused]] gpu::dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   command_list.deferred_resource_barrier(
      gpu::transition_barrier(*_shadow_map.resource(),
                              D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                              D3D12_RESOURCE_STATE_DEPTH_WRITE));
   command_list.flush_deferred_resource_barriers();

   auto shadow_cascade_cameras =
      make_shadow_cascades(sun_rotation(world), view_camera, view_frustrum);

   for (int cascade_index = 0; cascade_index < cascade_count; ++cascade_index) {
      auto& shadow_camera = shadow_cascade_cameras[cascade_index];

      _shadow_cascade_transforms[cascade_index] = shadow_camera.texture_matrix();

      frustrum shadow_frustrum{shadow_camera};

      auto depth_stencil_view = _shadow_map_dsv[cascade_index].cpu;

      command_list.clear_depth_stencil_view(depth_stencil_view,
                                            D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0x0);

      command_list.set_graphics_root_signature(*root_signatures.mesh_shadow);
      command_list.set_graphics_root_constant_buffer(rs::mesh_shadow::camera_cbv,
                                                     shadow_camera.view_projection_matrix());
      command_list.set_pipeline_state(*pipelines.mesh_shadow);

      command_list.ia_set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

      command_list.rs_set_scissor_rects({0, 0, shadow_res, shadow_res});
      command_list.rs_set_viewports({0, 0, shadow_res, shadow_res, 0.0f, 1.0f});

      command_list.om_set_render_targets({}, false, depth_stencil_view);

      for (std::size_t i = 0; i < meshes.size(); ++i) {
         if (not intersects_shadow_cascade(shadow_frustrum, meshes.bbox[i])) {
            continue;
         }

         if (are_flags_set(meshes.pipeline_flags[i],
                           material_pipeline_flags::transparent)) {
            continue;
         }

         command_list.set_graphics_root_constant_buffer_view(rs::mesh_shadow::object_cbv,
                                                             meshes.gpu_constants[i]);

         command_list.ia_set_index_buffer(meshes.mesh[i].index_buffer_view);
         command_list.ia_set_vertex_buffers(0, meshes.mesh[i].vertex_buffer_views[0]);

         command_list.draw_indexed_instanced(meshes.mesh[i].index_count, 1,
                                             meshes.mesh[i].start_index,
                                             meshes.mesh[i].start_vertex, 0);
      }
   }

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(*_shadow_map.resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE,
                              D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

auto light_clusters::light_descriptors() const noexcept -> gpu::descriptor_range
{
   return _resource_views.descriptors();
}

void light_clusters::init_proxy_geometry(gpu::device& device)
{
   _sphere_proxy_indices =
      device.create_buffer({.size = sizeof(sphere_proxy_indices)},
                           D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);
   _sphere_proxy_vertices =
      device.create_buffer({.size = sizeof(sphere_proxy_vertices)},
                           D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);

   auto copy_context = device.copy_manager.aquire_context();

   const auto upload_buffer_size =
      sizeof(sphere_proxy_indices) + sizeof(sphere_proxy_vertices);

   ID3D12Resource& upload_buffer = copy_context.create_upload_resource(
      CD3DX12_RESOURCE_DESC::Buffer(upload_buffer_size));

   std::byte* const upload_buffer_ptr = [&] {
      const D3D12_RANGE read_range{};
      void* map_void_ptr = nullptr;

      throw_if_failed(upload_buffer.Map(0, &read_range, &map_void_ptr));

      return static_cast<std::byte*>(map_void_ptr);
   }();

   const auto sphere_proxy_indices_offset = 0;
   const auto sphere_proxy_vertices_offset = sizeof(sphere_proxy_indices);

   std::memcpy(upload_buffer_ptr + sphere_proxy_indices_offset,
               &sphere_proxy_indices, sizeof(sphere_proxy_indices));

   std::memcpy(upload_buffer_ptr + sphere_proxy_vertices_offset,
               &sphere_proxy_vertices, sizeof(sphere_proxy_vertices));

   const D3D12_RANGE write_range{0, upload_buffer_size};
   upload_buffer.Unmap(0, &write_range);

   copy_context.command_list().copy_buffer_region(*_sphere_proxy_indices.view_resource(),
                                                  0, upload_buffer,
                                                  sphere_proxy_indices_offset,
                                                  sizeof(sphere_proxy_indices));

   copy_context.command_list().copy_buffer_region(*_sphere_proxy_vertices.view_resource(),
                                                  0, upload_buffer,
                                                  sphere_proxy_vertices_offset,
                                                  sizeof(sphere_proxy_vertices));

   device.copy_manager.close_and_execute(copy_context);
}
}
