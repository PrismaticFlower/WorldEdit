#include "interaction_context.hpp"
#include "utility/world_utilities.hpp"

namespace we::world {

auto selection::begin() const noexcept -> std::vector<selected_entity>::const_iterator
{
   return _selection.begin();
}

auto selection::end() const noexcept -> std::vector<selected_entity>::const_iterator
{
   return _selection.end();
}

auto selection::view() const noexcept -> std::span<const selected_entity>
{
   return _selection;
}

auto selection::view_updatable() noexcept -> std::span<selected_entity>
{
   return _selection;
}

auto selection::operator[](const std::size_t i) const noexcept -> selected_entity
{
   assert(i < size());

   return _selection[i];
}

void selection::add(const selected_entity entity) noexcept
{
   for (const selected_entity& selected : _selection) {
      if (selected == entity) return;
   }

   _selection.push_back(entity);
}

void selection::remove(const selected_entity entity) noexcept
{
   for (auto it = _selection.begin(); it != _selection.end(); ++it) {
      if (*it == entity) {
         _selection.erase(it);
         return;
      }
   }
}

void selection::clear() noexcept
{
   _selection.clear();
}

bool selection::empty() const noexcept
{
   return _selection.empty();
}

auto selection::size() const noexcept -> std::size_t
{
   return _selection.size();
}

bool is_selected(const selected_entity entity, const selection& selection) noexcept
{
   for (const auto& selected : selection) {
      if (selected == entity) return true;
   }

   return false;
}

bool is_selected(const path_id entity, const selection& selection) noexcept
{
   for (const auto& selected : selection) {
      if (not std::holds_alternative<path_id_node_pair>(selected)) continue;

      if (std::get<path_id_node_pair>(selected).id == entity) return true;
   }

   return false;
}

bool is_valid(const interaction_target entity, const world& world) noexcept
{
   if (std::holds_alternative<object_id>(entity)) {
      return find_entity(world.objects, std::get<object_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<light_id>(entity)) {
      return find_entity(world.lights, std::get<light_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<path_id_node_pair>(entity)) {
      auto [id, node_index] = std::get<path_id_node_pair>(entity);

      const path* path = find_entity(world.paths, id);

      if (not path) return false;

      return path->nodes.size() > node_index;
   }
   if (std::holds_alternative<region_id>(entity)) {
      return find_entity(world.regions, std::get<region_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<sector_id>(entity)) {
      return find_entity(world.sectors, std::get<sector_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<portal_id>(entity)) {
      return find_entity(world.portals, std::get<portal_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<hintnode_id>(entity)) {
      return find_entity(world.hintnodes, std::get<hintnode_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<barrier_id>(entity)) {
      return find_entity(world.barriers, std::get<barrier_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<planning_hub_id>(entity)) {
      return find_entity(world.planning_hubs, std::get<planning_hub_id>(entity)) !=
             nullptr;
   }
   if (std::holds_alternative<planning_connection_id>(entity)) {
      return find_entity(world.planning_connections,
                         std::get<planning_connection_id>(entity)) != nullptr;
   }
   if (std::holds_alternative<boundary_id>(entity)) {
      return find_entity(world.boundaries, std::get<boundary_id>(entity)) != nullptr;
   }

   return false;
}

}