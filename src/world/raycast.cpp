#include "raycast.hpp"
#include "math/intersectors.hpp"
#include "utility/look_for.hpp"

#include <limits>

#include <range/v3/view.hpp>

using namespace ranges::views;

namespace we::world {

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const light> lights, std::span<const region> regions) noexcept
   -> std::optional<raycast_result<light>>
{
   std::optional<light_id> hit;
   float min_distance = std::numeric_limits<float>::max();

   for (auto& light : lights) {
      if (light.light_type == light_type::directional) {
         // TODO: World proxies for region-less directional lights.

         const region* light_region = look_for(regions, [&](const region& region) {
            return region.description == light.directional_region;
         });

         if (not light_region) continue;

         if (light_region->shape == region_shape::box) {
            [[maybe_unused]] float3 box_normal{};

            quaternion inverse_light_region_rotation =
               glm::conjugate(light_region->rotation);

            float4x4 world_to_box{inverse_light_region_rotation};
            world_to_box[3] = {inverse_light_region_rotation * -light_region->position,
                               1.0f};

            float3 box_ray_origin = world_to_box * float4{ray_origin, 1.0f};
            float3 box_ray_direction =
               glm::normalize(float3x3{world_to_box} * ray_direction);

            const float intersection =
               boxIntersection(box_ray_origin, box_ray_direction,
                               light_region->size, box_normal)
                  .x;

            if (intersection < 0.0f) continue;

            if (intersection < min_distance) {
               hit = light.id;
               min_distance = intersection;
            }
         }
         else if (light_region->shape == region_shape::sphere) {
            const float intersection =
               sphIntersect(ray_origin, ray_direction, light_region->position,
                            glm::length(light_region->size));

            if (intersection < 0.0f) continue;

            if (intersection < min_distance) {
               hit = light.id;
               min_distance = intersection;
            }
         }
         else if (light_region->shape == region_shape::cylinder) {
            const float cylinder_radius =
               glm::length(float2{light_region->size.x, light_region->size.z});
            const float3 region_direction =
               glm::normalize(light_region->rotation * float3{0.0f, 1.0f, 0.0f});

            const float intersection =
               iCylinder(ray_origin, ray_direction,
                         light_region->position +
                            region_direction * light_region->size.y,
                         light_region->position +
                            -region_direction * light_region->size.y,
                         cylinder_radius)
                  .x;

            if (intersection < 0.0f) continue;

            if (intersection < min_distance) {
               hit = light.id;
               min_distance = intersection;
            }
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
            glm::normalize(light.rotation * float3{0.0f, 0.0f, 1.0f});
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
   }

   if (not hit) return std::nullopt;

   return raycast_result<light>{.distance = min_distance, .id = *hit};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const path> paths) noexcept
   -> std::optional<raycast_result<path>>
{
   std::optional<path_id> hit;
   std::size_t hit_node = 0;
   float min_distance = std::numeric_limits<float>::max();

   for (auto& path : paths) {
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
             std::span<const region> regions) noexcept
   -> std::optional<raycast_result<region>>
{
   std::optional<region_id> hit;
   float min_distance = std::numeric_limits<float>::max();

   for (auto& region : regions) {
      if (region.shape == region_shape::box) {
         [[maybe_unused]] float3 box_normal{};

         quaternion inverse_light_region_rotation = glm::conjugate(region.rotation);

         float4x4 world_to_box{inverse_light_region_rotation};
         world_to_box[3] = {inverse_light_region_rotation * -region.position, 1.0f};

         float3 box_ray_origin = world_to_box * float4{ray_origin, 1.0f};
         float3 box_ray_direction =
            glm::normalize(float3x3{world_to_box} * ray_direction);

         const float intersection =
            boxIntersection(box_ray_origin, box_ray_direction, region.size, box_normal)
               .x;

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = region.id;
            min_distance = intersection;
         }
      }
      else if (region.shape == region_shape::sphere) {
         const float intersection =
            sphIntersect(ray_origin, ray_direction, region.position,
                         glm::length(region.size));

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = region.id;
            min_distance = intersection;
         }
      }
      else if (region.shape == region_shape::cylinder) {
         const float cylinder_radius =
            glm::length(float2{region.size.x, region.size.z});
         const float3 region_direction =
            glm::normalize(region.rotation * float3{0.0f, 1.0f, 0.0f});

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
         }
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<sector>{.distance = min_distance, .id = *hit};
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
             std::span<const hintnode> hintnodes) noexcept
   -> std::optional<raycast_result<hintnode>>
{
   std::optional<hintnode_id> hit;
   float min_distance = std::numeric_limits<float>::max();

   for (auto& hintnode : hintnodes) {
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
   uint32 hit_edge = 0;
   float min_distance = std::numeric_limits<float>::max();

   for (auto& barrier : barriers) {
      for (uint32 i = 0; i < barrier.corners.size(); ++i) {
         const float2 a = barrier.corners[i];
         const float2 b = barrier.corners[(i + 1u) % 4u];

         const std::array quad = {float3{a.x, barrier_height, a.y},
                                  float3{a.x, -barrier_height, a.y},
                                  float3{b.x, barrier_height, b.y},
                                  float3{b.x, -barrier_height, b.y}};

         const float intersection = quadIntersect(ray_origin, ray_direction,
                                                  quad[0], quad[1], quad[3], quad[2])
                                       .x;

         if (intersection < 0.0f) continue;

         if (intersection < min_distance) {
            hit = barrier.id;
            hit_edge = i;
            min_distance = intersection;
         }
      }
   }

   if (not hit) return std::nullopt;

   return raycast_result<barrier>{.distance = min_distance, .id = *hit, .edge = hit_edge};
}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const boundary> boundaries, std::span<const path> paths,
             const float boundary_height) noexcept
   -> std::optional<raycast_result<boundary>>
{
   std::optional<boundary_id> hit;
   float min_distance = std::numeric_limits<float>::max();

   for (auto& boundary : boundaries) {
      const path* boundary_path = look_for(paths, [&](const path& path) {
         return path.name == boundary.name;
      });

      if (not boundary_path) continue;

      for (const auto [a, b] :
           zip(boundary_path->nodes, concat(boundary_path->nodes | drop(1),
                                            boundary_path->nodes | take(1)))) {

         const std::array quad = {a.position, b.position,
                                  a.position + float3{0.0f, boundary_height, 0.0f},
                                  b.position + float3{0.0f, boundary_height, 0.0f}};

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