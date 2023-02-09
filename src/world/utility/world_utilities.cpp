
#include "world_utilities.hpp"
#include "math/vector_funcs.hpp"
#include "utility/string_ops.hpp"

#include <cstring>

#include <fmt/core.h>

#include <absl/container/flat_hash_set.h>

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
}

template<typename T>
auto create_base_name(const std::string_view name,
                      const std::string_view base_name = missing_name_base<T>())
   -> std::string_view
{
   std::string_view stripped_name = utility::string::trim_trailing_digits(name);

   if (stripped_name.empty()) {
      stripped_name = base_name;
   }

   return stripped_name;
}

template<typename T>
auto create_unique_name_impl(const std::vector<T>& entities,
                             const std::string_view reference_name) -> std::string
{
   if (reference_name.empty()) return "";

   absl::flat_hash_set<std::string_view> used_names;

   used_names.reserve(entities.size());

   for (const auto& entity : entities) used_names.emplace(entity.name);

   if (not used_names.contains(reference_name)) {
      return std::string{reference_name};
   }

   const std::string_view base_name = create_base_name<T>(reference_name);

   uint32 index = 0;

   while (true) {
      std::string candidate_name = fmt::format("{}{}", base_name, index++);

      if (not used_names.contains(candidate_name)) return candidate_name;
   };
}

}

auto create_unique_name(const std::vector<object>& entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::vector<light>& entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::vector<path>& entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::vector<region>& entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::vector<sector>& entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::vector<portal>& entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::vector<hintnode>& entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::vector<barrier>& entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::vector<planning_hub>& entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::vector<planning_connection>& entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_name(const std::vector<boundary>& entities,
                        const std::string_view reference_name) -> std::string
{
   return create_unique_name_impl(entities, reference_name);
}

auto create_unique_light_region_name(const std::vector<light>& lights,
                                     const std::vector<region>& regions,
                                     const std::string_view reference_name) -> std::string
{
   absl::flat_hash_set<std::string_view> used_names;

   used_names.reserve(lights.size());

   for (const auto& light : lights) {
      used_names.emplace(light.name);
      if (is_region_light(light)) used_names.emplace(light.region_name);
   }
   for (const auto& region : regions) used_names.emplace(region.name);

   if (not reference_name.empty() and not used_names.contains(reference_name)) {
      return std::string{reference_name};
   }

   const std::string_view base_name =
      create_base_name<light>(reference_name, "LightRegion");

   uint32 index = 0;

   while (true) {
      std::string candidate_name = fmt::format("{}{}", base_name, index++);

      if (not used_names.contains(candidate_name)) return candidate_name;
   };
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

}