#include "delete_layer.hpp"
#include "utility/string_icompare.hpp"
#include "world/object_class_library.hpp"

#include "world/blocks/bounding_box.hpp"

#include <vector>

#include <fmt/core.h>

namespace we::edits {

namespace {

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

struct delete_entry_block {
   uint32 index = 0;

   bool hidden = false;
   int8 layer = 0;
   world::block_description_box description;
   world::id<world::block_description_box> id;
};

struct delete_layer_data {
   int index;
   world::layer_description layer;

   std::vector<remap_entry<world::object>> remap_objects;
   std::vector<remap_entry<world::light>> remap_lights;
   std::vector<remap_entry<world::path>> remap_paths;
   std::vector<remap_entry<world::region>> remap_regions;
   std::vector<remap_entry<world::hintnode>> remap_hintnodes;
   std::vector<remap_entry_game_mode> remap_game_modes;
   std::vector<remap_entry_common_layer> remap_common_layers;

   std::vector<remap_entry_block> remap_blocks_boxes;

   std::vector<delete_entry<world::object>> delete_objects;
   std::vector<delete_entry<world::light>> delete_lights;
   std::vector<delete_entry<world::path>> delete_paths;
   std::vector<delete_entry<world::region>> delete_regions;
   std::vector<delete_entry<world::hintnode>> delete_hintnodes;
   std::vector<delete_entry_req> delete_requirements;
   std::vector<delete_entry_game_mode> delete_game_mode_entries;
   std::vector<delete_entry_req_game_mode> delete_game_mode_requirements;
   std::vector<int> delete_common_layers;

   std::vector<delete_entry_block> delete_blocks_boxes;
};

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

auto make_delete_entries(int layer_index, const world::blocks_boxes& blocks)
   -> std::vector<delete_entry_block>
{
   std::size_t count = 0;

   for (const int8 block_layer : blocks.layer) {
      if (block_layer == layer_index) count += 1;
   }

   std::vector<delete_entry_block> entries;
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

void apply_delete_entries(world::blocks_boxes& boxes,
                          std::span<const delete_entry_block> entries)
{
   for (const delete_entry_block& entry : entries) {
      boxes.bbox.min_x.erase(boxes.bbox.min_x.begin() + entry.index);
      boxes.bbox.min_y.erase(boxes.bbox.min_y.begin() + entry.index);
      boxes.bbox.min_z.erase(boxes.bbox.min_z.begin() + entry.index);
      boxes.bbox.max_x.erase(boxes.bbox.max_x.begin() + entry.index);
      boxes.bbox.max_y.erase(boxes.bbox.max_y.begin() + entry.index);
      boxes.bbox.max_z.erase(boxes.bbox.max_z.begin() + entry.index);

      boxes.hidden.erase(boxes.hidden.begin() + entry.index);
      boxes.layer.erase(boxes.layer.begin() + entry.index);
      boxes.description.erase(boxes.description.begin() + entry.index);
      boxes.ids.erase(boxes.ids.begin() + entry.index);

      boxes.dirty.remove_index(entry.index);
      boxes.dirty.add({entry.index, static_cast<uint32>(boxes.size())});
   }
}

void revert_delete_entries(world::blocks_boxes& boxes,
                           std::span<const delete_entry_block> entries)
{
   for (std::ptrdiff_t i = (std::ssize(entries) - 1); i >= 0; --i) {
      const delete_entry_block& entry = entries[i];

      const math::bounding_box bbox = get_bounding_box(entry.description);

      boxes.bbox.min_x.insert(boxes.bbox.min_x.begin() + entry.index, bbox.min.x);
      boxes.bbox.min_y.insert(boxes.bbox.min_y.begin() + entry.index, bbox.min.y);
      boxes.bbox.min_z.insert(boxes.bbox.min_z.begin() + entry.index, bbox.min.z);
      boxes.bbox.max_x.insert(boxes.bbox.max_x.begin() + entry.index, bbox.max.x);
      boxes.bbox.max_y.insert(boxes.bbox.max_y.begin() + entry.index, bbox.max.y);
      boxes.bbox.max_z.insert(boxes.bbox.max_z.begin() + entry.index, bbox.max.z);

      boxes.hidden.insert(boxes.hidden.begin() + entry.index, entry.hidden);
      boxes.layer.insert(boxes.layer.begin() + entry.index, entry.layer);
      boxes.description.insert(boxes.description.begin() + entry.index,
                               entry.description);
      boxes.ids.insert(boxes.ids.begin() + entry.index, entry.id);

      boxes.dirty.add({entry.index, static_cast<uint32>(boxes.size())});
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

      apply_remap_entries(world.objects, _data.remap_objects);
      apply_remap_entries(world.lights, _data.remap_lights);
      apply_remap_entries(world.paths, _data.remap_paths);
      apply_remap_entries(world.regions, _data.remap_regions);
      apply_remap_entries(world.hintnodes, _data.remap_hintnodes);
      apply_remap_entries(world.game_modes, _data.remap_game_modes);
      apply_remap_entries(world.common_layers, _data.remap_common_layers);
      apply_remap_entries(world.blocks.boxes.layer, _data.remap_blocks_boxes);

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

      revert_remap_entries(world.objects, _data.remap_objects);
      revert_remap_entries(world.lights, _data.remap_lights);
      revert_remap_entries(world.paths, _data.remap_paths);
      revert_remap_entries(world.regions, _data.remap_regions);
      revert_remap_entries(world.hintnodes, _data.remap_hintnodes);
      revert_remap_entries(world.game_modes, _data.remap_game_modes);
      revert_remap_entries(world.common_layers, _data.remap_common_layers);
      revert_remap_entries(world.blocks.boxes.layer, _data.remap_blocks_boxes);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const delete_layer_data _data;
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
      },
      object_class_library);
}

}