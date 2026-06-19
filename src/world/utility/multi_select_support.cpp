#include "multi_select_support.hpp"

#include <charconv>

namespace we::world {

namespace {

template<typename T>
auto get_first_rename_suffix(const pinned_vector<T>& entities,
                             const std::string_view name_prefix)
{
   uint64 start = 0;

   for (const T& entity : entities) {
      const std::string_view entity_name_trimmed =
         string::trim_trailing_digits(entity.name);

      if (string::iequals(entity_name_trimmed, name_prefix)) {
         std::string_view entity_name_digits = entity.name;

         entity_name_digits.remove_prefix(entity_name_trimmed.size());

         if (entity_name_digits.empty()) continue;

         uint64 entity_name_suffix = 0;

         if (std::from_chars(entity_name_digits.data(),
                             entity_name_digits.data() + entity_name_digits.size(),
                             entity_name_suffix)
                .ec != std::errc{}) {
            continue;
         }

         start = std::max(start, entity_name_suffix);
      }
   }

   return start;
}

}

auto get_rename_suffixes(const multi_select_name_flags flags,
                         const std::string_view name_prefix, const world& world)
   -> multi_select_rename_suffixes
{
   multi_select_rename_suffixes suffixes;

   if (are_flags_set(flags, multi_select_name_flags::object)) {
      suffixes.object_start = get_first_rename_suffix(world.objects, name_prefix);
   }

   if (are_flags_set(flags, multi_select_name_flags::light)) {
      suffixes.light_start = get_first_rename_suffix(world.lights, name_prefix);
   }

   if (are_flags_set(flags, multi_select_name_flags::path)) {
      suffixes.path_start = get_first_rename_suffix(world.paths, name_prefix);
   }

   if (are_flags_set(flags, multi_select_name_flags::region)) {
      suffixes.region_start = get_first_rename_suffix(world.regions, name_prefix);
   }

   if (are_flags_set(flags, multi_select_name_flags::sector)) {
      suffixes.sector_start = get_first_rename_suffix(world.sectors, name_prefix);
   }

   if (are_flags_set(flags, multi_select_name_flags::portal)) {
      suffixes.portal_start = get_first_rename_suffix(world.portals, name_prefix);
   }

   if (are_flags_set(flags, multi_select_name_flags::hintnode)) {
      suffixes.hintnode_start = get_first_rename_suffix(world.hintnodes, name_prefix);
   }

   if (are_flags_set(flags, multi_select_name_flags::barrier)) {
      suffixes.barrier_start = get_first_rename_suffix(world.barriers, name_prefix);
   }

   if (are_flags_set(flags, multi_select_name_flags::planning_hub)) {
      suffixes.planning_hub_start =
         get_first_rename_suffix(world.planning_hubs, name_prefix);
   }

   if (are_flags_set(flags, multi_select_name_flags::planning_connection)) {
      suffixes.planning_connection_start =
         get_first_rename_suffix(world.planning_connections, name_prefix);
   }

   if (are_flags_set(flags, multi_select_name_flags::boundary)) {
      suffixes.boundary_start = get_first_rename_suffix(world.boundaries, name_prefix);
   }

   if (are_flags_set(flags, multi_select_name_flags::measurement)) {
      suffixes.measurement_start =
         get_first_rename_suffix(world.measurements, name_prefix);
   }

   return suffixes;
}

}