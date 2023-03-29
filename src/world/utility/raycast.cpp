#include "raycast.hpp"
#include "../object_class.hpp"
#include "boundary_nodes.hpp"
#include "math/intersectors.hpp"
#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <limits>

#include <range/v3/view.hpp>

using namespace ranges::views;

namespace we::world {

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const active_layers active_layers, std::span<const object> objects,
             const object_class_library& object_classes) noexcept
   -> std::optional<raycast_result<object>>
{
   using namespace assets;

   std::optional<object_id> hit;
   float min_distance = std::numeric_limits<float>::max();
   float3 surface_normalWS;

   for (auto& object : objects) {
      if (not active_layers[object.layer]) continue;

      quaternion inverse_object_rotation = conjugate(object.rotation);

      float4x4 world_to_obj = to_matrix(inverse_object_rotation);
      world_to_obj[3] = {inverse_object_rotation * -object.position, 1.0f};

      float3 obj_ray_origin = world_to_obj * ray_origin;
      float3 obj_ray_direction = normalize(float3x3{world_to_obj} * ray_direction);

      const msh::flat_model& model = *object_classes[object.class_name].model;

      float3 box_centre = (model.bounding_box.min + model.bounding_box.max) * 0.5f;
      float3 box_size = model.bounding_box.max - model.bounding_box.min;

      const float box_intersection =
         boxIntersection(obj_ray_origin - box_centre, obj_ray_direction, box_size);

      if (box_intersection < 0.0f) continue;

      std::optional<msh::ray_hit> model_hit =
         model.bvh.query(obj_ray_origin, obj_ray_direction);

      if (not model_hit) continue;

      if (model_hit->distance < min_distance) {
         hit = object.id;
         min_distance = model_hit->distance;
         surface_normalWS =
            normalize(float3x3{transpose(world_to_obj)} * model_hit->normal);
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<object>{.distance = min_distance,
                                 .normalWS = surface_normalWS,
                                 .id = *hit};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const active_layers active_layers, std::span<const light> lights) noexcept
   -> std::optional<raycast_result<light>>
{
   std::optional<light_id> hit;
   float min_distance = std::numeric_limits<float>::max();

   for (auto& light : lights) {
      if (not active_layers[light.layer]) continue;

      if (light.light_type == light_type::directional) {
         const float intersection =
            sphIntersect(ray_origin, ray_direction, light.position, 2.8284f);

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = light.id;
            min_distance = intersection;
         }
      }
      else if (light.light_type == light_type::point) {
         const float intersection =
            sphIntersect(ray_origin, ray_direction, light.position, light.range);

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = light.id;
            min_distance = intersection;
         }
      }
      else if (light.light_type == light_type::spot) {
         const float3 light_direction =
            normalize(light.rotation * float3{0.0f, 0.0f, 1.0f});
         const float cone_radius =
            (light.range / 2.0f) * std::tan(light.outer_cone_angle);

         const float3 cone_end_position =
            light.position + light_direction * light.range;

         const float intersection =
            iCappedCone(ray_origin, ray_direction, light.position,
                        cone_end_position, 0.0f, cone_radius)
               .x;

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = light.id;
            min_distance = intersection;
         }
      }
      else if (light.light_type == light_type::directional_region_box) {
         quaternion inverse_light_region_rotation = conjugate(light.region_rotation);

         float4x4 world_to_box = to_matrix(inverse_light_region_rotation);
         world_to_box[3] = {inverse_light_region_rotation * -light.position, 1.0f};

         float3 box_ray_origin = world_to_box * ray_origin;
         float3 box_ray_direction = normalize(float3x3{world_to_box} * ray_direction);

         const float intersection =
            boxIntersection(box_ray_origin, box_ray_direction, light.region_size);

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = light.id;
            min_distance = intersection;
         }
      }
      else if (light.light_type == light_type::directional_region_sphere) {
         const float intersection =
            sphIntersect(ray_origin, ray_direction, light.position,
                         length(light.region_size));

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = light.id;
            min_distance = intersection;
         }
      }
      else if (light.light_type == light_type::directional_region_cylinder) {
         const float cylinder_radius =
            length(float2{light.region_size.x, light.region_size.z});
         const float3 region_direction =
            normalize(light.region_rotation * float3{0.0f, 1.0f, 0.0f});

         const float intersection =
            iCylinder(ray_origin, ray_direction,
                      light.position + region_direction * light.region_size.y,
                      light.position + -region_direction * light.region_size.y,
                      cylinder_radius)
               .x;

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = light.id;
            min_distance = intersection;
         }
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<light>{.distance = min_distance, .id = *hit};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const active_layers active_layers, std::span<const path> paths) noexcept
   -> std::optional<raycast_result<path>>
{
   std::optional<path_id> hit;
   std::size_t hit_node = 0;
   float min_distance = std::numeric_limits<float>::max();

   for (auto& path : paths) {
      if (not active_layers[path.layer]) continue;

      for (const auto& [i, node] : enumerate(path.nodes)) {

         const float intersection =
            sphIntersect(ray_origin, ray_direction, node.position, 0.707f);

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = path.id;
            hit_node = i;
            min_distance = intersection;
         }
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<path>{.distance = min_distance, .id = *hit, .node_index = hit_node};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const active_layers active_layers, std::span<const region> regions) noexcept
   -> std::optional<raycast_result<region>>
{
   std::optional<region_id> hit;
   float min_distance = std::numeric_limits<float>::max();

   for (auto& region : regions) {
      if (not active_layers[region.layer]) continue;

      if (region.shape == region_shape::box) {
         quaternion inverse_light_region_rotation = conjugate(region.rotation);

         float4x4 world_to_box = to_matrix(inverse_light_region_rotation);
         world_to_box[3] = {inverse_light_region_rotation * -region.position, 1.0f};

         float3 box_ray_origin = world_to_box * ray_origin;
         float3 box_ray_direction = normalize(float3x3{world_to_box} * ray_direction);

         const float intersection =
            boxIntersection(box_ray_origin, box_ray_direction, region.size);

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = region.id;
            min_distance = intersection;
         }
      }
      else if (region.shape == region_shape::sphere) {
         const float intersection =
            sphIntersect(ray_origin, ray_direction, region.position,
                         length(region.size));

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = region.id;
            min_distance = intersection;
         }
      }
      else if (region.shape == region_shape::cylinder) {
         const float cylinder_radius = length(float2{region.size.x, region.size.z});
         const float3 region_direction =
            normalize(region.rotation * float3{0.0f, 1.0f, 0.0f});

         const float intersection =
            iCylinder(ray_origin, ray_direction,
                      region.position + region_direction * region.size.y,
                      region.position + -region_direction * region.size.y,
                      cylinder_radius)
               .x;

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = region.id;
            min_distance = intersection;
         }
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<region>{.distance = min_distance, .id = *hit};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const sector> sectors) noexcept
   -> std::optional<raycast_result<sector>>
{
   std::optional<sector_id> hit;
   float min_distance = std::numeric_limits<float>::max();
   float3 normalWS;

   for (auto& sector : sectors) {
      for (const auto [a, b] : zip(sector.points, concat(sector.points | drop(1),
                                                         sector.points | take(1)))) {
         const std::array quad = {float3{a.x, sector.base, a.y},
                                  float3{b.x, sector.base, b.y},
                                  float3{a.x, sector.base + sector.height, a.y},
                                  float3{b.x, sector.base + sector.height, b.y}};

         const float intersection = quadIntersect(ray_origin, ray_direction,
                                                  quad[0], quad[1], quad[3], quad[2])
                                       .x;

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = sector.id;
            min_distance = intersection;
            normalWS = normalize(cross(quad[1] - quad[0], quad[2] - quad[0]));
         }
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<sector>{.distance = min_distance,
                                 .normalWS = normalWS,
                                 .id = *hit};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const portal> portals) noexcept
   -> std::optional<raycast_result<portal>>
{
   std::optional<portal_id> hit;
   float min_distance = std::numeric_limits<float>::max();

   for (auto& portal : portals) {
      const float half_width = portal.width * 0.5f;
      const float half_height = portal.height * 0.5f;

      std::array quad = {float3{-half_width, -half_height, 0.0f},
                         float3{half_width, -half_height, 0.0f},
                         float3{-half_width, half_height, 0.0f},
                         float3{half_width, half_height, 0.0f}};

      for (auto& v : quad) {
         v = portal.rotation * v;
         v += portal.position;
      }

      const float intersection = quadIntersect(ray_origin, ray_direction,
                                               quad[0], quad[1], quad[3], quad[2])
                                    .x;

      if (intersection < 0.0f) continue;

      if (intersection < min_distance) {
         hit = portal.id;
         min_distance = intersection;
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<portal>{.distance = min_distance, .id = *hit};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const active_layers active_layers, std::span<const hintnode> hintnodes) noexcept
   -> std::optional<raycast_result<hintnode>>
{
   std::optional<hintnode_id> hit;
   float min_distance = std::numeric_limits<float>::max();

   for (auto& hintnode : hintnodes) {
      if (not active_layers[hintnode.layer]) continue;

      const float intersection =
         sphIntersect(ray_origin, ray_direction, hintnode.position, 1.4142f);

      if (intersection < 0.0f) continue;

      if (intersection < min_distance) {
         hit = hintnode.id;
         min_distance = intersection;
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<hintnode>{.distance = min_distance, .id = *hit};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const barrier> barriers, const float barrier_height) noexcept
   -> std::optional<raycast_result<barrier>>
{
   std::optional<barrier_id> hit;
   float min_distance = std::numeric_limits<float>::max();

   for (auto& barrier : barriers) {
      float4x4 world_to_box = transpose(
         make_rotation_matrix_from_euler({0.0f, barrier.rotation_angle, 0.0f}));
      world_to_box[3] = {world_to_box * float3{-barrier.position.x, 0.0f,
                                               -barrier.position.y},
                         1.0f};

      float3 box_ray_origin = world_to_box * ray_origin;
      float3 box_ray_direction = normalize(float3x3{world_to_box} * ray_direction);

      const float intersection =
         boxIntersection(box_ray_origin, box_ray_direction,
                         {barrier.size.x, barrier_height, barrier.size.y});

      if (intersection < 0.0f) continue;

      if (intersection < min_distance) {
         hit = barrier.id;
         min_distance = intersection;
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<barrier>{.distance = min_distance, .id = *hit};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const planning_hub> hubs, const float hub_height) noexcept
   -> std::optional<raycast_result<planning_hub>>
{
   std::optional<planning_hub_id> hit;
   float min_distance = std::numeric_limits<float>::max();

   for (auto& hub : hubs) {
      const float3 top_position = {hub.position.x, hub_height, hub.position.y};
      const float3 bottom_position = {hub.position.x, -hub_height, hub.position.y};

      const float intersection = iCylinder(ray_origin, ray_direction, top_position,
                                           bottom_position, hub.radius)
                                    .x;

      if (intersection < 0.0f) continue;

      if (intersection < min_distance) {
         hit = hub.id;
         min_distance = intersection;
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<planning_hub>{.distance = min_distance, .id = *hit};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const planning_connection> connections,
             std::span<const planning_hub> hubs,
             const absl::flat_hash_map<planning_hub_id, std::size_t>& planning_hub_index,
             const float connection_height) noexcept
   -> std::optional<raycast_result<planning_connection>>
{
   std::optional<planning_connection_id> hit;
   float min_distance = std::numeric_limits<float>::max();

   for (auto& connection : connections) {
      const planning_hub& start = hubs[planning_hub_index.at(connection.start)];
      const planning_hub& end = hubs[planning_hub_index.at(connection.end)];

      const float3 start_position = {start.position.x, 0.0f, start.position.y};
      const float3 end_position = {end.position.x, 0.0f, end.position.y};

      const math::bounding_box start_bbox{
         .min = float3{-start.radius, -connection_height, -start.radius} + start_position,
         .max = float3{start.radius, connection_height, start.radius} + start_position};
      const math::bounding_box end_bbox{
         .min = float3{-end.radius, -connection_height, -end.radius} + end_position,
         .max = float3{end.radius, connection_height, end.radius} + end_position};

      const math::bounding_box bbox = math::combine(start_bbox, end_bbox);

      const float3 bbox_centre = (bbox.min + bbox.max) / 2.0f;
      const float3 bbox_size = (bbox.max - bbox.min) / 2.0f;

      if (boxIntersection(ray_origin - bbox_centre, ray_direction, bbox_size) < 0.0f) {
         continue;
      }

      const float3 normal =
         normalize(float3{-(start_position.z - end_position.z), 0.0f,
                          start_position.x - end_position.x});

      std::array<float3, 4> points{start_position + normal * start.radius,
                                   start_position - normal * start.radius,
                                   end_position + normal * end.radius,
                                   end_position - normal * end.radius};

      const float3 height_offset = {0.0f, connection_height, 0.0f};

      std::array<float3, 8> corners = {points[0] + height_offset,
                                       points[1] + height_offset,
                                       points[2] + height_offset,
                                       points[3] + height_offset,
                                       points[0] - height_offset,
                                       points[1] - height_offset,
                                       points[2] - height_offset,
                                       points[3] - height_offset};

      constexpr std::array<std::array<uint32, 4>, 6> quads = {{{0u, 1u, 2u, 3u},
                                                               {4u, 5u, 6u, 7u},
                                                               {0u, 2u, 4u, 6u},
                                                               {1u, 3u, 5u, 7u},
                                                               {0u, 1u, 4u, 5u},
                                                               {2u, 3u, 6u, 7u}}};

      for (const auto& quad : quads) {
         const float intersection =
            quadIntersect(ray_origin, ray_direction, corners[quad[0]],
                          corners[quad[1]], corners[quad[3]], corners[quad[2]])
               .x;

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = connection.id;
            min_distance = intersection;
         }
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<planning_connection>{.distance = min_distance, .id = *hit};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const boundary> boundaries, const float boundary_height) noexcept
   -> std::optional<raycast_result<boundary>>
{
   std::optional<boundary_id> hit;
   float min_distance = std::numeric_limits<float>::max();

   for (auto& boundary : boundaries) {
      const std::array<float2, 12> nodes = get_boundary_nodes(boundary);

      for (std::size_t i = 0; i < nodes.size(); ++i) {
         const float2 a = nodes[i];
         const float2 b = nodes[(i + 1) % nodes.size()];

         const std::array quad = {float3{a.x, 0.0f, a.y}, float3{b.x, 0.0f, b.y},
                                  float3{a.x, boundary_height, a.y},
                                  float3{b.x, boundary_height, b.y}};

         const float intersection = quadIntersect(ray_origin, ray_direction,
                                                  quad[0], quad[1], quad[3], quad[2])
                                       .x;

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = boundary.id;
            min_distance = intersection;
         }
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<boundary>{.distance = min_distance, .id = *hit};
}
}