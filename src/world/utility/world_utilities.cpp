
#include "world_utilities.hpp"
#include "math/distance_funcs.hpp"
#include "math/vector_funcs.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include <charconv>
#include <cstring>

#include <fmt/core.h>

namespace we::world {

namespace {

template<typename T>
auto missing_name_base() -> std::string_view
{
   if constexpr (std::is_same_v<T, object>) return "Object";
   if constexpr (std::is_same_v<T, light>) return "Light";
   if constexpr (std::is_same_v<T, path>) return "Path";
   if constexpr (std::is_same_v<T, region>) return "Region";
   if constexpr (std::is_same_v<T, sector>) return "Sector";
   if constexpr (std::is_same_v<T, portal>) return "Portal";
   if constexpr (std::is_same_v<T, hintnode>) return "Hintnodes";
   if constexpr (std::is_same_v<T, barrier>) return "Barrier";
   if constexpr (std::is_same_v<T, planning_hub>) return "Hub";
   if constexpr (std::is_same_v<T, planning_connection>) return "Connection";
   if constexpr (std::is_same_v<T, boundary>) return "Boundary";
   if constexpr (std::is_same_v<T, animation>) return "Animation";
   if constexpr (std::is_same_v<T, animation_group>) return "AnimationGroup";
}

template<typename T>
auto create_base_name(const std::string_view name,
                      const std::string_view base_name = missing_name_base<T>())
   -> std::string_view
{
   std::string_view stripped_name = string::trim_trailing_digits(name);

   if (stripped_name.empty()) {
      stripped_name = base_name;
   }

   return stripped_name;
}

template<typename T>
bool is_name_unique(const std::span<const T> entities,
                    const std::string_view reference_name) noexcept
{
   for (auto& entity : entities) {
      if (string::iequals(entity.name, reference_name)) return false;
   }

   return true;
}

bool is_light_region_name_unique(const std::span<const light> lights,
                                 const std::string_view reference_name) noexcept
{
   for (auto& light : lights) {
      if (not is_region_light(light)) continue;

      if (string::iequals(light.region_name, reference_name)) return false;
   }

   return true;
}

bool is_light_name_and_region_name_unique(const std::span<const light> lights,
                                          const std::string_view reference_name) noexcept
{
   for (auto& light : lights) {
      if (string::iequals(light.name, reference_name)) return false;

      if (is_region_light(light) and
          string::iequals(light.region_name, reference_name)) {
         return false;
      }
   }

   return true;
}

template<typename T>
auto create_unique_name_impl(const std::span<const T> entities,
                             const std::string_view reference_name) -> std::string
{
   if (reference_name.empty()) return "";

   if (is_name_unique(entities, reference_name)) {
      return std::string{reference_name};
   }

   const std::string_view base_name = create_base_name<T>(reference_name);

   uint64 max_index = 0;

   for (auto& entity : entities) {
      if (string::istarts_with(entity.name, base_name)) {
         const std::string_view name_index =
            std::string_view{entity.name}.substr(base_name.size());

         uint64 index = 0;

         if (std::from_chars_result result =
                std::from_chars(name_index.data(),
                                name_index.data() + name_index.size(), index);
             result.ec == std::errc{} and
             result.ptr == name_index.data() + name_index.size()) {
            max_index = std::max(max_index, index);
         }
      }
   }

   return fmt::format("{}{}", base_name, max_index + 1);
}

}

auto get_hub_index(const std::span<const planning_hub> hubs, planning_hub_id id) -> uint32
{
   return static_cast<uint32>(
      find_entity<const planning_hub, planning_hub_id>(hubs, id) - hubs.data());
}

auto create_unique_name(const std::span<const object> entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::span<const light> entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::span<const path> entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::span<const sector> entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::span<const portal> entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::span<const hintnode> entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::span<const barrier> entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::span<const planning_hub> entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::span<const planning_connection> entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::span<const boundary> entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::span<const animation> entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::span<const animation_group> entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::span<const region> regions,
                        const std::span<const light> lights,
                        const std::string_view reference_name) -> std::string
{
   if (reference_name.empty()) return "";

   if (is_name_unique(regions, reference_name) and
       is_light_region_name_unique(lights, reference_name)) {
      return std::string{reference_name};
   }

   const std::string_view base_name = create_base_name<region>(reference_name);

   uint64 max_index = 0;

   for (const region& region : regions) {
      if (string::istarts_with(region.name, base_name)) {
         const std::string_view name_index =
            std::string_view{region.name}.substr(base_name.size());

         uint64 index = 0;

         if (std::from_chars_result result =
                std::from_chars(name_index.data(),
                                name_index.data() + name_index.size(), index);
             result.ec == std::errc{} and
             result.ptr == name_index.data() + name_index.size()) {
            max_index = std::max(max_index, index);
         }
      }
   }

   for (const light& light : lights) {
      if (not is_region_light(light)) continue;

      if (string::istarts_with(light.region_name, base_name)) {
         const std::string_view name_index =
            std::string_view{light.region_name}.substr(base_name.size());

         uint64 index = 0;

         if (std::from_chars_result result =
                std::from_chars(name_index.data(),
                                name_index.data() + name_index.size(), index);
             result.ec == std::errc{} and
             result.ptr == name_index.data() + name_index.size()) {
            max_index = std::max(max_index, index);
         }
      }
   }

   return fmt::format("{}{}", base_name, max_index + 1);
}

auto create_unique_light_region_name(const std::span<const light> lights,
                                     const std::span<const region> regions,
                                     const std::string_view reference_name) -> std::string
{
   if (not reference_name.empty() and
       is_light_name_and_region_name_unique(lights, reference_name) and
       is_name_unique(regions, reference_name)) {
      return std::string{reference_name};
   }

   uint64 max_index = 0;
   const std::string_view base_name =
      create_base_name<light>(reference_name, "LightRegion");

   for (const light& light : lights) {
      if (string::istarts_with(light.name, base_name)) {
         const std::string_view name_index =
            std::string_view{light.name}.substr(base_name.size());

         uint64 index = 0;

         if (std::from_chars_result result =
                std::from_chars(name_index.data(),
                                name_index.data() + name_index.size(), index);
             result.ec == std::errc{} and
             result.ptr == name_index.data() + name_index.size()) {
            max_index = std::max(max_index, index);
         }
      }

      if (is_region_light(light) and
          string::istarts_with(light.region_name, base_name)) {
         const std::string_view name_index =
            std::string_view{light.region_name}.substr(base_name.size());

         uint64 index = 0;

         if (std::from_chars_result result =
                std::from_chars(name_index.data(),
                                name_index.data() + name_index.size(), index);
             result.ec == std::errc{} and
             result.ptr == name_index.data() + name_index.size()) {
            max_index = std::max(max_index, index);
         }
      }
   }

   for (const region& region : regions) {
      if (string::istarts_with(region.name, base_name)) {
         const std::string_view name_index =
            std::string_view{region.name}.substr(base_name.size());

         uint64 index = 0;

         if (std::from_chars_result result =
                std::from_chars(name_index.data(),
                                name_index.data() + name_index.size(), index);
             result.ec == std::errc{} and
             result.ptr == name_index.data() + name_index.size()) {
            max_index = std::max(max_index, index);
         }
      }
   }

   return fmt::format("{}{}", base_name, max_index + 1);
}

bool is_directional_light(const light& light) noexcept
{
   switch (light.light_type) {
   case light_type::directional:
   case light_type::directional_region_box:
   case light_type::directional_region_sphere:
   case light_type::directional_region_cylinder:
      return true;
   default:
      return false;
   }
}

bool is_region_light(const light& light) noexcept
{
   switch (light.light_type) {
   case light_type::directional_region_box:
   case light_type::directional_region_sphere:
   case light_type::directional_region_cylinder:
      return true;
   default:
      return false;
   }
}

auto find_closest_node(const float3& point, const path& path) noexcept -> clostest_node_result
{
   if (path.nodes.size() <= 1) return {0, not path.nodes.empty()};

   std::size_t closest_index = 0;
   float closest_distance = FLT_MAX;

   for (std::size_t i = 0; i < path.nodes.size(); ++i) {
      const float point_distance = distance(path.nodes[i].position, point);

      if (point_distance < closest_distance) {
         closest_distance = point_distance;
         closest_index = i;
      }
   }

   if (closest_index > 0 and (closest_index + 1) < path.nodes.size()) {

      const float3 back_position = path.nodes[closest_index - 1].position;
      const float3 forward_position = path.nodes[closest_index + 1].position;

      const float back_distance = distance(point, back_position);
      const float forward_distance = distance(point, forward_position);

      return {closest_index, forward_distance <= back_distance};
   }

   return {closest_index, true};
}

auto find_closest_edge(const float2& point, const sector& sector) noexcept -> std::size_t
{
   if (sector.points.size() <= 1) return 0;

   std::size_t nearest = 0;
   float nearest_distance = FLT_MAX;

   for (std::size_t i = 0; i < sector.points.size(); ++i) {
      float2 a = sector.points[i];
      float2 b = sector.points[(i + 1) % sector.points.size()];

      float distance = line_distance_lnorm(point, a, b);

      if (distance < nearest_distance) {
         nearest = i;
         nearest_distance = distance;
      }
   }

   return nearest;
}

}