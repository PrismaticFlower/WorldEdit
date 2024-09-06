#include "entity_group_utilities.hpp"
#include "../object_class.hpp"

#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"
#include "utility/string_icompare.hpp"

namespace we::world {

namespace {

template<typename T>
auto get_placed_entity_name_impl(std::string_view name, std::span<const T> world_entities,
                                 std::span<const std::type_identity_t<T>> group_entities,
                                 const uint32 group_base_index) noexcept
   -> std::string_view
{
   if (world_entities.size() < group_base_index) return name;
   if (world_entities.size() - group_base_index < group_entities.size()) {
      return name;
   }

   for (uint32 i = 0; i < group_entities.size(); ++i) {
      if (string::iequals(name, group_entities[i].name)) {
         return world_entities[group_base_index + i].name;
      }
   }

   return name;
}

}

auto entity_group_bbox(const entity_group& group,
                       const object_class_library& object_classes) noexcept
   -> math::bounding_box
{
   math::bounding_box group_bbox{.min = float3{FLT_MAX, FLT_MAX, FLT_MAX},
                                 .max = float3{-FLT_MAX, -FLT_MAX, -FLT_MAX}};

   for (const object& object : group.objects) {
      math::bounding_box bbox = object_classes[object.class_handle].model->bounding_box;

      bbox = object.rotation * bbox + object.position;

      group_bbox = math::combine(group_bbox, bbox);
   }

   for (const path& path : group.paths) {
      for (const path::node& node : path.nodes) {
         group_bbox = math::integrate(group_bbox, node.position);
      }
   }

   for (const light& light : group.lights) {
      switch (light.light_type) {
      case light_type::directional: {
         group_bbox = math::integrate(group_bbox, light.position);

      } break;
      case light_type::point: {
         const math::bounding_box bbox{.min = light.position - light.range,
                                       .max = light.position + light.range};

         group_bbox = math::combine(bbox, group_bbox);
      } break;
      case light_type::spot: {
         const float outer_cone_radius =
            light.range * std::tan(light.outer_cone_angle * 0.5f);
         const float half_radius = outer_cone_radius * 0.5f;

         math::bounding_box bbox{.min = {-half_radius, 0.0f, -half_radius},
                                 .max = {half_radius, light.range, half_radius}};

         bbox = light.rotation * bbox + light.position;

         group_bbox = math::combine(bbox, group_bbox);
      } break;
      case light_type::directional_region_box: {
         math::bounding_box bbox{.min = {-light.region_size},
                                 .max = {light.region_size}};

         bbox = light.region_rotation * bbox + light.position;

         group_bbox = math::combine(bbox, group_bbox);
      } break;
      case light_type::directional_region_sphere: {
         const float sphere_radius = length(light.region_size);

         const math::bounding_box bbox{.min = light.position - sphere_radius,
                                       .max = light.position + sphere_radius};

         group_bbox = math::combine(bbox, group_bbox);
      } break;
      case light_type::directional_region_cylinder: {
         const float cylinder_length =
            length(float2{light.region_size.x, light.region_size.z});

         math::bounding_box bbox{.min = {-cylinder_length, -light.region_size.y,
                                         -cylinder_length},
                                 .max = {cylinder_length, light.region_size.y,
                                         cylinder_length}};

         bbox = light.region_rotation * bbox + light.position;

         group_bbox = math::combine(bbox, group_bbox);
      } break;
      }
   }

   for (const region& region : group.regions) {
      switch (region.shape) {
      case region_shape::box: {
         math::bounding_box bbox{.min = {-region.size}, .max = {region.size}};

         bbox = region.rotation * bbox + region.position;

         group_bbox = math::combine(bbox, group_bbox);
      } break;
      case region_shape::sphere: {
         const float sphere_radius = length(region.size);

         const math::bounding_box bbox{.min = region.position - sphere_radius,
                                       .max = region.position + sphere_radius};

         group_bbox = math::combine(bbox, group_bbox);
      } break;
      case region_shape::cylinder: {
         const float cylinder_length = length(float2{region.size.x, region.size.z});

         math::bounding_box bbox{.min = {-cylinder_length, -region.size.y, -cylinder_length},
                                 .max = {cylinder_length, region.size.y, cylinder_length}};

         bbox = region.rotation * bbox + region.position;

         group_bbox = math::combine(bbox, group_bbox);
      } break;
      }
   }

   for (const sector& sector : group.sectors) {
      for (auto& point : sector.points) {
         group_bbox.min.x = std::min(group_bbox.min.x, point.x);
         group_bbox.min.z = std::min(group_bbox.min.z, point.y);

         group_bbox.max.x = std::max(group_bbox.max.x, point.x);
         group_bbox.max.z = std::max(group_bbox.max.z, point.y);
      }

      group_bbox.min.y = std::min(group_bbox.min.y, sector.base);
      group_bbox.max.y = std::max(group_bbox.max.y, sector.base + sector.height);
   }

   for (const portal& portal : group.portals) {
      const float half_width = portal.width * 0.5f;
      const float half_height = portal.height * 0.5f;

      math::bounding_box bbox{.min = {-half_width, -half_height, 0.0f},
                              .max = {half_width, half_height, 0.0f}};

      bbox = portal.rotation * bbox + portal.position;

      group_bbox = math::combine(bbox, group_bbox);
   }

   for (const hintnode& hintnode : group.hintnodes) {
      group_bbox = math::integrate(group_bbox, hintnode.position);
   }

   for (const barrier& barrier : group.barriers) {
      group_bbox = math::integrate(group_bbox, barrier.position);
   }

   for (const planning_hub& hub : group.planning_hubs) {
      group_bbox = math::integrate(group_bbox, hub.position);
   }

   for (const boundary& boundary : group.boundaries) {
      group_bbox = math::integrate(group_bbox, boundary.position);
   }

   for (const measurement& measurement : group.measurements) {
      group_bbox = math::integrate(group_bbox, measurement.start);
      group_bbox = math::integrate(group_bbox, measurement.end);
   }

   return group_bbox;
}

auto get_placed_entity_name(std::string_view name, std::span<const object> world_objects,
                            const entity_group& group,
                            const uint32 group_base_index) noexcept -> std::string_view
{
   return get_placed_entity_name_impl(name, world_objects, group.objects,
                                      group_base_index);
}

auto get_placed_entity_name(std::string_view name, std::span<const path> world_paths,
                            const entity_group& group,
                            const uint32 group_base_index) noexcept -> std::string_view
{
   return get_placed_entity_name_impl(name, world_paths, group.paths, group_base_index);
}

auto get_placed_entity_name(std::string_view name, std::span<const sector> world_sectors,
                            const entity_group& group,
                            const uint32 group_base_index) noexcept -> std::string_view
{
   return get_placed_entity_name_impl(name, world_sectors, group.sectors,
                                      group_base_index);
}

}