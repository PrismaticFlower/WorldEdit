#include "make_command_post_linked_entities.hpp"
#include "math/vector_funcs.hpp"
#include "raycast.hpp"
#include "world_utilities.hpp"

#include <cmath>
#include <limits>

#include <fmt/core.h>

namespace we::world {

namespace {

auto make_spawn_path_nodes(const make_command_post_linked_entities_inputs inputs,
                           const std::span<const object> objects,
                           const object_class_library& object_classes,
                           const terrain_collision& terrain_collision)
   -> std::vector<path::node>
{
   constexpr std::array directions = {float3{0.000000f, 0.000000f, -1.000000f},
                                      float3{-0.707107f, 0.000000f, -0.707107f},
                                      float3{-1.000000f, 0.000000f, 0.000000f},
                                      float3{-0.707107f, 0.000000f, 0.707107f},
                                      float3{0.000000f, 0.000000f, 1.000000f},
                                      float3{0.707107f, 0.000000f, 0.707107f},
                                      float3{1.000000f, 0.000000f, 0.000000f},
                                      float3{0.707107f, 0.000000f, -0.707107f}};

   std::vector<path::node> nodes;

   nodes.reserve(8);

   for (const auto& direction : directions) {
      float hit_distance = std::numeric_limits<float>::max();

      if (std::optional<raycast_result<object>> hit =
             raycast(inputs.position, direction, active_layers{true}, objects,
                     object_classes);
          hit) {
         if (hit->distance < hit_distance) hit_distance = hit->distance;
      }

      if (auto hit = terrain_collision.raycast(inputs.position, direction); hit) {
         if (hit->distance < hit_distance) hit_distance = hit->distance;
      }

      if (hit_distance >= inputs.spawn_radius) {
         nodes.push_back({.position = inputs.position + direction * inputs.spawn_radius});
      }
   }

   if (nodes.empty()) nodes.push_back({.position = inputs.position});

   return nodes;
}

}

auto make_command_post_linked_entities(
   const make_command_post_linked_entities_inputs inputs,
   const std::span<const object> objects, const std::span<const path> paths,
   const std::span<const region> regions, const object_class_library& object_classes,
   const terrain_collision& terrain_collision) noexcept -> command_post_linked_entities
{
   command_post_linked_entities linked;

   const float capture_radius_sq = inputs.capture_radius * inputs.capture_radius;
   const float capture_size = std::sqrt(capture_radius_sq / 3.0f);

   const std::string capture_region_name =
      create_unique_name(regions, fmt::format("{}_capture", inputs.name));

   linked.capture_region = {.name = capture_region_name,
                            .layer = inputs.layer,
                            .position = inputs.position,
                            .size = {capture_size, capture_size, capture_size},
                            .shape = region_shape::sphere,
                            .description = capture_region_name};

   const std::string control_region_name =
      create_unique_name(regions, fmt::format("{}_control", inputs.name));

   const float control_radius_sq = inputs.control_radius * inputs.control_radius;
   const float control_size = std::sqrt(control_radius_sq / 2.0f);

   linked.control_region = {.name = control_region_name,
                            .layer = inputs.layer,
                            .position = inputs.position,
                            .size = {control_size, inputs.control_height, control_size},
                            .shape = region_shape::cylinder,
                            .description = control_region_name};

   linked.spawn_path = {.name =
                           create_unique_name(paths,
                                              fmt::format("{}_spawn", inputs.name)),
                        .layer = inputs.layer,
                        .nodes = make_spawn_path_nodes(inputs, objects, object_classes,
                                                       terrain_collision)};

   return linked;
}

}