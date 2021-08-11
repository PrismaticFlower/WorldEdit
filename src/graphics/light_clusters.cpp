
#include "light_clusters.hpp"
#include "gpu/barrier_helpers.hpp"
#include "world/world_utilities.hpp"

#include <cmath>

#include <boost/algorithm/string.hpp>
#include <boost/container/static_vector.hpp>

namespace we::graphics {

namespace {

enum class light_type : uint32 { directional, point, spot };
enum class directional_region_type : uint32 { none, box, sphere, cylinder };

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

static_assert(sizeof(light_description) == 64);

struct global_light_constants {
   uint32 light_count = 0;
   std::array<uint32, 3> padding0;
   float3 sky_ambient_color;
   uint32 padding1;
   float3 ground_ambient_color;
   uint32 padding2;
   std::array<float4x4, 4> shadow_transforms;
   float2 shadow_resolution;
   float2 inv_shadow_resolution;
};

static_assert(sizeof(global_light_constants) == 320);

constexpr auto max_lights =
   ((D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16) - sizeof(global_light_constants)) /
   sizeof(light_description);
constexpr auto max_regional_lights = 512;

struct light_constants {
   global_light_constants global;

   std::array<light_description, max_lights> lights;
};

static_assert(sizeof(light_constants) == D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16);

struct light_region_description {
   float4x4 inverse_transform;
   float3 position;
   directional_region_type type;
   float3 size;
   uint32 padding;
};

static_assert(sizeof(light_region_description) == 96);

constexpr auto shadow_res = 2048;
constexpr auto cascade_count = 4;

}

light_clusters::light_clusters(gpu::device& gpu_device)
   : _gpu_device{&gpu_device}
{
   _lights_constant_buffer =
      gpu_device.create_buffer({.size = sizeof(light_constants)},
                               D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);
   _regional_lights_buffer =
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
      gpu::resource_view_desc{.resource = _lights_constant_buffer.resource()},

      gpu::resource_view_desc{.resource = _regional_lights_buffer.resource()},

      gpu::resource_view_desc{.resource = _shadow_map.resource()}};

   resource_view_descs[0].view_desc =
      gpu::constant_buffer_view{.buffer_location =
                                   _lights_constant_buffer.gpu_virtual_address(),
                                .size = _lights_constant_buffer.size()};

   resource_view_descs[1].view_desc = gpu::shader_resource_view_desc{
      .type_description = gpu::buffer_srv{.first_element = 0,
                                          .number_elements = max_regional_lights,
                                          .structure_byte_stride =

                                             sizeof(light_region_description)}};
   resource_view_descs[2].view_desc =
      gpu::shader_resource_view_desc{.format = DXGI_FORMAT_R32_FLOAT,
                                     .type_description = gpu::texture2d_array_srv{
                                        .array_size = cascade_count}};

   _resource_views = _gpu_device->create_resource_view_set(resource_view_descs);
}

void light_clusters::update_lights(const frustrum& view_frustrum,
                                   const world::world& world,
                                   gpu::command_list& command_list,
                                   gpu::dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   light_constants light_constants{
      .global = {.light_count = 0,
                 .sky_ambient_color = world.lighting_settings.ambient_sky_color,
                 .ground_ambient_color = world.lighting_settings.ambient_ground_color,
                 .shadow_transforms = _shadow_cascade_transforms,
                 .shadow_resolution = {shadow_res, shadow_res},
                 .inv_shadow_resolution = {1.0f / shadow_res, 1.0f / shadow_res}}};

   boost::container::static_vector<light_region_description, max_regional_lights> regional_lights_descriptions;

   uint32& light_count = light_constants.global.light_count;

   for (auto& light : world.lights) {
      if (light_count >= light_constants.lights.size()) break;

      switch (light.light_type) {
      case world::light_type::directional: {
         const float3 light_direction =
            glm::normalize(light.rotation * float3{0.0f, 0.0f, -1.0f});

         if (light.directional_region) {
            if (regional_lights_descriptions.size() ==
                regional_lights_descriptions.capacity()) {
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

            const uint32 region_description_index =
               static_cast<uint32>(regional_lights_descriptions.size());

            switch (region->shape) {
            case world::region_shape::box: {
               regional_lights_descriptions.push_back(
                  {.inverse_transform = inverse_region_transform,
                   .position = region->position,
                   .type = directional_region_type::box,
                   .size = region->size});

               light_constants.lights[light_count++] =
                  {.direction = light_direction,
                   .type = light_type::directional,
                   .color = light.color,
                   .region_type = directional_region_type::box,
                   .directional_region_index = region_description_index};
               break;
            }
            case world::region_shape::sphere: {
               regional_lights_descriptions.push_back(
                  {.inverse_transform = inverse_region_transform,
                   .position = region->position,
                   .type = directional_region_type::sphere,
                   .size = float3{glm::length(region->size)}});

               light_constants.lights[light_count++] =
                  {.direction = light_direction,
                   .type = light_type::directional,
                   .color = light.color,
                   .region_type = directional_region_type::sphere,
                   .directional_region_index = region_description_index};
               break;
            }
            case world::region_shape::cylinder: {
               const float radius =
                  glm::length(float2{region->size.x, region->size.z});

               regional_lights_descriptions.push_back(
                  {.inverse_transform = inverse_region_transform,
                   .position = region->position,
                   .type = directional_region_type::cylinder,
                   .size = float3{radius, region->size.y, radius}});

               light_constants.lights[light_count++] =
                  {.direction = light_direction,
                   .type = light_type::directional,
                   .color = light.color,
                   .region_type = directional_region_type::cylinder,
                   .directional_region_index = region_description_index};
               break;
            }
            }
         }
         else {
            light_constants.lights[light_count++] = {.direction = light_direction,
                                                     .type = light_type::directional,
                                                     .color = light.color,
                                                     .region_type =
                                                        directional_region_type::none};
         }

         break;
      }
      case world::light_type::point: {
         if (not intersects(view_frustrum, light.position, light.range)) {
            continue;
         }

         light_constants.lights[light_count++] = {.type = light_type::point,
                                                  .position = light.position,
                                                  .range = light.range,
                                                  .color = light.color};

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

         light_constants.lights[light_count++] =
            {.direction = light_direction,
             .type = light_type::spot,
             .position = light.position,
             .range = light.range,
             .color = light.color,
             .spot_outer_param = std::cos(light.outer_cone_angle / 2.0f),
             .spot_inner_param = 1.0f / (std::cos(light.inner_cone_angle / 2.0f) -
                                         std::cos(light.outer_cone_angle / 2.0f))};

         break;
      }
      }
   }

   {
   }

   auto upload_buffer = dynamic_buffer_allocator.allocate(sizeof(light_constants));

   std::memcpy(upload_buffer.cpu_address, &light_constants, sizeof(light_constants));

   command_list.copy_buffer_region(*_lights_constant_buffer.resource(), 0,
                                   *dynamic_buffer_allocator.view_resource(),
                                   upload_buffer.gpu_address -
                                      dynamic_buffer_allocator.gpu_base_address(),
                                   sizeof(light_constants));

   const auto regional_lights_descriptions_size =
      sizeof(light_region_description) * regional_lights_descriptions.size();

   upload_buffer =
      dynamic_buffer_allocator.allocate(regional_lights_descriptions_size);

   std::memcpy(upload_buffer.cpu_address, regional_lights_descriptions.data(),
               regional_lights_descriptions_size);

   command_list.copy_buffer_region(*_regional_lights_buffer.resource(), 0,
                                   *dynamic_buffer_allocator.view_resource(),
                                   upload_buffer.gpu_address -
                                      dynamic_buffer_allocator.gpu_base_address(),
                                   regional_lights_descriptions_size);

   const std::array barriers{
      gpu::transition_barrier(*_lights_constant_buffer.resource(),
                              D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
      gpu::transition_barrier(*_regional_lights_buffer.resource(),
                              D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)};

   command_list.deferred_resource_barrier(barriers);
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
   gpu::command_list& command_list,
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

      command_list.set_graphics_root_signature(
         *_gpu_device->root_signatures.depth_only_mesh);
      command_list.set_graphics_root_constant_buffer(1, shadow_camera.view_projection_matrix());
      command_list.set_pipeline_state(*_gpu_device->pipelines.shadow_mesh);

      command_list.ia_set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

      command_list.rs_set_scissor_rects({0, 0, shadow_res, shadow_res});
      command_list.rs_set_viewports({0, 0, shadow_res, shadow_res, 0.0f, 1.0f});

      command_list.om_set_render_targets({}, false, depth_stencil_view);

      for (std::size_t i = 0; i < meshes.size(); ++i) {
         if (not intersects_shadow_cascade(shadow_frustrum, meshes.bbox[i])) {
            continue;
         }

         if (are_flags_set(meshes.pipeline_flags[i],
                           gpu::material_pipeline_flags::transparent)) {
            continue;
         }

         command_list.set_graphics_root_constant_buffer_view(0, meshes.gpu_constants[i]);

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
}
