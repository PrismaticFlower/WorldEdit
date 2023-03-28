#include "delete_layer.hpp"

#include <vector>

namespace we::edits {

namespace {

template<typename T>
struct remap_entry {
   int index = 0;
};

template<typename T>
struct delete_entry {
   int index = 0;
   T entity;
};

struct delete_layer_data {
   int index;
   world::layer_description layer;

   std::vector<remap_entry<world::object>> remap_objects;
   std::vector<remap_entry<world::light>> remap_lights;
   std::vector<remap_entry<world::path>> remap_paths;
   std::vector<remap_entry<world::region>> remap_regions;
   std::vector<remap_entry<world::hintnode>> remap_hintnodes;

   std::vector<delete_entry<world::object>> delete_objects;
   std::vector<delete_entry<world::light>> delete_lights;
   std::vector<delete_entry<world::path>> delete_paths;
   std::vector<delete_entry<world::region>> delete_regions;
   std::vector<delete_entry<world::hintnode>> delete_hintnodes;
};

template<typename T>
auto make_remap_entries(int layer_index, const std::vector<T>& entities)
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

template<typename T>
void apply_remap_entries(std::vector<T>& entities,
                         std::span<const remap_entry<std::type_identity_t<T>>> entries)
{
   for (const auto& [index] : entries) entities[index].layer -= 1;
}

template<typename T>
void revert_remap_entries(std::vector<T>& entities,
                          std::span<const remap_entry<std::type_identity_t<T>>> entries)
{
   for (const auto& [index] : entries) entities[index].layer += 1;
}

template<typename T>
auto make_delete_entries(int layer_index, const std::vector<T>& entities)
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

template<typename T>
void apply_delete_entries(std::vector<T>& entities,
                          std::span<const delete_entry<std::type_identity_t<T>>> entries)
{
   for (const auto& [index, entity] : entries) {
      entities.erase(entities.begin() + index);
   }
}

template<typename T>
void revert_delete_entries(std::vector<T>& entities,
                           std::span<const delete_entry<std::type_identity_t<T>>> entries)
{
   for (std::ptrdiff_t i = (std::ssize(entries) - 1); i >= 0; --i) {
      const auto& [index, entity] = entries[i];

      entities.insert(entities.begin() + index, entity);
   }
}

struct delete_layer final : edit<world::edit_context> {
   delete_layer(delete_layer_data data) : _data{std::move(data)} {}

   void apply(world::edit_context& context) const noexcept override
   {
      world::world& world = context.world;

      world.layer_descriptions.erase(world.layer_descriptions.begin() + _data.index);

      apply_remap_entries(world.objects, _data.remap_objects);
      apply_remap_entries(world.lights, _data.remap_lights);
      apply_remap_entries(world.paths, _data.remap_paths);
      apply_remap_entries(world.regions, _data.remap_regions);
      apply_remap_entries(world.hintnodes, _data.remap_hintnodes);

      apply_delete_entries(world.objects, _data.delete_objects);
      apply_delete_entries(world.lights, _data.delete_lights);
      apply_delete_entries(world.paths, _data.delete_paths);
      apply_delete_entries(world.regions, _data.delete_regions);
      apply_delete_entries(world.hintnodes, _data.delete_hintnodes);
   }

   void revert(world::edit_context& context) const noexcept override
   {
      world::world& world = context.world;

      world.layer_descriptions.insert(world.layer_descriptions.begin() + _data.index,
                                      _data.layer);

      revert_delete_entries(world.objects, _data.delete_objects);
      revert_delete_entries(world.lights, _data.delete_lights);
      revert_delete_entries(world.paths, _data.delete_paths);
      revert_delete_entries(world.regions, _data.delete_regions);
      revert_delete_entries(world.hintnodes, _data.delete_hintnodes);

      revert_remap_entries(world.objects, _data.remap_objects);
      revert_remap_entries(world.lights, _data.remap_lights);
      revert_remap_entries(world.paths, _data.remap_paths);
      revert_remap_entries(world.regions, _data.remap_regions);
      revert_remap_entries(world.hintnodes, _data.remap_hintnodes);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const delete_layer_data _data;
};

}

auto make_delete_layer(int layer_index, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{

   return std::make_unique<delete_layer>(delete_layer_data{
      .index = layer_index,
      .layer = world.layer_descriptions[layer_index],

      .remap_objects = make_remap_entries(layer_index, world.objects),
      .remap_lights = make_remap_entries(layer_index, world.lights),
      .remap_paths = make_remap_entries(layer_index, world.paths),
      .remap_regions = make_remap_entries(layer_index, world.regions),
      .remap_hintnodes = make_remap_entries(layer_index, world.hintnodes),

      .delete_objects = make_delete_entries(layer_index, world.objects),
      .delete_lights = make_delete_entries(layer_index, world.lights),
      .delete_paths = make_delete_entries(layer_index, world.paths),
      .delete_regions = make_delete_entries(layer_index, world.regions),
      .delete_hintnodes = make_delete_entries(layer_index, world.hintnodes)});
}

}