
#include "light_clusters.hpp"
#include "gpu/barrier_helpers.hpp"
#include "world/world_utilities.hpp"

#include <cmath>

#include <boost/container/static_vector.hpp>

namespace sk::graphics {

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

constexpr auto max_lights = ((D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16) - 16) /
                            sizeof(light_description);
constexpr auto max_regional_lights = 512;

struct light_constants {
   uint32 light_count = 0;
   std::array<uint32, 3> padding0;

   std::array<light_description, max_lights> lights;

   std::array<float4, 3> padding1;
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

   _descriptors =
      _gpu_device->allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2);

   _gpu_device->create_constant_buffer_view(
      {.buffer_location = _lights_constant_buffer.resource()->GetGPUVirtualAddress(),
       .size = _lights_constant_buffer.size()},
      _descriptors[0]);

   _gpu_device->create_shader_resource_view(
      *_regional_lights_buffer.resource(),
      gpu::shader_resource_view_desc{
         .type_description = gpu::buffer_srv{.first_element = 0,
                                             .number_elements = max_regional_lights,
                                             .structure_byte_stride =
                                                sizeof(light_region_description)}},
      _descriptors[1]);
}

void light_clusters::update_lights(const frustrum& view_frustrum,
                                   const world::world& world,
                                   gpu::command_list& command_list,
                                   gpu::dynamic_buffer_allocator& dynamic_buffer_allocator)
{
   light_constants light_constants{.light_count = 0};

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
                                   *dynamic_buffer_allocator.resource(),
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
                                   *dynamic_buffer_allocator.resource(),
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

auto light_clusters::light_descriptors() const noexcept -> gpu::descriptor_range
{
   return _descriptors;
}

}
