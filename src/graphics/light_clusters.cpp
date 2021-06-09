
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

constexpr auto max_lights = ((D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16) - 86) /
                            sizeof(light_description);
constexpr auto max_regional_lights = 512;

struct light_constants {
   uint32 light_count = 0;
   std::array<uint32, 3> padding0;
   float3 sky_ambient_color;
   uint32 padding1;
   float3 ground_ambient_color;
   uint32 padding2;
   float4x4 shadow_transform;

   std::array<light_description, max_lights> lights;

   std::array<float4, 1> padding3;
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
       .width = 2048,
       .height = 2048,
       .optimized_clear_value =
          D3D12_CLEAR_VALUE{.Format = DXGI_FORMAT_D32_FLOAT,
                            .DepthStencil = {.Depth = 1.0f, .Stencil = 0x0}}},
      D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

   _shadow_map_dsv =
      gpu_device.allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

   _gpu_device->device_d3d->CreateDepthStencilView(_shadow_map.view_resource(),
                                                   nullptr, _shadow_map_dsv[0].cpu);

   // _resource_views = _gpu_device->create_resource_view_set(std::array{
   // gpu::resource_view_desc{
   //    .resource = *_lights_constant_buffer.resource(),
   //    .view_desc =
   //       gpu::constant_buffer_view{.buffer_location =
   //                                    _lights_constant_buffer.resource()->GetGPUVirtualAddress(),
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
                                   _lights_constant_buffer.resource()->GetGPUVirtualAddress(),
                                .size = _lights_constant_buffer.size()};

   resource_view_descs[1].view_desc = gpu::shader_resource_view_desc{
      .type_description = gpu::buffer_srv{.first_element = 0,
                                          .number_elements = max_regional_lights,
                                          .structure_byte_stride =

                                             sizeof(light_region_description)}};
   resource_view_descs[2].view_desc =
      gpu::shader_resource_view_desc{.format = DXGI_FORMAT_R32_FLOAT,
                                     .type_description = gpu::texture2d_srv{}};

   _resource_views = _gpu_device->create_resource_view_set(resource_view_descs);
}

void light_clusters::update_lights(const frustrum& view_frustrum,
                                   const world::world& world,
                                   gpu::command_list& command_list,
                                   gpu::dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   light_constants light_constants{.light_count = 0,
                                   .sky_ambient_color =
                                      world.lighting_settings.ambient_sky_color,
                                   .ground_ambient_color =
                                      world.lighting_settings.ambient_ground_color,
                                   .shadow_transform = _shadow_transform};

   boost::container::static_vector<light_region_description, max_regional_lights> regional_lights_descriptions;

   for (auto& light : world.lights) {
      if (light_constants.light_count >= light_constants.lights.size()) break;

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

               light_constants.lights[light_constants.light_count++] =
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

               light_constants.lights[light_constants.light_count++] =
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

               light_constants.lights[light_constants.light_count++] =
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
            light_constants.lights[light_constants.light_count++] =
               {.direction = light_direction,
                .type = light_type::directional,
                .color = light.color,
                .region_type = directional_region_type::none};
         }

         break;
      }
      case world::light_type::point: {
         if (not intersects(view_frustrum, light.position, light.range)) {
            continue;
         }

         light_constants
            .lights[light_constants.light_count++] = {.type = light_type::point,
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

         light_constants.lights[light_constants.light_count++] =
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

auto sun_direction(const world::world& world) -> float3
{
   for (auto& light : world.lights) {
      if (boost::iequals(light.name, world.lighting_settings.global_lights[0])) {
         return glm::normalize(light.rotation * float3{0.0f, 0.0f, -1.0f});
      }
   }

   return float3{1.0f, 0.0f, 0.0f};
}

auto make_shadow_camera(const float3 light_direction, const camera& view_camera,
                        const frustrum& view_frustrum) -> shadow_orthographic_camera
{
   shadow_orthographic_camera cam;

   float3 frustrum_centre{0.0f};

   for (auto& corner : view_frustrum.corners) {
      frustrum_centre += corner;
   }

   frustrum_centre /= 8.0f;

   (void)view_camera;
   // float3 min{std::numeric_limits<float>::max()};
   // float3 max{std::numeric_limits<float>::lowest()};
   //
   // {
   //    const float3 light_camera_position = frustrum_centre;
   //    const float3 look_at = frustrum_centre - light_direction;
   //    const float4x4 light_view =
   //       glm::lookAtLH(light_camera_position, look_at, view_camera.right());
   //
   //    for (auto& view_frustrum_corner : view_frustrum.corners) {
   //       const float3 corner = light_view * float4{view_frustrum_corner, 1.0f};
   //       min = glm::min(min, corner);
   //       max = glm::max(max, corner);
   //    }
   // }

   // const float3 light_camera_position = frustrum_centre + light_direction *
   // -min.z; const float3 up = view_camera.right();

   // cam.bounds({min.x, min.y, 0.0f}, {max.x, max.y, max.z - min.z});
   // cam.look_at(light_camera_position, frustrum_centre, view_camera.right());
   cam.bounds({-256.f, -256.f, -256.f}, {256.f, 256.f, 256.f});
   cam.look_at({0.0f, 0.0f, 0.0f}, -light_direction, {0.0f, 1.0f, 0.0f});

   return cam;
}

}

void light_clusters::TEMP_render_shadow_maps(
   const camera& view_camera, const frustrum& view_frustrum,
   const world_mesh_list& meshes, const world::world& world,
   gpu::command_list& command_list,
   gpu::dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   const float3 light_direction = sun_direction(world);

   shadow_orthographic_camera shadow_camera =
      make_shadow_camera(light_direction, view_camera, view_frustrum);
   frustrum shadow_frustrum{shadow_camera};

   struct shadow_render_list_item {
      D3D12_INDEX_BUFFER_VIEW index_buffer_view;
      D3D12_VERTEX_BUFFER_VIEW position_vertex_buffer_view;
      D3D12_GPU_VIRTUAL_ADDRESS object_transform_address;
      uint32 index_count;
      uint32 start_index;
      uint32 start_vertex;
   };

   static std::vector<shadow_render_list_item> render_list;
   render_list.clear();

   for (std::size_t i = 0; i < meshes.size(); ++i) {
      if (not intersects(shadow_frustrum, meshes.bbox[i])) continue;

      if (are_flags_set(meshes.pipeline_flags[i],
                        gpu::material_pipeline_flags::transparent)) {
         continue;
      }

      render_list.push_back(
         {.index_buffer_view = meshes.mesh[i].index_buffer_view,
          .position_vertex_buffer_view = meshes.mesh[i].vertex_buffer_views[0],

          .object_transform_address = meshes.gpu_constants[i],

          .index_count = meshes.mesh[i].index_count,
          .start_index = meshes.mesh[i].start_index,
          .start_vertex = meshes.mesh[i].start_vertex});
   }

   const D3D12_GPU_VIRTUAL_ADDRESS shadow_view_projection_matrix_address = [&] {
      auto allocation = dynamic_buffer_allocator.allocate(sizeof(float4x4));

      std::memcpy(allocation.cpu_address,
                  &shadow_camera.view_projection_matrix(), sizeof(float4x4));

      return allocation.gpu_address;
   }();

   auto depth_stencil_view = _shadow_map_dsv[0].cpu;

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(*_shadow_map.resource(),
                              D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                              D3D12_RESOURCE_STATE_DEPTH_WRITE));
   command_list.flush_deferred_resource_barriers();

   command_list.clear_depth_stencil_view(depth_stencil_view,
                                         D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0x0);

   command_list.set_graphics_root_signature(*_gpu_device->root_signatures.depth_only_mesh);
   command_list.set_graphics_root_constant_buffer(1, shadow_camera.view_projection_matrix());
   command_list.set_pipeline_state(*_gpu_device->pipelines.depth_only_mesh);

   command_list.ia_set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   command_list.rs_set_scissor_rects({0, 0, 2048, 2048});
   command_list.rs_set_viewports({0, 0, 2048, 2048, 0.0f, 1.0f});

   command_list.om_set_render_targets({}, false, _shadow_map_dsv.start().cpu);

   for (auto& mesh : render_list) {
      command_list.set_graphics_root_constant_buffer_view(0, mesh.object_transform_address);

      command_list.ia_set_index_buffer(mesh.index_buffer_view);
      command_list.ia_set_vertex_buffers(0, mesh.position_vertex_buffer_view);

      command_list.draw_indexed_instanced(mesh.index_count, 1, mesh.start_index,
                                          mesh.start_vertex, 0);
   }

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(*_shadow_map.resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE,
                              D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

   _shadow_transform = shadow_camera.view_projection_matrix();
}

auto light_clusters::light_descriptors() const noexcept -> gpu::descriptor_range
{
   return _resource_views.descriptors();
}
}
