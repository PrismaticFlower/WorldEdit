#include "delete_layer.hpp"
#include "utility/string_icompare.hpp"
#include "world/object_class_library.hpp"

#include "world/blocks/utility/bounding_box.hpp"

#include <vector>

#include <fmt/core.h>

using we::string::iequals;

namespace we::edits {

namespace {

struct unlinked_object_property {
   uint32 object_index = 0;
   uint32 property_index = 0;
   std::string value;
};

struct unlinked_path_property {
   uint32 path_index = 0;
   uint32 property_index = 0;
   world::path::property value;
};

struct unlinked_sector_entry {
   uint32 sector_index = 0;
   uint32 entry_index = 0;
   std::string entry;
};

struct unlinked_animation_group {
   uint32 group_index = 0;
   uint32 entry_index = 0;
   world::animation_group::entry entry;
};

struct unlinked_animation_hierarchy {
   uint32 hierarchy_index = 0;
   uint32 object_index = 0;
   std::string object;
};

struct unlinked_animation_hierarchy_root {
   uint32 hierarchy_index = 0;
   world::animation_hierarchy hierarchy;
};

struct unlinked_hintnode {
   uint32 hintnode_index = 0;
   std::string value;
};

struct unlinked_properties {
   std::vector<unlinked_object_property> object_properties;
   std::vector<unlinked_path_property> path_properties;
   std::vector<unlinked_sector_entry> sector_entries;
   std::vector<unlinked_animation_group> animation_groups;
   std::vector<unlinked_animation_hierarchy> animation_hierarchies;
   std::vector<unlinked_animation_hierarchy_root> animation_hierarchy_roots;
   std::vector<unlinked_hintnode> hintnodes;

   std::optional<std::string> global_light_1;
   std::optional<std::string> global_light_2;
};

template<typename T>
struct remap_entry {
   int index = 0;
};

struct remap_entry_game_mode {
   int game_mode_index = 0;
   int layer_entry_index = 0;
};

struct remap_entry_common_layer {
   int index = 0;
};

struct remap_entry_block {
   int index = 0;
};

template<typename T>
struct delete_entry {
   int index = 0;
   T entity;
};

struct delete_entry_req {
   int list_index = 0;
   int entry_index = 0;
   std::string entry;
};

struct delete_entry_game_mode {
   int game_mode_index = 0;
   int layer_entry_index = 0;
};

struct delete_entry_req_game_mode {
   int game_mode_index = 0;
   int list_index = 0;
   int entry_index = 0;
   std::string entry;
};

template<typename T>
struct delete_entry_block {
   uint32 index = 0;

   bool hidden = false;
   int8 layer = 0;
   T description;
   world::id<T> id;
};

struct delete_layer_data {
   int index;
   world::layer_description layer;

   unlinked_properties unlinked_properties;

   std::vector<remap_entry<world::object>> remap_objects;
   std::vector<remap_entry<world::light>> remap_lights;
   std::vector<remap_entry<world::path>> remap_paths;
   std::vector<remap_entry<world::region>> remap_regions;
   std::vector<remap_entry<world::hintnode>> remap_hintnodes;
   std::vector<remap_entry_game_mode> remap_game_modes;
   std::vector<remap_entry_common_layer> remap_common_layers;

   std::vector<remap_entry_block> remap_blocks_boxes;
   std::vector<remap_entry_block> remap_blocks_ramps;
   std::vector<remap_entry_block> remap_blocks_quads;
   std::vector<remap_entry_block> remap_blocks_custom;
   std::vector<remap_entry_block> remap_blocks_hemispheres;
   std::vector<remap_entry_block> remap_blocks_pyramids;
   std::vector<remap_entry_block> remap_blocks_terrain_cut_boxes;

   std::vector<delete_entry<world::object>> delete_objects;
   std::vector<delete_entry<world::light>> delete_lights;
   std::vector<delete_entry<world::path>> delete_paths;
   std::vector<delete_entry<world::region>> delete_regions;
   std::vector<delete_entry<world::hintnode>> delete_hintnodes;
   std::vector<delete_entry_req> delete_requirements;
   std::vector<delete_entry_game_mode> delete_game_mode_entries;
   std::vector<delete_entry_req_game_mode> delete_game_mode_requirements;
   std::vector<int> delete_common_layers;

   std::vector<delete_entry_block<world::block_description_box>> delete_blocks_boxes;
   std::vector<delete_entry_block<world::block_description_ramp>> delete_blocks_ramps;
   std::vector<delete_entry_block<world::block_description_quad>> delete_blocks_quads;
   std::vector<delete_entry_block<world::block_description_custom>> delete_blocks_custom;
   std::vector<delete_entry_block<world::block_description_hemisphere>> delete_blocks_hemispheres;
   std::vector<delete_entry_block<world::block_description_pyramid>> delete_blocks_pyramids;
   std::vector<delete_entry_block<world::block_description_terrain_cut_box>> delete_blocks_terrain_cut_boxes;
};

auto make_unlink_properties(int layer_index, const world::world& world) -> unlinked_properties
{
   unlinked_properties unlinked_properties;

   for (const world::object& object : world.objects) {
      if (object.layer != layer_index) continue;
      if (object.name.empty()) continue;

      for (uint32 other_object_index = 0;
           other_object_index < world.objects.size(); ++other_object_index) {
         const world::object& other_object = world.objects[other_object_index];

         if (other_object.layer == layer_index) continue;

         for (uint32 property_index = 0;
              property_index < other_object.instance_properties.size();
              ++property_index) {
            const world::instance_property& prop =
               other_object.instance_properties[property_index];

            if (iequals(prop.key, "ControlZone") and iequals(prop.value, object.name)) {
               unlinked_properties.object_properties.emplace_back(other_object_index,
                                                                  property_index);
            }
         }
      }

      for (uint32 path_index = 0; path_index < world.paths.size(); ++path_index) {
         const world::path& path = world.paths[path_index];

         if (path.layer == layer_index) continue;

         uint32 delete_offset = 0;

         for (uint32 property_index = 0;
              property_index < path.properties.size(); ++property_index) {
            const auto& [key, value] = path.properties[property_index];

            if (iequals(key, "EnableObject") and iequals(value, object.name)) {
               unlinked_properties.path_properties.emplace_back(path_index,
                                                                property_index -
                                                                   delete_offset);

               delete_offset += 1;
            }
         }
      }

      for (uint32 sector_index = 0; sector_index < world.sectors.size(); ++sector_index) {
         const world::sector& sector = world.sectors[sector_index];

         uint32 delete_offset = 0;

         for (uint32 entry_index = 0; entry_index < sector.objects.size();
              ++entry_index) {
            if (iequals(sector.objects[entry_index], object.name)) {
               unlinked_properties.sector_entries.emplace_back(sector_index,
                                                               entry_index - delete_offset);

               delete_offset += 1;
            }
         }
      }

      for (uint32 hintnode_index = 0; hintnode_index < world.hintnodes.size();
           ++hintnode_index) {
         const world::hintnode& hintnode = world.hintnodes[hintnode_index];

         if (hintnode.layer == layer_index) continue;

         if (iequals(hintnode.command_post, object.name)) {
            unlinked_properties.hintnodes.emplace_back(hintnode_index);
         }
      }

      for (uint32 group_index = 0; group_index < world.animation_groups.size();
           ++group_index) {
         const world::animation_group& group = world.animation_groups[group_index];

         uint32 delete_offset = 0;

         for (uint32 entry_index = 0; entry_index < group.entries.size(); ++entry_index) {
            if (iequals(group.entries[entry_index].object, object.name)) {
               unlinked_properties.animation_groups.emplace_back(group_index,
                                                                 entry_index - delete_offset);

               delete_offset += 1;
            }
         }
      }

      uint32 hierarchy_delete_offset = 0;

      for (uint32 hierarchy_index = 0;
           hierarchy_index < world.animation_hierarchies.size(); ++hierarchy_index) {
         const world::animation_hierarchy& hierarchy =
            world.animation_hierarchies[hierarchy_index];

         uint32 delete_offset = 0;

         for (uint32 entry_index = 0; entry_index < hierarchy.objects.size();
              ++entry_index) {
            if (iequals(hierarchy.objects[entry_index], object.name)) {
               unlinked_properties.animation_hierarchies
                  .emplace_back(hierarchy_index, entry_index - delete_offset);

               delete_offset += 1;
            }
         }

         if (iequals(hierarchy.root_object, object.name)) {
            unlinked_properties.animation_hierarchy_roots.emplace_back(
               hierarchy_index - hierarchy_delete_offset);

            hierarchy_delete_offset += 1;
         }
      }
   }

   for (const world::path& path : world.paths) {
      if (path.layer != layer_index) continue;
      if (path.name.empty()) continue;

      for (uint32 object_index = 0; object_index < world.objects.size(); ++object_index) {
         const world::object& object = world.objects[object_index];

         for (uint32 property_index = 0;
              property_index < object.instance_properties.size(); ++property_index) {
            const auto& [key, value] = object.instance_properties[property_index];

            if (iequals(key, "SpawnPath") or iequals(key, "AllyPath") or
                iequals(key, "TurretPath")) {
               if (iequals(value, path.name)) {
                  unlinked_properties.object_properties.emplace_back(object_index,
                                                                     property_index);
               }
            }
         }
      }
   }

   for (const world::region& region : world.regions) {
      if (region.layer != layer_index) continue;
      if (region.description.empty()) continue;

      for (uint32 object_index = 0; object_index < world.objects.size(); ++object_index) {
         const world::object& object = world.objects[object_index];

         for (uint32 property_index = 0;
              property_index < object.instance_properties.size(); ++property_index) {
            const auto& [key, value] = object.instance_properties[property_index];

            if (iequals(key, "CaptureRegion") or
                iequals(key, "ControlRegion") or iequals(key, "EffectRegion") or
                iequals(key, "KillRegion") or iequals(key, "SpawnRegion")) {
               if (iequals(value, region.description)) {
                  unlinked_properties.object_properties.emplace_back(object_index,
                                                                     property_index);
               }
            }
         }
      }
   }

   for (const world::light& light : world.lights) {
      if (light.layer != layer_index) continue;

      if (iequals(light.name, world.global_lights.global_light_1)) {
         unlinked_properties.global_light_1 = "";
      }

      if (iequals(light.name, world.global_lights.global_light_2)) {
         unlinked_properties.global_light_2 = "";
      }
   }

   return unlinked_properties;
}

template<typename T>
auto make_remap_entries(int layer_index, const pinned_vector<T>& entities)
   -> std::vector<remap_entry<T>>
{
   std::size_t count = 0;

   for (const auto& entity : entities) {
      if (entity.layer > layer_index) count += 1;
   }

   std::vector<remap_entry<T>> entries;
   entries.reserve(count);

   for (int i = 0; i < entities.size(); ++i) {
      if (entities[i].layer > layer_index) {
         entries.emplace_back(i);
      }
   }

   return entries;
}

auto make_game_mode_remap_entries(int layer_index,
                                  const std::span<const world::game_mode_description> game_modes)
   -> std::vector<remap_entry_game_mode>
{
   std::size_t count = 0;

   for (const auto& game_mode : game_modes) {
      for (const auto& layer : game_mode.layers) {
         if (layer > layer_index) count += 1;
      }
   }

   std::vector<remap_entry_game_mode> entries;
   entries.reserve(count);

   for (int game_mode_index = 0; game_mode_index < game_modes.size(); ++game_mode_index) {
      for (int game_mode_layer_index = 0;
           game_mode_layer_index < game_modes[game_mode_index].layers.size();
           ++game_mode_layer_index) {
         if (game_modes[game_mode_index].layers[game_mode_layer_index] > layer_index) {
            entries.emplace_back(game_mode_index, game_mode_layer_index);
         }
      }
   }

   return entries;
}

auto make_common_layers_remap_entries(int layer_index,
                                      const std::span<const int> common_layers)
   -> std::vector<remap_entry_common_layer>
{
   std::size_t count = 0;

   for (const int& layer : common_layers) {
      if (layer > layer_index) count += 1;
   }

   std::vector<remap_entry_common_layer> entries;
   entries.reserve(count);

   for (int common_layer_index = 0;
        common_layer_index < std::ssize(common_layers); ++common_layer_index) {
      if (common_layers[common_layer_index] > layer_index) {
         entries.emplace_back(common_layer_index);
      }
   }

   return entries;
}

auto make_block_remap_entries(int layer_index, const std::span<const int8> block_layers)
   -> std::vector<remap_entry_block>
{
   std::size_t count = 0;

   for (const int8 block_layer : block_layers) {
      if (block_layer > layer_index) count += 1;
   }

   std::vector<remap_entry_block> entries;
   entries.reserve(count);

   for (int i = 0; i < block_layers.size(); ++i) {
      if (block_layers[i] > layer_index) {
         entries.emplace_back(i);
      }
   }

   return entries;
}

void apply_unlinked_entities(world::world& world, unlinked_properties& unlinked_properties)
{
   for (unlinked_object_property& unlinked : unlinked_properties.object_properties) {
      std::swap(world.objects[unlinked.object_index]
                   .instance_properties[unlinked.property_index]
                   .value,
                unlinked.value);
   }

   for (unlinked_path_property& unlinked : unlinked_properties.path_properties) {
      std::vector<world::path::property>& properties =
         world.paths[unlinked.path_index].properties;

      std::swap(properties[unlinked.property_index], unlinked.value);

      properties.erase(properties.begin() + unlinked.property_index);
   }

   for (unlinked_sector_entry& unlinked : unlinked_properties.sector_entries) {
      std::vector<std::string>& objects = world.sectors[unlinked.sector_index].objects;

      std::swap(objects[unlinked.entry_index], unlinked.entry);

      objects.erase(objects.begin() + unlinked.entry_index);
   }

   for (unlinked_hintnode& unlinked : unlinked_properties.hintnodes) {
      std::swap(world.hintnodes[unlinked.hintnode_index].command_post, unlinked.value);
   }

   for (unlinked_animation_group& unlinked : unlinked_properties.animation_groups) {
      std::vector<world::animation_group::entry>& entries =
         world.animation_groups[unlinked.group_index].entries;

      std::swap(entries[unlinked.entry_index], unlinked.entry);

      entries.erase(entries.begin() + unlinked.entry_index);
   }

   for (unlinked_animation_hierarchy& unlinked :
        unlinked_properties.animation_hierarchies) {
      std::vector<std::string>& objects =
         world.animation_hierarchies[unlinked.hierarchy_index].objects;

      std::swap(objects[unlinked.object_index], unlinked.object);

      objects.erase(objects.begin() + unlinked.object_index);
   }

   for (unlinked_animation_hierarchy_root& unlinked :
        unlinked_properties.animation_hierarchy_roots) {
      std::swap(world.animation_hierarchies[unlinked.hierarchy_index],
                unlinked.hierarchy);

      world.animation_hierarchies.erase(world.animation_hierarchies.begin() +
                                        unlinked.hierarchy_index);
   }

   if (unlinked_properties.global_light_1) {
      std::swap(*unlinked_properties.global_light_1, world.global_lights.global_light_1);
   }

   if (unlinked_properties.global_light_2) {
      std::swap(*unlinked_properties.global_light_2, world.global_lights.global_light_2);
   }
}

void revert_unlinked_entities(world::world& world, unlinked_properties& unlinked_properties)
{
   for (unlinked_object_property& unlinked : unlinked_properties.object_properties) {
      std::swap(world.objects[unlinked.object_index]
                   .instance_properties[unlinked.property_index]
                   .value,
                unlinked.value);
   }

   for (std::ptrdiff_t i = (std::ssize(unlinked_properties.path_properties) - 1);
        i >= 0; --i) {
      unlinked_path_property& unlinked = unlinked_properties.path_properties[i];

      std::vector<world::path::property>& properties =
         world.paths[unlinked.path_index].properties;

      properties.emplace(properties.begin() + unlinked.property_index,
                         std::move(unlinked.value));
   }

   for (std::ptrdiff_t i = (std::ssize(unlinked_properties.sector_entries) - 1);
        i >= 0; --i) {
      unlinked_sector_entry& unlinked = unlinked_properties.sector_entries[i];

      std::vector<std::string>& objects = world.sectors[unlinked.sector_index].objects;

      objects.insert(objects.begin() + unlinked.entry_index, std::move(unlinked.entry));
   }

   for (std::ptrdiff_t i = (std::ssize(unlinked_properties.hintnodes) - 1);
        i >= 0; --i) {
      unlinked_hintnode& unlinked = unlinked_properties.hintnodes[i];

      std::swap(world.hintnodes[unlinked.hintnode_index].command_post, unlinked.value);
   }

   for (std::ptrdiff_t i = (std::ssize(unlinked_properties.animation_groups) - 1);
        i >= 0; --i) {
      unlinked_animation_group& unlinked = unlinked_properties.animation_groups[i];

      std::vector<world::animation_group::entry>& entries =
         world.animation_groups[unlinked.group_index].entries;

      entries.insert(entries.begin() + unlinked.entry_index, std::move(unlinked.entry));
   }

   for (std::ptrdiff_t i = (std::ssize(unlinked_properties.animation_hierarchies) - 1);
        i >= 0; --i) {
      unlinked_animation_hierarchy& unlinked =
         unlinked_properties.animation_hierarchies[i];

      std::vector<std::string>& objects =
         world.animation_hierarchies[unlinked.hierarchy_index].objects;

      objects.insert(objects.begin() + unlinked.object_index,
                     std::move(unlinked.object));
   }

   for (std::ptrdiff_t i =
           (std::ssize(unlinked_properties.animation_hierarchy_roots) - 1);
        i >= 0; --i) {
      unlinked_animation_hierarchy_root& unlinked =
         unlinked_properties.animation_hierarchy_roots[i];

      world.animation_hierarchies.insert(world.animation_hierarchies.begin() +
                                            unlinked.hierarchy_index,
                                         std::move(unlinked.hierarchy));
   }

   if (unlinked_properties.global_light_1) {
      std::swap(*unlinked_properties.global_light_1, world.global_lights.global_light_1);
   }

   if (unlinked_properties.global_light_2) {
      std::swap(*unlinked_properties.global_light_2, world.global_lights.global_light_2);
   }
}

template<typename T>
void apply_remap_entries(pinned_vector<T>& entities,
                         std::span<const remap_entry<std::type_identity_t<T>>> entries)
{
   for (const auto& [index] : entries) entities[index].layer -= 1;
}

template<typename T>
void revert_remap_entries(pinned_vector<T>& entities,
                          std::span<const remap_entry<std::type_identity_t<T>>> entries)
{
   for (const auto& [index] : entries) entities[index].layer += 1;
}

void apply_remap_entries(std::vector<world::game_mode_description>& game_modes,
                         std::span<const remap_entry_game_mode> entries)
{
   for (const auto& [game_mode_index, layer_entry_index] : entries) {
      game_modes[game_mode_index].layers[layer_entry_index] -= 1;
   }
}

void revert_remap_entries(std::vector<world::game_mode_description>& game_modes,
                          std::span<const remap_entry_game_mode> entries)
{
   for (const auto& [game_mode_index, layer_entry_index] : entries) {
      game_modes[game_mode_index].layers[layer_entry_index] += 1;
   }
}

void apply_remap_entries(std::vector<int>& common_layers,
                         std::span<const remap_entry_common_layer> entries)
{
   for (const auto& [index] : entries) common_layers[index] -= 1;
}

void revert_remap_entries(std::vector<int>& common_layers,
                          std::span<const remap_entry_common_layer> entries)
{
   for (const auto& [index] : entries) common_layers[index] += 1;
}

void apply_remap_entries(std::span<int8> block_layers,
                         std::span<const remap_entry_block> entries)
{
   for (const auto& [index] : entries) block_layers[index] -= 1;
}

void revert_remap_entries(std::span<int8> block_layers,
                          std::span<const remap_entry_block> entries)
{
   for (const auto& [index] : entries) block_layers[index] += 1;
}

template<typename T>
auto make_delete_entries(int layer_index, const pinned_vector<T>& entities)
   -> std::vector<delete_entry<T>>
{
   std::size_t count = 0;

   for (const auto& entity : entities) {
      if (entity.layer == layer_index) count += 1;
   }

   std::vector<delete_entry<T>> entries;
   entries.reserve(count);

   int delete_offset = 0;

   for (int i = 0; i < entities.size(); ++i) {
      if (entities[i].layer == layer_index) {
         entries.emplace_back(i - delete_offset, entities[i]);
         delete_offset += 1;
      }
   }

   return entries;
}

auto make_requirements_delete_entries(const std::string_view layer_file_name,
                                      const std::span<const world::requirement_list> requirements)
   -> std::vector<delete_entry_req>
{
   std::size_t count = 0;

   for (const auto& list : requirements) {
      if (not string::iequals(list.file_type, "world")) continue;

      for (const auto& entry : list.entries) {
         if (string::iequals(entry, layer_file_name)) count += 1;
      }
   }

   std::vector<delete_entry_req> entries;
   entries.reserve(count);

   for (int list_index = 0; list_index < requirements.size(); ++list_index) {
      const auto& list = requirements[list_index];

      if (not string::iequals(list.file_type, "world")) continue;

      int delete_offset = 0;

      for (int i = 0; i < list.entries.size(); ++i) {
         if (string::iequals(list.entries[i], layer_file_name)) {
            entries.emplace_back(list_index, i - delete_offset, list.entries[i]);
            delete_offset += 1;
         }
      }
   }

   return entries;
}

auto make_game_mode_delete_entries(int layer_index,
                                   const std::span<const world::game_mode_description> game_modes)
   -> std::vector<delete_entry_game_mode>
{
   std::size_t count = 0;

   for (const auto& game_mode : game_modes) {
      for (const auto& layer : game_mode.layers) {
         if (layer == layer_index) count += 1;
      }
   }

   std::vector<delete_entry_game_mode> entries;
   entries.reserve(count);

   for (int game_mode_index = 0; game_mode_index < game_modes.size(); ++game_mode_index) {
      int delete_offset = 0;

      for (int i = 0; i < game_modes[game_mode_index].layers.size(); ++i) {
         if (game_modes[game_mode_index].layers[i] == layer_index) {
            entries.emplace_back(game_mode_index, i - delete_offset);
            delete_offset += 1;
         }
      }
   }

   return entries;
}

auto makee_game_mode_requirements_delete_entries(
   const std::string_view layer_file_name,
   const std::span<const world::game_mode_description> game_modes)
   -> std::vector<delete_entry_req_game_mode>
{
   std::size_t count = 0;

   for (const auto& game_mode : game_modes) {
      for (const auto& list : game_mode.requirements) {
         if (not string::iequals(list.file_type, "world")) continue;

         for (const auto& entry : list.entries) {
            if (string::iequals(entry, layer_file_name)) count += 1;
         }
      }
   }

   std::vector<delete_entry_req_game_mode> entries;
   entries.reserve(count);

   for (int game_mode_index = 0; game_mode_index < game_modes.size(); ++game_mode_index) {
      const auto& game_mode = game_modes[game_mode_index];

      for (int list_index = 0; list_index < game_mode.requirements.size(); ++list_index) {
         const auto& list = game_mode.requirements[list_index];

         if (not string::iequals(list.file_type, "world")) continue;

         int delete_offset = 0;

         for (int i = 0; i < list.entries.size(); ++i) {
            if (string::iequals(list.entries[i], layer_file_name)) {
               entries.emplace_back(game_mode_index, list_index,
                                    i - delete_offset, list.entries[i]);
               delete_offset += 1;
            }
         }
      }
   }

   return entries;
}

auto makee_common_layers_delete_entries(const int layer_index,
                                        const std::span<const int> common_layers)
   -> std::vector<int>
{
   std::size_t count = 0;

   for (const int other_layer : common_layers) {
      if (other_layer == layer_index) count += 1;
   }

   std::vector<int> entries;
   entries.reserve(count);

   int delete_offset = 0;

   for (int i = 0; i < common_layers.size(); ++i) {
      if (common_layers[i] != layer_index) continue;

      entries.emplace_back(i - delete_offset);
      delete_offset += 1;
   }

   return entries;
}

template<typename Blocks>
auto make_delete_entries(int layer_index, const Blocks& blocks)
   -> std::vector<delete_entry_block<typename decltype(Blocks::description)::value_type>>
{
   using block_type = decltype(Blocks::description)::value_type;

   std::size_t count = 0;

   for (const int8 block_layer : blocks.layer) {
      if (block_layer == layer_index) count += 1;
   }

   std::vector<delete_entry_block<block_type>> entries;
   entries.reserve(count);

   uint32 delete_offset = 0;

   for (uint32 i = 0; i < blocks.size(); ++i) {
      if (blocks.layer[i] == layer_index) {
         entries.push_back({
            .index = i - delete_offset,
            .hidden = blocks.hidden[i],
            .layer = blocks.layer[i],
            .description = blocks.description[i],
            .id = blocks.ids[i],
         });

         delete_offset += 1;
      }
   }

   return entries;
}

template<typename T>
void apply_delete_entries(pinned_vector<T>& entities,
                          std::span<const delete_entry<std::type_identity_t<T>>> entries)
{
   for (const auto& [index, entity] : entries) {
      entities.erase(entities.begin() + index);
   }
}

template<typename T>
void revert_delete_entries(pinned_vector<T>& entities,
                           std::span<const delete_entry<std::type_identity_t<T>>> entries)
{
   for (std::ptrdiff_t i = (std::ssize(entries) - 1); i >= 0; --i) {
      const auto& [index, entity] = entries[i];

      entities.insert(entities.begin() + index, entity);
   }
}

void apply_delete_entries(pinned_vector<world::object>& entities,
                          std::span<const delete_entry<world::object>> entries,
                          world::object_class_library& object_class_library)
{
   for (const auto& [index, entity] : entries) {
      object_class_library.free(entities[index].class_handle);

      entities.erase(entities.begin() + index);
   }
}

void revert_delete_entries(pinned_vector<world::object>& entities,
                           std::span<const delete_entry<world::object>> entries,
                           world::object_class_library& object_class_library)
{
   for (std::ptrdiff_t i = (std::ssize(entries) - 1); i >= 0; --i) {
      const auto& [index, entity] = entries[i];

      entities.insert(entities.begin() + index, entity);

      entities[index].class_handle =
         object_class_library.acquire(entities[index].class_name);
   }
}

void apply_delete_entries(std::vector<world::requirement_list>& requirements,
                          std::span<const delete_entry_req> entries)
{
   for (const auto& [list_index, entry_index, entry] : entries) {
      auto& list = requirements[list_index];

      list.entries.erase(list.entries.begin() + entry_index);
   }
}

void revert_delete_entries(std::vector<world::requirement_list>& requirements,
                           std::span<const delete_entry_req> entries)
{
   for (std::ptrdiff_t i = (std::ssize(entries) - 1); i >= 0; --i) {
      const auto& [list_index, entry_index, entry] = entries[i];
      auto& list = requirements[list_index];

      list.entries.emplace(list.entries.begin() + entry_index, entry);
   }
}

void apply_delete_entries(std::vector<world::game_mode_description>& game_modes,
                          std::span<const delete_entry_game_mode> entries)
{
   for (const auto& [game_mode_index, layer_entry_index] : entries) {
      auto& layers = game_modes[game_mode_index].layers;

      layers.erase(layers.begin() + layer_entry_index);
   }
}

void revert_delete_entries(std::vector<world::game_mode_description>& game_modes,
                           std::span<const delete_entry_game_mode> entries,
                           const int layer_index)
{
   for (std::ptrdiff_t i = (std::ssize(entries) - 1); i >= 0; --i) {
      const auto& [game_mode_index, layer_entry_index] = entries[i];
      auto& layers = game_modes[game_mode_index].layers;

      layers.insert(layers.begin() + layer_entry_index, layer_index);
   }
}

void apply_delete_entries(std::vector<world::game_mode_description>& game_modes,
                          std::span<const delete_entry_req_game_mode> entries)
{
   for (const auto& [game_mode_index, list_index, entry_index, entry] : entries) {
      auto& list = game_modes[game_mode_index].requirements[list_index];

      list.entries.erase(list.entries.begin() + entry_index);
   }
}

void revert_delete_entries(std::vector<world::game_mode_description>& game_modes,
                           std::span<const delete_entry_req_game_mode> entries)
{
   for (std::ptrdiff_t i = (std::ssize(entries) - 1); i >= 0; --i) {
      const auto& [game_mode_index, list_index, entry_index, entry] = entries[i];
      auto& list = game_modes[game_mode_index].requirements[list_index];

      list.entries.emplace(list.entries.begin() + entry_index, entry);
   }
}

void apply_delete_entries(std::vector<int>& common_layers, std::span<const int> entries)
{
   for (const int index : entries) {
      common_layers.erase(common_layers.begin() + index);
   }
}

void revert_delete_entries(std::vector<int>& common_layers, int layer_index,
                           std::span<const int> entries)
{
   for (std::ptrdiff_t i = (std::ssize(entries) - 1); i >= 0; --i) {
      common_layers.insert(common_layers.begin() + entries[i], layer_index);
   }
}

template<typename Blocks>
void apply_delete_entries(
   Blocks& blocks,
   std::span<const delete_entry_block<typename decltype(Blocks::description)::value_type>> entries)
{
   using block_type = decltype(Blocks::description)::value_type;

   for (const delete_entry_block<block_type>& entry : entries) {
      blocks.bbox.min_x.erase(blocks.bbox.min_x.begin() + entry.index);
      blocks.bbox.min_y.erase(blocks.bbox.min_y.begin() + entry.index);
      blocks.bbox.min_z.erase(blocks.bbox.min_z.begin() + entry.index);
      blocks.bbox.max_x.erase(blocks.bbox.max_x.begin() + entry.index);
      blocks.bbox.max_y.erase(blocks.bbox.max_y.begin() + entry.index);
      blocks.bbox.max_z.erase(blocks.bbox.max_z.begin() + entry.index);

      blocks.hidden.erase(blocks.hidden.begin() + entry.index);
      blocks.layer.erase(blocks.layer.begin() + entry.index);
      blocks.description.erase(blocks.description.begin() + entry.index);
      blocks.ids.erase(blocks.ids.begin() + entry.index);

      blocks.dirty.remove_index(entry.index);
      blocks.dirty.add({entry.index, static_cast<uint32>(blocks.size())});
   }

   assert(blocks.is_balanced());
}

template<typename Blocks>
void revert_delete_entries(
   Blocks& blocks,
   std::span<const delete_entry_block<typename decltype(Blocks::description)::value_type>> entries)
{
   using block_type = decltype(Blocks::description)::value_type;

   for (std::ptrdiff_t i = (std::ssize(entries) - 1); i >= 0; --i) {
      const delete_entry_block<block_type>& entry = entries[i];

      const math::bounding_box bbox = world::get_bounding_box(entry.description);

      blocks.bbox.min_x.insert(blocks.bbox.min_x.begin() + entry.index, bbox.min.x);
      blocks.bbox.min_y.insert(blocks.bbox.min_y.begin() + entry.index, bbox.min.y);
      blocks.bbox.min_z.insert(blocks.bbox.min_z.begin() + entry.index, bbox.min.z);
      blocks.bbox.max_x.insert(blocks.bbox.max_x.begin() + entry.index, bbox.max.x);
      blocks.bbox.max_y.insert(blocks.bbox.max_y.begin() + entry.index, bbox.max.y);
      blocks.bbox.max_z.insert(blocks.bbox.max_z.begin() + entry.index, bbox.max.z);

      blocks.hidden.insert(blocks.hidden.begin() + entry.index, entry.hidden);
      blocks.layer.insert(blocks.layer.begin() + entry.index, entry.layer);
      blocks.description.insert(blocks.description.begin() + entry.index,
                                entry.description);
      blocks.ids.insert(blocks.ids.begin() + entry.index, entry.id);

      blocks.dirty.add({entry.index, static_cast<uint32>(blocks.size())});
   }

   assert(blocks.is_balanced());
}

void apply_delete_entries(
   world::blocks& blocks,
   std::span<const delete_entry_block<world::block_description_custom>> entries)
{
   for (const delete_entry_block<world::block_description_custom>& entry : entries) {
      blocks.custom_meshes.remove(blocks.custom.mesh[entry.index]);

      blocks.custom.bbox.min_x.erase(blocks.custom.bbox.min_x.begin() + entry.index);
      blocks.custom.bbox.min_y.erase(blocks.custom.bbox.min_y.begin() + entry.index);
      blocks.custom.bbox.min_z.erase(blocks.custom.bbox.min_z.begin() + entry.index);
      blocks.custom.bbox.max_x.erase(blocks.custom.bbox.max_x.begin() + entry.index);
      blocks.custom.bbox.max_y.erase(blocks.custom.bbox.max_y.begin() + entry.index);
      blocks.custom.bbox.max_z.erase(blocks.custom.bbox.max_z.begin() + entry.index);

      blocks.custom.hidden.erase(blocks.custom.hidden.begin() + entry.index);
      blocks.custom.layer.erase(blocks.custom.layer.begin() + entry.index);
      blocks.custom.description.erase(blocks.custom.description.begin() + entry.index);
      blocks.custom.mesh.erase(blocks.custom.mesh.begin() + entry.index);
      blocks.custom.ids.erase(blocks.custom.ids.begin() + entry.index);

      blocks.custom.dirty.remove_index(entry.index);
      blocks.custom.dirty.add({entry.index, static_cast<uint32>(blocks.custom.size())});
   }

   assert(blocks.custom.is_balanced());
}

void revert_delete_entries(
   world::blocks& blocks,
   std::span<const delete_entry_block<world::block_description_custom>> entries)
{
   for (std::ptrdiff_t i = (std::ssize(entries) - 1); i >= 0; --i) {
      const delete_entry_block<world::block_description_custom>& entry = entries[i];

      const math::bounding_box bbox = world::get_bounding_box(entry.description);

      blocks.custom.bbox.min_x.insert(blocks.custom.bbox.min_x.begin() + entry.index,
                                      bbox.min.x);
      blocks.custom.bbox.min_y.insert(blocks.custom.bbox.min_y.begin() + entry.index,
                                      bbox.min.y);
      blocks.custom.bbox.min_z.insert(blocks.custom.bbox.min_z.begin() + entry.index,
                                      bbox.min.z);
      blocks.custom.bbox.max_x.insert(blocks.custom.bbox.max_x.begin() + entry.index,
                                      bbox.max.x);
      blocks.custom.bbox.max_y.insert(blocks.custom.bbox.max_y.begin() + entry.index,
                                      bbox.max.y);
      blocks.custom.bbox.max_z.insert(blocks.custom.bbox.max_z.begin() + entry.index,
                                      bbox.max.z);

      blocks.custom.hidden.insert(blocks.custom.hidden.begin() + entry.index,
                                  entry.hidden);
      blocks.custom.layer.insert(blocks.custom.layer.begin() + entry.index,
                                 entry.layer);
      blocks.custom.description.insert(blocks.custom.description.begin() + entry.index,
                                       entry.description);
      blocks.custom.mesh.insert(blocks.custom.mesh.begin() + entry.index,
                                blocks.custom_meshes.add(
                                   entry.description.mesh_description));
      blocks.custom.ids.insert(blocks.custom.ids.begin() + entry.index, entry.id);

      blocks.custom.dirty.add({entry.index, static_cast<uint32>(blocks.custom.size())});
   }
}

struct delete_layer final : edit<world::edit_context> {
   delete_layer(delete_layer_data data, world::object_class_library& object_class_library)
      : _data{std::move(data)}, _object_class_library{object_class_library}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      world::world& world = context.world;

      world.layer_descriptions.erase(world.layer_descriptions.begin() + _data.index);
      world.deleted_layers.push_back(_data.layer.name);

      apply_unlinked_entities(world, _data.unlinked_properties);

      apply_remap_entries(world.objects, _data.remap_objects);
      apply_remap_entries(world.lights, _data.remap_lights);
      apply_remap_entries(world.paths, _data.remap_paths);
      apply_remap_entries(world.regions, _data.remap_regions);
      apply_remap_entries(world.hintnodes, _data.remap_hintnodes);
      apply_remap_entries(world.game_modes, _data.remap_game_modes);
      apply_remap_entries(world.common_layers, _data.remap_common_layers);

      apply_remap_entries(world.blocks.boxes.layer, _data.remap_blocks_boxes);
      apply_remap_entries(world.blocks.ramps.layer, _data.remap_blocks_ramps);
      apply_remap_entries(world.blocks.quads.layer, _data.remap_blocks_quads);
      apply_remap_entries(world.blocks.custom.layer, _data.remap_blocks_custom);
      apply_remap_entries(world.blocks.hemispheres.layer, _data.remap_blocks_hemispheres);
      apply_remap_entries(world.blocks.pyramids.layer, _data.remap_blocks_pyramids);
      apply_remap_entries(world.blocks.pyramids.layer,
                          _data.remap_blocks_terrain_cut_boxes);

      apply_delete_entries(world.objects, _data.delete_objects, _object_class_library);
      apply_delete_entries(world.lights, _data.delete_lights);
      apply_delete_entries(world.paths, _data.delete_paths);
      apply_delete_entries(world.regions, _data.delete_regions);
      apply_delete_entries(world.hintnodes, _data.delete_hintnodes);
      apply_delete_entries(world.requirements, _data.delete_requirements);
      apply_delete_entries(world.game_modes, _data.delete_game_mode_entries);
      apply_delete_entries(world.game_modes, _data.delete_game_mode_requirements);
      apply_delete_entries(world.common_layers, _data.delete_common_layers);

      apply_delete_entries(world.blocks.boxes, _data.delete_blocks_boxes);
      apply_delete_entries(world.blocks.ramps, _data.delete_blocks_ramps);
      apply_delete_entries(world.blocks.quads, _data.delete_blocks_quads);
      apply_delete_entries(world.blocks, _data.delete_blocks_custom);
      apply_delete_entries(world.blocks.pyramids, _data.delete_blocks_pyramids);
      apply_delete_entries(world.blocks.terrain_cut_boxes,
                           _data.delete_blocks_terrain_cut_boxes);
   }

   void revert(world::edit_context& context) noexcept override
   {
      world::world& world = context.world;

      world.layer_descriptions.insert(world.layer_descriptions.begin() + _data.index,
                                      _data.layer);
      world.deleted_layers.pop_back();

      revert_delete_entries(world.objects, _data.delete_objects, _object_class_library);
      revert_delete_entries(world.lights, _data.delete_lights);
      revert_delete_entries(world.paths, _data.delete_paths);
      revert_delete_entries(world.regions, _data.delete_regions);
      revert_delete_entries(world.hintnodes, _data.delete_hintnodes);
      revert_delete_entries(world.requirements, _data.delete_requirements);
      revert_delete_entries(world.game_modes, _data.delete_game_mode_entries,
                            _data.index);
      revert_delete_entries(world.game_modes, _data.delete_game_mode_requirements);
      revert_delete_entries(world.common_layers, _data.index, _data.delete_common_layers);

      revert_delete_entries(world.blocks.boxes, _data.delete_blocks_boxes);
      revert_delete_entries(world.blocks.ramps, _data.delete_blocks_ramps);
      revert_delete_entries(world.blocks.quads, _data.delete_blocks_quads);
      revert_delete_entries(world.blocks, _data.delete_blocks_custom);
      revert_delete_entries(world.blocks.pyramids, _data.delete_blocks_pyramids);
      revert_delete_entries(world.blocks.terrain_cut_boxes,
                            _data.delete_blocks_terrain_cut_boxes);

      revert_remap_entries(world.objects, _data.remap_objects);
      revert_remap_entries(world.lights, _data.remap_lights);
      revert_remap_entries(world.paths, _data.remap_paths);
      revert_remap_entries(world.regions, _data.remap_regions);
      revert_remap_entries(world.hintnodes, _data.remap_hintnodes);
      revert_remap_entries(world.game_modes, _data.remap_game_modes);
      revert_remap_entries(world.common_layers, _data.remap_common_layers);

      revert_remap_entries(world.blocks.boxes.layer, _data.remap_blocks_boxes);
      revert_remap_entries(world.blocks.ramps.layer, _data.remap_blocks_ramps);
      revert_remap_entries(world.blocks.quads.layer, _data.remap_blocks_quads);
      revert_remap_entries(world.blocks.custom.layer, _data.remap_blocks_custom);
      revert_remap_entries(world.blocks.hemispheres.layer,
                           _data.remap_blocks_hemispheres);
      revert_remap_entries(world.blocks.pyramids.layer, _data.remap_blocks_pyramids);
      revert_remap_entries(world.blocks.pyramids.layer,
                           _data.remap_blocks_terrain_cut_boxes);

      revert_unlinked_entities(world, _data.unlinked_properties);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   delete_layer_data _data;
   world::object_class_library& _object_class_library;
};

}

auto make_delete_layer(int layer_index, const world::world& world,
                       world::object_class_library& object_class_library)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const std::string file_name =
      fmt::format("{}_{}", world.name, world.layer_descriptions[layer_index].name);

   return std::make_unique<delete_layer>(
      delete_layer_data{
         .index = layer_index,
         .layer = world.layer_descriptions[layer_index],

         .unlinked_properties = make_unlink_properties(layer_index, world),

         .remap_objects = make_remap_entries(layer_index, world.objects),
         .remap_lights = make_remap_entries(layer_index, world.lights),
         .remap_paths = make_remap_entries(layer_index, world.paths),
         .remap_regions = make_remap_entries(layer_index, world.regions),
         .remap_hintnodes = make_remap_entries(layer_index, world.hintnodes),
         .remap_game_modes = make_game_mode_remap_entries(layer_index, world.game_modes),
         .remap_common_layers =
            make_common_layers_remap_entries(layer_index, world.common_layers),

         .remap_blocks_boxes =
            make_block_remap_entries(layer_index, world.blocks.boxes.layer),
         .remap_blocks_ramps =
            make_block_remap_entries(layer_index, world.blocks.ramps.layer),
         .remap_blocks_quads =
            make_block_remap_entries(layer_index, world.blocks.quads.layer),
         .remap_blocks_custom =
            make_block_remap_entries(layer_index, world.blocks.custom.layer),
         .remap_blocks_hemispheres =
            make_block_remap_entries(layer_index, world.blocks.hemispheres.layer),
         .remap_blocks_pyramids =
            make_block_remap_entries(layer_index, world.blocks.pyramids.layer),
         .remap_blocks_terrain_cut_boxes =
            make_block_remap_entries(layer_index, world.blocks.terrain_cut_boxes.layer),

         .delete_objects = make_delete_entries(layer_index, world.objects),
         .delete_lights = make_delete_entries(layer_index, world.lights),
         .delete_paths = make_delete_entries(layer_index, world.paths),
         .delete_regions = make_delete_entries(layer_index, world.regions),
         .delete_hintnodes = make_delete_entries(layer_index, world.hintnodes),
         .delete_requirements =
            make_requirements_delete_entries(file_name, world.requirements),
         .delete_game_mode_entries =
            make_game_mode_delete_entries(layer_index, world.game_modes),
         .delete_game_mode_requirements =
            makee_game_mode_requirements_delete_entries(file_name, world.game_modes),
         .delete_common_layers =
            makee_common_layers_delete_entries(layer_index, world.common_layers),

         .delete_blocks_boxes = make_delete_entries(layer_index, world.blocks.boxes),
         .delete_blocks_ramps = make_delete_entries(layer_index, world.blocks.ramps),
         .delete_blocks_quads = make_delete_entries(layer_index, world.blocks.quads),
         .delete_blocks_custom = make_delete_entries(layer_index, world.blocks.custom),
         .delete_blocks_hemispheres =
            make_delete_entries(layer_index, world.blocks.hemispheres),
         .delete_blocks_pyramids = make_delete_entries(layer_index, world.blocks.pyramids),
         .delete_blocks_terrain_cut_boxes =
            make_delete_entries(layer_index, world.blocks.terrain_cut_boxes),
      },
      object_class_library);
}

}