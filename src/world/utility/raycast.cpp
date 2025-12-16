#include "raycast.hpp"

#include "../object_class.hpp"

#include "math/iq_intersectors.hpp"
#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <limits>

namespace we::world {

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const active_layers active_layers, std::span<const object> objects,
             const object_class_library& object_classes,
             function_ptr<bool(const object&) noexcept> filter) noexcept
   -> std::optional<raycast_result<object>>
{
   using namespace assets;

   std::optional<object_id> hit;
   uint32 hit_index = 0;
   float min_distance = std::numeric_limits<float>::max();
   float3 surface_normalWS;

   for (std::size_t object_index = 0; object_index < objects.size(); ++object_index) {
      const object& object = objects[object_index];

      if (not active_layers[object.layer]) continue;
      if (object.hidden) continue;
      if (filter and not filter(object)) continue;

      quaternion inverse_rotation = conjugate(object.rotation);
      float3 inverse_position = inverse_rotation * -object.position;

      float3 obj_ray_origin = inverse_rotation * ray_origin + inverse_position;
      float3 obj_ray_direction = normalize(inverse_rotation * ray_direction);

      const msh::flat_model& model = *object_classes[object.class_handle].model;

      float3 box_centre = (model.bounding_box.min + model.bounding_box.max) * 0.5f;
      float3 box_size = (model.bounding_box.max - model.bounding_box.min) * 0.5f;

      const float box_intersection =
         boxIntersection(obj_ray_origin - box_centre, obj_ray_direction, box_size);

      if (box_intersection < 0.0f) continue;

      std::optional<msh::ray_hit> model_hit =
         model.bvh.query(obj_ray_origin, obj_ray_direction);

      if (not model_hit) continue;

      if (model_hit->distance < min_distance) {
         hit = object.id;
         hit_index = static_cast<uint32>(object_index);
         min_distance = model_hit->distance;
         surface_normalWS =
            normalize(object.rotation * model_hit->unnormalized_normal);
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<object>{.distance = min_distance,
                                 .normalWS = surface_normalWS,
                                 .id = *hit,
                                 .index = hit_index};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const active_layers active_layers, std::span<const light> lights,
             const raycast_light_sizes& sizes,
             function_ptr<bool(const light&) noexcept> filter) noexcept
   -> std::optional<raycast_result<light>>
{
   std::optional<light_id> hit;
   uint32 hit_index = 0;
   float min_distance = std::numeric_limits<float>::max();

   for (std::size_t light_index = 0; light_index < lights.size(); ++light_index) {
      const light& light = lights[light_index];

      if (not active_layers[light.layer]) continue;
      if (light.hidden) continue;
      if (filter and not filter(light)) continue;

      if (light.light_type == light_type::directional or
          light.light_type == light_type::point or
          light.light_type == light_type::spot) {
         float proxy_radius = 0.0f;

         if (light.light_type == light_type::directional) {
            proxy_radius = sizes.directional;
         }
         else if (light.light_type == light_type::point) {
            proxy_radius = sizes.point;
         }
         else if (light.light_type == light_type::spot) {
            proxy_radius = sizes.spot;
         }

         const float intersection =
            sphIntersect(ray_origin, ray_direction, light.position, proxy_radius);

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = light.id;
            hit_index = static_cast<uint32>(light_index);
            min_distance = intersection;
         }
      }
      else if (light.light_type == light_type::directional_region_box) {
         quaternion inverse_rotation = conjugate(light.region_rotation);
         float3 inverse_position = inverse_rotation * -light.position;

         float3 box_ray_origin = inverse_rotation * ray_origin + inverse_position;
         float3 box_ray_direction = normalize(inverse_rotation * ray_direction);

         const float intersection =
            boxIntersection(box_ray_origin, box_ray_direction, light.region_size);

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = light.id;
            hit_index = static_cast<uint32>(light_index);
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
            hit_index = static_cast<uint32>(light_index);
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
            hit_index = static_cast<uint32>(light_index);
            min_distance = intersection;
         }
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<light>{.distance = min_distance, .id = *hit, .index = hit_index};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const active_layers active_layers, std::span<const path> paths,
             const float node_size,
             function_ptr<bool(const path&, uint32) noexcept> filter) noexcept
   -> std::optional<raycast_result<path>>
{
   std::optional<path_id> hit;
   uint32 hit_index = 0;
   uint32 hit_node = 0;
   float min_distance = std::numeric_limits<float>::max();

   for (std::size_t path_index = 0; path_index < paths.size(); ++path_index) {
      const path& path = paths[path_index];

      if (not active_layers[path.layer]) continue;
      if (path.hidden) continue;

      for (uint32 i = 0; i < path.nodes.size(); ++i) {
         if (filter and not filter(path, i)) continue;

         const path::node& node = path.nodes[i];

         const float intersection =
            sphIntersect(ray_origin, ray_direction, node.position,
                         0.707f * (node_size / 0.5f));

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = path.id;
            hit_index = static_cast<uint32>(path_index);
            hit_node = i;
            min_distance = intersection;
         }
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<path>{.distance = min_distance,
                               .id = *hit,
                               .index = hit_index,
                               .node_index = hit_node};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const active_layers active_layers, std::span<const region> regions,
             function_ptr<bool(const region&) noexcept> filter) noexcept
   -> std::optional<raycast_result<region>>
{
   std::optional<region_id> hit;
   uint32 hit_index = 0;
   float min_distance = std::numeric_limits<float>::max();

   for (std::size_t region_index = 0; region_index < regions.size(); ++region_index) {
      const region& region = regions[region_index];

      if (not active_layers[region.layer]) continue;
      if (region.hidden) continue;
      if (filter and not filter(region)) continue;

      if (region.shape == region_shape::box) {
         quaternion inverse_rotation = conjugate(region.rotation);
         float3 inverse_position = inverse_rotation * -region.position;

         float3 box_ray_origin = inverse_rotation * ray_origin + inverse_position;
         float3 box_ray_direction = normalize(inverse_rotation * ray_direction);

         const float intersection =
            boxIntersection(box_ray_origin, box_ray_direction, region.size);

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = region.id;
            hit_index = static_cast<uint32>(region_index);
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
            hit_index = static_cast<uint32>(region_index);
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
            hit_index = static_cast<uint32>(region_index);
            min_distance = intersection;
         }
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<region>{.distance = min_distance, .id = *hit, .index = hit_index};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const sector> sectors,
             function_ptr<bool(const sector&) noexcept> filter) noexcept
   -> std::optional<raycast_result<sector>>
{
   std::optional<sector_id> hit;
   uint32 hit_index = 0;
   float min_distance = std::numeric_limits<float>::max();
   float3 normalWS;

   for (std::size_t sector_index = 0; sector_index < sectors.size(); ++sector_index) {
      const sector& sector = sectors[sector_index];

      if (sector.hidden) continue;
      if (filter and not filter(sector)) continue;

      for (std::size_t i = 0; i < sector.points.size(); ++i) {
         const float2 a = sector.points[i];
         const float2 b = sector.points[(i + 1) % sector.points.size()];

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
            hit_index = static_cast<uint32>(sector_index);
            min_distance = intersection;
            normalWS = normalize(cross(quad[1] - quad[0], quad[2] - quad[0]));
         }
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<sector>{.distance = min_distance,
                                 .normalWS = normalWS,
                                 .id = *hit,
                                 .index = hit_index};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const portal> portals,
             function_ptr<bool(const portal&) noexcept> filter) noexcept
   -> std::optional<raycast_result<portal>>
{
   std::optional<portal_id> hit;
   uint32 hit_index = 0;
   float min_distance = std::numeric_limits<float>::max();

   for (std::size_t portal_index = 0; portal_index < portals.size(); ++portal_index) {
      const portal& portal = portals[portal_index];

      if (portal.hidden) continue;
      if (filter and not filter(portal)) continue;

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
         hit_index = static_cast<uint32>(portal_index);
         min_distance = intersection;
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<portal>{.distance = min_distance, .id = *hit, .index = hit_index};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const active_layers active_layers, std::span<const hintnode> hintnodes,
             function_ptr<bool(const hintnode&) noexcept> filter) noexcept
   -> std::optional<raycast_result<hintnode>>
{
   std::optional<hintnode_id> hit;
   uint32 hit_index = 0;
   float min_distance = std::numeric_limits<float>::max();

   for (std::size_t hintnode_index = 0; hintnode_index < hintnodes.size();
        ++hintnode_index) {
      const hintnode& hintnode = hintnodes[hintnode_index];

      if (not active_layers[hintnode.layer]) continue;
      if (hintnode.hidden) continue;
      if (filter and not filter(hintnode)) continue;

      const float bounding_intersection =
         sphIntersect(ray_origin, ray_direction, hintnode.position, 2.0f);

      if (bounding_intersection < 0.0f) continue;

      constexpr static std::array<float3, 8> hexahedron_vertices{
         {{0.000000f, 1.000000f, 1.000000f},
          {-0.866025f, 1.000000f, -0.500000f},
          {0.866025f, 1.000000f, -0.500000f},
          {0.000000f, 2.000000f, 0.000000f},
          {0.000000f, 1.000000f, 1.000000f},
          {0.866025f, 1.000000f, -0.500000f},
          {-0.866026f, 1.000000f, -0.500000f},
          {0.000000f, 0.000000f, -0.000000f}}};

      constexpr static std::array<std::array<uint16, 3>, 6> hexahedron_indices{
         {{0, 3, 1}, {1, 3, 2}, {2, 3, 0}, {4, 7, 5}, {5, 7, 6}, {6, 7, 4}}};

      quaternion inverse_rotation = conjugate(hintnode.rotation);
      float3 inverse_position = inverse_rotation * -hintnode.position;

      const float3 node_ray_origin = inverse_rotation * ray_origin + inverse_position;
      const float3 node_ray_direction = normalize(inverse_rotation * ray_direction);

      for (const auto& [i0, i1, i2] : hexahedron_indices) {
         const float intersection =
            triIntersect(node_ray_origin, node_ray_direction, hexahedron_vertices[i0],
                         hexahedron_vertices[i1], hexahedron_vertices[i2])
               .x;

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = hintnode.id;
            hit_index = static_cast<uint32>(hintnode_index);
            min_distance = intersection;
         }
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<hintnode>{.distance = min_distance, .id = *hit, .index = hit_index};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const barrier> barriers, const float barrier_height,
             function_ptr<bool(const barrier&) noexcept> filter) noexcept
   -> std::optional<raycast_result<barrier>>
{
   std::optional<barrier_id> hit;
   uint32 hit_index = 0;
   float min_distance = std::numeric_limits<float>::max();

   for (std::size_t barrier_index = 0; barrier_index < barriers.size(); ++barrier_index) {
      const barrier& barrier = barriers[barrier_index];

      if (barrier.hidden) continue;
      if (filter and not filter(barrier)) continue;

      float4x4 world_to_box = transpose(
         make_rotation_matrix_from_euler({0.0f, barrier.rotation_angle, 0.0f}));
      world_to_box[3] = {world_to_box * -barrier.position, 1.0f};

      float3 box_ray_origin = world_to_box * ray_origin;
      float3 box_ray_direction = normalize(float3x3{world_to_box} * ray_direction);

      const float intersection =
         boxIntersection(box_ray_origin, box_ray_direction,
                         {barrier.size.x, barrier_height, barrier.size.y});

      if (intersection < 0.0f) continue;

      if (intersection < min_distance) {
         hit = barrier.id;
         hit_index = static_cast<uint32>(barrier_index);
         min_distance = intersection;
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<barrier>{.distance = min_distance, .id = *hit, .index = hit_index};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const planning_hub> hubs, const float hub_height,
             function_ptr<bool(const planning_hub&) noexcept> filter) noexcept
   -> std::optional<raycast_result<planning_hub>>
{
   std::optional<planning_hub_id> hit;
   uint32 hit_index = 0;
   float min_distance = std::numeric_limits<float>::max();

   for (std::size_t hub_index = 0; hub_index < hubs.size(); ++hub_index) {
      const planning_hub& hub = hubs[hub_index];

      if (hub.hidden) continue;
      if (filter and not filter(hub)) continue;

      const float3 top_position = hub.position + float3{0.0f, hub_height, 0.0f};
      const float3 bottom_position = hub.position + float3{0.0f, -hub_height, 0.0f};

      const float intersection = iCylinder(ray_origin, ray_direction, top_position,
                                           bottom_position, hub.radius)
                                    .x;

      if (intersection < 0.0f) continue;

      if (intersection < min_distance) {
         hit = hub.id;
         hit_index = static_cast<uint32>(hub_index);
         min_distance = intersection;
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<planning_hub>{.distance = min_distance,
                                       .id = *hit,
                                       .index = hit_index};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const planning_connection> connections,
             std::span<const planning_hub> hubs, const float connection_height,
             function_ptr<bool(const planning_connection&) noexcept> filter) noexcept
   -> std::optional<raycast_result<planning_connection>>
{
   std::optional<planning_connection_id> hit;
   uint32 hit_index = 0;
   float min_distance = std::numeric_limits<float>::max();

   for (std::size_t connection_index = 0; connection_index < connections.size();
        ++connection_index) {
      const planning_connection& connection = connections[connection_index];

      if (connection.hidden) continue;
      if (filter and not filter(connection)) continue;

      const planning_hub& start = hubs[connection.start_hub_index];
      const planning_hub& end = hubs[connection.end_hub_index];

      const math::bounding_box start_bbox{
         .min = float3{-start.radius, -connection_height, -start.radius} + start.position,
         .max = float3{start.radius, connection_height, start.radius} + start.position};
      const math::bounding_box end_bbox{
         .min = float3{-end.radius, -connection_height, -end.radius} + end.position,
         .max = float3{end.radius, connection_height, end.radius} + end.position};

      const math::bounding_box bbox = math::combine(start_bbox, end_bbox);

      const float3 bbox_centre = (bbox.min + bbox.max) / 2.0f;
      const float3 bbox_size = (bbox.max - bbox.min) / 2.0f;

      if (boxIntersection(ray_origin - bbox_centre, ray_direction, bbox_size) < 0.0f) {
         continue;
      }

      const float3 normal =
         normalize(float3{-(start.position.z - end.position.z), 0.0f,
                          start.position.x - end.position.x});

      std::array<float3, 4> points{start.position + normal * start.radius,
                                   start.position - normal * start.radius,
                                   end.position + normal * end.radius,
                                   end.position - normal * end.radius};

      const float3 height_offset = {0.0f, connection_height, 0.0f};

      std::array<float3, 8> corners = {points[0] + height_offset,
                                       points[1] + height_offset,
                                       points[2] + height_offset,
                                       points[3] + height_offset,
                                       points[0] - height_offset,
                                       points[1] - height_offset,
                                       points[2] - height_offset,
                                       points[3] - height_offset};

      constexpr std::array<std::array<uint32, 3>, 12> tris = {{
         // Top
         {3, 2, 0}, //
         {3, 0, 1}, //

         // Bottom
         {4, 6, 7}, //
         {5, 4, 7}, //

         // Side 0
         {0, 6, 4}, //
         {0, 2, 6}, //

         // Side 1
         {1, 5, 7}, //
         {7, 3, 1}, //

         // Back
         {4, 1, 0}, //
         {5, 1, 4}, //

         // Front
         {2, 3, 6}, //
         {6, 3, 7}  //
      }};

      for (const auto& tri : tris) {
         const float intersection =
            triIntersect(ray_origin, ray_direction, corners[tri[0]],
                         corners[tri[1]], corners[tri[2]])
               .x;

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = connection.id;
            hit_index = static_cast<uint32>(connection_index);
            min_distance = intersection;
         }
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<planning_connection>{.distance = min_distance,
                                              .id = *hit,
                                              .index = hit_index};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const boundary> boundaries, const float boundary_height,
             function_ptr<bool(const boundary&) noexcept> filter) noexcept
   -> std::optional<raycast_result<boundary>>
{
   std::optional<boundary_id> hit;
   uint32 hit_index = 0;
   float min_distance = std::numeric_limits<float>::max();

   for (std::size_t boundary_index = 0; boundary_index < boundaries.size();
        ++boundary_index) {
      const boundary& boundary = boundaries[boundary_index];

      if (boundary.hidden) continue;
      if (filter and not filter(boundary)) continue;

      const std::span<const float3> nodes = boundary.points;

      for (std::size_t i = 0; i < nodes.size(); ++i) {
         const float3 a = nodes[i];
         const float3 b = nodes[(i + 1) % nodes.size()];

         const std::array quad = {
            float3{a.x, a.y - boundary_height, a.z},
            float3{a.x, a.y + boundary_height, a.z},
            float3{b.x, b.y + boundary_height, b.z},
            float3{b.x, b.y - boundary_height, b.z},
         };

         const float intersection = quadIntersect(ray_origin, ray_direction,
                                                  quad[0], quad[1], quad[2], quad[3])
                                       .x;

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = boundary.id;
            hit_index = static_cast<uint32>(boundary_index);
            min_distance = intersection;
         }
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<boundary>{.distance = min_distance, .id = *hit, .index = hit_index};
}
}