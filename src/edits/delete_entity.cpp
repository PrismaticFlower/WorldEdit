#include "delete_entity.hpp"
#include "types.hpp"
#include "utility/string_icompare.hpp"
#include "world/object_class_library.hpp"
#include "world/utility/world_utilities.hpp"

#include <vector>

using we::string::iequals;

namespace we::edits {

namespace {

template<typename Type>
auto get_entity_index(const pinned_vector<Type>& entities,
                      const world::id<Type> id) noexcept -> uint32
{
   for (uint32 i = 0; i < entities.size(); ++i) {
      if (entities[i].id == id) return i;
   }

   std::terminate();
}

struct unlinked_object_property {
   uint32 object_index = 0;
   uint32 property_index = 0;
   std::string value;
};

struct broken_planning_weights {
   uint32 hub_index;
   uint32 weight_index;
   world::planning_branch_weights weights;
};

struct delete_object final : edit<world::edit_context> {
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

   struct unlinked_hintnode {
      uint32 hintnode_index = 0;
      std::string value;
   };

   delete_object(world::object object, uint32 object_index,
                 std::vector<unlinked_object_property> unlinked_object_properties,
                 std::vector<unlinked_path_property> unlinked_path_properties,
                 std::vector<unlinked_sector_entry> unlinked_sector_entries,
                 std::vector<unlinked_hintnode> unlinked_hintnodes,
                 world::object_class_library& object_class_library)
      : _object{std::move(object)},
        _object_index{object_index},
        _unlinked_object_properties{std::move(unlinked_object_properties)},
        _unlinked_path_properties{std::move(unlinked_path_properties)},
        _unlinked_sector_entries{std::move(unlinked_sector_entries)},
        _unlinked_hintnodes{std::move(unlinked_hintnodes)},
        _object_class_library{object_class_library}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      for (unlinked_object_property& unlinked : _unlinked_object_properties) {
         std::swap(context.world.objects[unlinked.object_index]
                      .instance_properties[unlinked.property_index]
                      .value,
                   unlinked.value);
      }

      for (unlinked_path_property& unlinked : _unlinked_path_properties) {
         std::vector<world::path::property>& properties =
            context.world.paths[unlinked.path_index].properties;

         std::swap(properties[unlinked.property_index], unlinked.value);

         properties.erase(properties.begin() + unlinked.property_index);
      }

      for (unlinked_sector_entry& unlinked : _unlinked_sector_entries) {
         std::vector<std::string>& objects =
            context.world.sectors[unlinked.sector_index].objects;

         std::swap(objects[unlinked.entry_index], unlinked.entry);

         objects.erase(objects.begin() + unlinked.entry_index);
      }

      for (unlinked_hintnode& unlinked : _unlinked_hintnodes) {
         std::swap(context.world.hintnodes[unlinked.hintnode_index].command_post,
                   unlinked.value);
      }

      _object_class_library.free(context.world.objects[_object_index].class_handle);

      context.world.objects.erase(context.world.objects.begin() + _object_index);
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.world.objects.insert(context.world.objects.begin() + _object_index,
                                   _object);

      for (unlinked_object_property& unlinked : _unlinked_object_properties) {
         std::swap(context.world.objects[unlinked.object_index]
                      .instance_properties[unlinked.property_index]
                      .value,
                   unlinked.value);
      }

      for (std::ptrdiff_t i = (std::ssize(_unlinked_path_properties) - 1);
           i >= 0; --i) {
         unlinked_path_property& unlinked = _unlinked_path_properties[i];

         std::vector<world::path::property>& properties =
            context.world.paths[unlinked.path_index].properties;

         properties.emplace(properties.begin() + unlinked.property_index,
                            std::move(unlinked.value));
      }

      for (std::ptrdiff_t i = (std::ssize(_unlinked_sector_entries) - 1); i >= 0; --i) {
         unlinked_sector_entry& unlinked = _unlinked_sector_entries[i];

         std::vector<std::string>& objects =
            context.world.sectors[unlinked.sector_index].objects;

         objects.insert(objects.begin() + unlinked.entry_index,
                        std::move(unlinked.entry));
      }

      for (std::ptrdiff_t i = (std::ssize(_unlinked_hintnodes) - 1); i >= 0; --i) {
         unlinked_hintnode& unlinked = _unlinked_hintnodes[i];

         std::swap(context.world.hintnodes[unlinked.hintnode_index].command_post,
                   unlinked.value);
      }

      context.world.objects[_object_index].class_handle =
         _object_class_library.acquire(context.world.objects[_object_index].class_name);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::object _object;
   const uint32 _object_index;
   std::vector<unlinked_object_property> _unlinked_object_properties;
   std::vector<unlinked_path_property> _unlinked_path_properties;
   std::vector<unlinked_sector_entry> _unlinked_sector_entries;
   std::vector<unlinked_hintnode> _unlinked_hintnodes;
   world::object_class_library& _object_class_library;
};

template<typename T>
struct delete_entity final : edit<world::edit_context> {
   delete_entity(T entity, uint32 entity_index)
      : _entity{std::move(entity)}, _entity_index{entity_index}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      pinned_vector<T>& entities = world::select_entities<T>(context.world);

      entities.erase(entities.begin() + _entity_index);
   }

   void revert(world::edit_context& context) noexcept override
   {
      pinned_vector<T>& entities = world::select_entities<T>(context.world);

      entities.insert(entities.begin() + _entity_index, _entity);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const T _entity;
   const uint32 _entity_index;
};

struct delete_light final : edit<world::edit_context> {
   struct unlinked_global_light {
      std::string name;
   };

   delete_light(world::light light, uint32 light_index,
                std::unique_ptr<unlinked_global_light> unlinked_global_light_1,
                std::unique_ptr<unlinked_global_light> unlinked_global_light_2)
      : _light{std::move(light)},
        _light_index{light_index},
        _unlinked_global_light_1{std::move(unlinked_global_light_1)},
        _unlinked_global_light_2{std::move(unlinked_global_light_2)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      world::world& world = context.world;

      if (_unlinked_global_light_1) {
         std::swap(world.global_lights.global_light_1, _unlinked_global_light_1->name);
      }

      if (_unlinked_global_light_2) {
         std::swap(world.global_lights.global_light_2, _unlinked_global_light_2->name);
      }

      world.lights.erase(world.lights.begin() + _light_index);
   }

   void revert(world::edit_context& context) noexcept override
   {
      world::world& world = context.world;

      world.lights.insert(world.lights.begin() + _light_index, _light);

      if (_unlinked_global_light_1) {
         std::swap(world.global_lights.global_light_1, _unlinked_global_light_1->name);
      }

      if (_unlinked_global_light_2) {
         std::swap(world.global_lights.global_light_2, _unlinked_global_light_2->name);
      }
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::light _light;
   const uint32 _light_index;

   std::unique_ptr<unlinked_global_light> _unlinked_global_light_1;
   std::unique_ptr<unlinked_global_light> _unlinked_global_light_2;
};

struct delete_path_node final : edit<world::edit_context> {
   delete_path_node(world::path::node node, uint32 path_index, uint32 node_index)
      : _node{std::move(node)}, _path_index{path_index}, _node_index{node_index}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      std::vector<world::path::node>& nodes = context.world.paths[_path_index].nodes;

      nodes.erase(nodes.begin() + _node_index);
   }

   void revert(world::edit_context& context) noexcept override
   {
      std::vector<world::path::node>& nodes = context.world.paths[_path_index].nodes;

      nodes.insert(nodes.begin() + _node_index, _node);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::path::node _node;
   const uint32 _path_index;
   const uint32 _node_index;
};

struct delete_path final : edit<world::edit_context> {
   delete_path(world::path path, uint32 path_index,
               std::vector<unlinked_object_property> unlinked_object_properties)
      : _path{std::move(path)},
        _path_index{path_index},
        _unlinked_object_properties{std::move(unlinked_object_properties)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      context.world.paths.erase(context.world.paths.begin() + _path_index);

      for (unlinked_object_property& unlinked : _unlinked_object_properties) {
         std::swap(context.world.objects[unlinked.object_index]
                      .instance_properties[unlinked.property_index]
                      .value,
                   unlinked.value);
      }
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.world.paths.insert(context.world.paths.begin() + _path_index, _path);

      for (unlinked_object_property& unlinked : _unlinked_object_properties) {
         std::swap(context.world.objects[unlinked.object_index]
                      .instance_properties[unlinked.property_index]
                      .value,
                   unlinked.value);
      }
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::path _path;
   const uint32 _path_index;
   std::vector<unlinked_object_property> _unlinked_object_properties;
};

struct delete_region final : edit<world::edit_context> {
   delete_region(world::region region, uint32 region_index,
                 std::vector<unlinked_object_property> unlinked_object_properties)
      : _region{std::move(region)},
        _region_index{region_index},
        _unlinked_object_properties{std::move(unlinked_object_properties)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      context.world.regions.erase(context.world.regions.begin() + _region_index);

      for (unlinked_object_property& unlinked : _unlinked_object_properties) {
         std::swap(context.world.objects[unlinked.object_index]
                      .instance_properties[unlinked.property_index]
                      .value,
                   unlinked.value);
      }
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.world.regions.insert(context.world.regions.begin() + _region_index,
                                   _region);

      for (unlinked_object_property& unlinked : _unlinked_object_properties) {
         std::swap(context.world.objects[unlinked.object_index]
                      .instance_properties[unlinked.property_index]
                      .value,
                   unlinked.value);
      }
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::region _region;
   const uint32 _region_index;
   std::vector<unlinked_object_property> _unlinked_object_properties;
};

struct delete_sector final : edit<world::edit_context> {
   struct unlinked_portal {
      uint32 portal_index = 0;
      bool sector1 = false;
      bool sector2 = false;
   };

   delete_sector(world::sector sector, uint32 sector_index,
                 std::vector<unlinked_portal> unlinked_portals)
      : _sector{std::move(sector)},
        _sector_index{sector_index},
        _unlinked_portals{std::move(unlinked_portals)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      context.world.sectors.erase(context.world.sectors.begin() + _sector_index);

      for (const auto& unlinked : _unlinked_portals) {
         world::portal& portal = context.world.portals[unlinked.portal_index];

         if (unlinked.sector1) portal.sector1 = "";
         if (unlinked.sector2) portal.sector2 = "";
      }
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.world.sectors.insert(context.world.sectors.begin() + _sector_index,
                                   _sector);

      for (const auto& unlinked : _unlinked_portals) {
         world::portal& portal = context.world.portals[unlinked.portal_index];

         if (unlinked.sector1) portal.sector1 = _sector.name;
         if (unlinked.sector2) portal.sector2 = _sector.name;
      }
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::sector _sector;
   const uint32 _sector_index;
   const std::vector<unlinked_portal> _unlinked_portals;
};

struct delete_planning_hub final : edit<world::edit_context> {
   struct broken_connection {
      uint32 index;
      world::planning_connection connection;
   };

   delete_planning_hub(uint32 hub_index,
                       std::vector<broken_connection> broken_connections,
                       std::vector<broken_planning_weights> broken_weights)
      : _hub_index{hub_index},
        _broken_connections{std::move(broken_connections)},
        _broken_weights{std::move(broken_weights)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      for (broken_planning_weights& broken : _broken_weights) {
         context.world.planning_hubs[broken.hub_index].weights.erase(
            context.world.planning_hubs[broken.hub_index].weights.begin() +
            broken.weight_index);
      }

      for (broken_connection& broken : _broken_connections) {
         broken.connection =
            std::move(context.world.planning_connections[broken.index]);

         context.world.planning_connections.erase(
            context.world.planning_connections.begin() + broken.index);

         for (auto& hub : context.world.planning_hubs) {
            for (world::planning_branch_weights& weights : hub.weights) {
               if (weights.connection_index > broken.index) {
                  weights.connection_index -= 1;
               }
            }
         }
      }

      for (auto& hub : context.world.planning_hubs) {
         for (world::planning_branch_weights& weights : hub.weights) {
            if (weights.hub_index > _hub_index) {
               weights.hub_index -= 1;
            }
         }
      }

      for (auto& connection : context.world.planning_connections) {
         if (connection.start_hub_index > _hub_index) {
            connection.start_hub_index -= 1;
         }

         if (connection.end_hub_index > _hub_index) {
            connection.end_hub_index -= 1;
         }
      }

      _hub = std::move(context.world.planning_hubs[_hub_index]);

      context.world.planning_hubs.erase(context.world.planning_hubs.begin() + _hub_index);
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.world.planning_hubs.insert(context.world.planning_hubs.begin() + _hub_index,
                                         std::move(_hub));

      for (auto& connection : context.world.planning_connections) {
         if (connection.start_hub_index >= _hub_index) {
            connection.start_hub_index += 1;
         }

         if (connection.end_hub_index >= _hub_index) {
            connection.end_hub_index += 1;
         }
      }

      for (auto& hub : context.world.planning_hubs) {
         for (world::planning_branch_weights& weights : hub.weights) {
            if (weights.hub_index >= _hub_index) {
               weights.hub_index += 1;
            }
         }
      }

      for (std::ptrdiff_t i = std::ssize(_broken_connections) - 1; i >= 0; --i) {
         broken_connection& broken = _broken_connections[i];

         context.world.planning_connections
            .insert(context.world.planning_connections.begin() + broken.index,
                    std::move(broken.connection));

         for (auto& hub : context.world.planning_hubs) {
            for (world::planning_branch_weights& weights : hub.weights) {
               if (weights.connection_index >= broken.index) {
                  weights.connection_index += 1;
               }
            }
         }
      }

      for (std::ptrdiff_t i = std::ssize(_broken_weights) - 1; i >= 0; --i) {
         broken_planning_weights& broken = _broken_weights[i];

         context.world.planning_hubs[broken.hub_index].weights.insert(
            context.world.planning_hubs[broken.hub_index].weights.begin() +
               broken.weight_index,
            broken.weights);
      }
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   uint32 _hub_index;
   world::planning_hub _hub;
   std::vector<broken_connection> _broken_connections;
   std::vector<broken_planning_weights> _broken_weights;
};

struct delete_planning_connection final : edit<world::edit_context> {
   delete_planning_connection(uint32 connection_index,
                              std::vector<broken_planning_weights> broken_weights)
      : _connection_index{connection_index},
        _broken_weights{std::move(broken_weights)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      for (broken_planning_weights& broken : _broken_weights) {
         context.world.planning_hubs[broken.hub_index].weights.erase(
            context.world.planning_hubs[broken.hub_index].weights.begin() +
            broken.weight_index);
      }

      for (auto& hub : context.world.planning_hubs) {
         for (world::planning_branch_weights& weights : hub.weights) {
            if (weights.connection_index > _connection_index) {
               weights.connection_index -= 1;
            }
         }
      }

      _connection = std::move(context.world.planning_connections[_connection_index]);

      context.world.planning_connections.erase(
         context.world.planning_connections.begin() + _connection_index);
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.world.planning_connections
         .insert(context.world.planning_connections.begin() + _connection_index,
                 std::move(_connection));

      for (auto& hub : context.world.planning_hubs) {
         for (world::planning_branch_weights& weights : hub.weights) {
            if (weights.connection_index >= _connection_index) {
               weights.connection_index += 1;
            }
         }
      }

      for (std::ptrdiff_t i = std::ssize(_broken_weights) - 1; i >= 0; --i) {
         broken_planning_weights& broken = _broken_weights[i];

         context.world.planning_hubs[broken.hub_index].weights.insert(
            context.world.planning_hubs[broken.hub_index].weights.begin() +
               broken.weight_index,
            broken.weights);
      }
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   uint32 _connection_index;
   world::planning_connection _connection;
   std::vector<broken_planning_weights> _broken_weights;
};

}

auto make_delete_entity(world::object_id object_id, const world::world& world,
                        world::object_class_library& object_class_library)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 object_index = get_entity_index(world.objects, object_id);
   const world::object& object = world.objects[object_index];

   uint32 object_property_count = 0;
   uint32 path_property_count = 0;
   uint32 sector_entry_count = 0;
   uint32 hintnode_count = 0;

   for (const auto& other_object : world.objects) {
      for (const auto& prop : other_object.instance_properties) {
         if (iequals(prop.key, "ControlZone") and iequals(prop.value, object.name)) {
            object_property_count += 1;
         }
      }
   }

   for (const auto& path : world.paths) {
      for (const auto& [key, value] : path.properties) {
         if (iequals(key, "EnableObject") and iequals(value, object.name)) {
            path_property_count += 1;
         }
      }
   }

   for (const auto& sector : world.sectors) {
      for (const auto& entry : sector.objects) {
         if (iequals(entry, object.name)) sector_entry_count += 1;
      }
   }

   for (const auto& hintnode : world.hintnodes) {
      if (iequals(hintnode.command_post, object.name)) hintnode_count += 1;
   }

   std::vector<unlinked_object_property> object_property_refs;
   std::vector<delete_object::unlinked_path_property> path_property_refs;
   std::vector<delete_object::unlinked_sector_entry> sector_entry_refs;
   std::vector<delete_object::unlinked_hintnode> hintnode_refs;

   object_property_refs.reserve(object_property_count);
   path_property_refs.reserve(path_property_count);
   sector_entry_refs.reserve(sector_entry_count);
   hintnode_refs.reserve(hintnode_count);

   for (uint32 other_object_index = 0;
        other_object_index < world.objects.size(); ++other_object_index) {
      const world::object& other_object = world.objects[other_object_index];

      for (uint32 property_index = 0;
           property_index < other_object.instance_properties.size(); ++property_index) {
         const world::instance_property& prop =
            other_object.instance_properties[property_index];

         if (iequals(prop.key, "ControlZone") and iequals(prop.value, object.name)) {
            object_property_refs.emplace_back(other_object_index, property_index);
         }
      }
   }

   for (uint32 path_index = 0; path_index < world.paths.size(); ++path_index) {
      const world::path& path = world.paths[path_index];

      uint32 delete_offset = 0;

      for (uint32 property_index = 0; property_index < path.properties.size();
           ++property_index) {
         const auto& [key, value] = path.properties[property_index];

         if (iequals(key, "EnableObject") and iequals(value, object.name)) {
            path_property_refs.emplace_back(path_index, property_index - delete_offset);

            delete_offset += 1;
         }
      }
   }

   for (uint32 sector_index = 0; sector_index < world.sectors.size(); ++sector_index) {
      const world::sector& sector = world.sectors[sector_index];

      uint32 delete_offset = 0;

      for (uint32 entry_index = 0; entry_index < sector.objects.size(); ++entry_index) {
         if (iequals(sector.objects[entry_index], object.name)) {
            sector_entry_refs.emplace_back(sector_index, entry_index - delete_offset);

            delete_offset += 1;
         }
      }
   }

   for (uint32 hintnode_index = 0; hintnode_index < world.hintnodes.size();
        ++hintnode_index) {
      const world::hintnode& hintnode = world.hintnodes[hintnode_index];

      if (iequals(hintnode.command_post, object.name)) {
         hintnode_refs.emplace_back(hintnode_index);
      }
   }

   return std::make_unique<delete_object>(object, object_index,
                                          std::move(object_property_refs),
                                          std::move(path_property_refs),
                                          std::move(sector_entry_refs),
                                          std::move(hintnode_refs),
                                          object_class_library);
}

auto make_delete_entity(world::light_id light_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 light_index = get_entity_index(world.lights, light_id);
   const world::light& light = world.lights[light_index];

   return std::make_unique<delete_light>(
      light, light_index,
      iequals(light.name, world.global_lights.global_light_1)
         ? std::make_unique<delete_light::unlinked_global_light>()
         : nullptr,
      iequals(light.name, world.global_lights.global_light_2)
         ? std::make_unique<delete_light::unlinked_global_light>()
         : nullptr);
}

auto make_delete_entity(world::path_id path_id, uint32 node, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 path_index = get_entity_index(world.paths, path_id);
   const world::path& path = world.paths[path_index];

   if (path.nodes.size() > 1) {
      return std::make_unique<delete_path_node>(path.nodes[node], path_index, node);
   }

   uint32 unlinked_object_property_count = 0;

   for (const auto& object : world.objects) {
      for (const auto& [key, value] : object.instance_properties) {
         if (iequals(key, "SpawnPath") or iequals(key, "AllyPath") or
             iequals(key, "TurretPath")) {
            if (iequals(value, path.name)) {
               unlinked_object_property_count += 1;
            }
         }
      }
   }

   std::vector<unlinked_object_property> unlinked_object_properties;

   unlinked_object_properties.reserve(unlinked_object_property_count);

   for (uint32 object_index = 0; object_index < world.objects.size(); ++object_index) {
      const world::object& object = world.objects[object_index];

      for (uint32 property_index = 0;
           property_index < object.instance_properties.size(); ++property_index) {
         const auto& [key, value] = object.instance_properties[property_index];

         if (iequals(key, "SpawnPath") or iequals(key, "AllyPath") or
             iequals(key, "TurretPath")) {
            if (iequals(value, path.name)) {
               unlinked_object_properties.emplace_back(object_index, property_index);
            }
         }
      }
   }

   return std::make_unique<delete_path>(path, path_index,
                                        std::move(unlinked_object_properties));
}

auto make_delete_entity(world::region_id region_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 region_index = get_entity_index(world.regions, region_id);
   const world::region& region = world.regions[region_index];

   uint32 unlinked_object_property_count = 0;

   for (const auto& object : world.objects) {
      for (const auto& [key, value] : object.instance_properties) {
         if (iequals(key, "CaptureRegion") or iequals(key, "ControlRegion") or
             iequals(key, "EffectRegion") or iequals(key, "KillRegion") or
             iequals(key, "SpawnRegion")) {
            if (iequals(value, region.description)) {
               unlinked_object_property_count += 1;
            }
         }
      }
   }

   std::vector<unlinked_object_property> unlinked_object_properties;

   unlinked_object_properties.reserve(unlinked_object_property_count);

   for (uint32 object_index = 0; object_index < world.objects.size(); ++object_index) {
      const world::object& object = world.objects[object_index];

      for (uint32 property_index = 0;
           property_index < object.instance_properties.size(); ++property_index) {
         const auto& [key, value] = object.instance_properties[property_index];

         if (iequals(key, "CaptureRegion") or iequals(key, "ControlRegion") or
             iequals(key, "EffectRegion") or iequals(key, "KillRegion") or
             iequals(key, "SpawnRegion")) {
            if (iequals(value, region.name)) {
               unlinked_object_properties.emplace_back(object_index, property_index);
            }
         }
      }
   }

   return std::make_unique<delete_region>(region, region_index,
                                          std::move(unlinked_object_properties));
}

auto make_delete_entity(world::sector_id sector_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 sector_index = get_entity_index(world.sectors, sector_id);
   const world::sector& sector = world.sectors[sector_index];

   uint32 unlinked_portal_count = 0;

   for (const auto& portal : world.portals) {
      if (iequals(portal.sector1, sector.name) or iequals(portal.sector2, sector.name)) {
         unlinked_portal_count += 1;
      }
   }

   std::vector<delete_sector::unlinked_portal> unlinked_portals;

   unlinked_portals.reserve(unlinked_portal_count);

   for (uint32 portal_index = 0; portal_index < world.portals.size(); ++portal_index) {
      const world::portal& portal = world.portals[portal_index];

      if (iequals(portal.sector1, sector.name) or iequals(portal.sector2, sector.name)) {
         unlinked_portals.emplace_back(portal_index,
                                       iequals(portal.sector1, sector.name),
                                       iequals(portal.sector2, sector.name));
      }
   }

   return std::make_unique<delete_sector>(sector, sector_index,
                                          std::move(unlinked_portals));
}

auto make_delete_entity(world::portal_id portal_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 portal_index = get_entity_index(world.portals, portal_id);
   const world::portal& portal = world.portals[portal_index];

   return std::make_unique<delete_entity<world::portal>>(portal, portal_index);
}

auto make_delete_entity(world::hintnode_id hintnode_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 hintnode_index = get_entity_index(world.hintnodes, hintnode_id);
   const world::hintnode& hintnode = world.hintnodes[hintnode_index];

   return std::make_unique<delete_entity<world::hintnode>>(hintnode, hintnode_index);
}

auto make_delete_entity(world::barrier_id barrier_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 barrier_index = get_entity_index(world.barriers, barrier_id);
   const world::barrier& barrier = world.barriers[barrier_index];

   return std::make_unique<delete_entity<world::barrier>>(barrier, barrier_index);
}

auto make_delete_entity(world::planning_hub_id planning_hub_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 planning_hub_index =
      get_entity_index(world.planning_hubs, planning_hub_id);

   uint32 broken_connection_count = 0;

   for (const auto& connection : world.planning_connections) {
      if (connection.start_hub_index == planning_hub_index or
          connection.end_hub_index == planning_hub_index) {
         broken_connection_count += 1;
      }
   }

   std::vector<delete_planning_hub::broken_connection> broken_connections;
   std::vector<uint32> broken_connections_indices;

   broken_connections.reserve(broken_connection_count);
   broken_connections_indices.reserve(broken_connection_count);

   uint32 connection_delete_offset = 0;

   for (uint32 i = 0; i < world.planning_connections.size(); ++i) {
      const world::planning_connection& connection = world.planning_connections[i];

      if (connection.start_hub_index == planning_hub_index or
          connection.end_hub_index == planning_hub_index) {
         broken_connections.emplace_back(i - connection_delete_offset);
         broken_connections_indices.emplace_back(i);

         connection_delete_offset += 1;
      }
   }

   uint32 broken_weights_count = 0;

   for (const auto& hub : world.planning_hubs) {
      for (const world::planning_branch_weights& weights : hub.weights) {
         if (weights.hub_index == planning_hub_index) {
            broken_weights_count += 1;
         }
         else {
            for (const uint32 broken_connection : broken_connections_indices) {
               if (weights.connection_index == broken_connection) {
                  broken_weights_count += 1;

                  break;
               }
            }
         }
      }
   }

   std::vector<broken_planning_weights> broken_weights;

   broken_weights.reserve(broken_weights_count);

   for (uint32 hub_index = 0; hub_index < world.planning_hubs.size(); ++hub_index) {
      const auto& hub = world.planning_hubs[hub_index];

      uint32 weight_delete_offset = 0;

      for (uint32 weight_index = 0; weight_index < hub.weights.size(); ++weight_index) {
         const world::planning_branch_weights& weights = hub.weights[weight_index];

         if (weights.hub_index == planning_hub_index) {
            broken_weights.push_back({.hub_index = hub_index,
                                      .weight_index = weight_index - weight_delete_offset,
                                      .weights = weights});

            weight_delete_offset += 1;
         }
         else {
            for (const uint32 broken_connection : broken_connections_indices) {
               if (weights.connection_index == broken_connection) {
                  broken_weights.push_back({.hub_index = hub_index,
                                            .weight_index = weight_index - weight_delete_offset,
                                            .weights = weights});

                  weight_delete_offset += 1;

                  break;
               }
            }
         }
      }
   }

   return std::make_unique<delete_planning_hub>(planning_hub_index,
                                                std::move(broken_connections),
                                                std::move(broken_weights));
}

auto make_delete_entity(world::planning_connection_id planning_connection_id,
                        const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 planning_connection_index =
      get_entity_index(world.planning_connections, planning_connection_id);

   uint32 broken_weights_count = 0;

   for (const auto& hub : world.planning_hubs) {
      for (const world::planning_branch_weights& weights : hub.weights) {
         if (weights.connection_index == planning_connection_index) {
            broken_weights_count += 1;
         }
      }
   }

   std::vector<broken_planning_weights> broken_weights;

   broken_weights.reserve(broken_weights_count);

   for (uint32 hub_index = 0; hub_index < world.planning_hubs.size(); ++hub_index) {
      const auto& hub = world.planning_hubs[hub_index];

      uint32 weight_delete_offset = 0;

      for (uint32 weight_index = 0; weight_index < hub.weights.size(); ++weight_index) {
         const world::planning_branch_weights& weights = hub.weights[weight_index];

         if (weights.connection_index == planning_connection_index) {
            broken_weights.push_back({.hub_index = hub_index,
                                      .weight_index = weight_index - weight_delete_offset,
                                      .weights = weights});

            weight_delete_offset += 1;
         }
      }
   }

   return std::make_unique<delete_planning_connection>(planning_connection_index,
                                                       std::move(broken_weights));
}

auto make_delete_entity(world::boundary_id boundary_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 boundary_index = get_entity_index(world.boundaries, boundary_id);
   const world::boundary& boundary = world.boundaries[boundary_index];

   return std::make_unique<delete_entity<world::boundary>>(boundary, boundary_index);
}

auto make_delete_entity(world::measurement_id measurement_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 measurement_index =
      get_entity_index(world.measurements, measurement_id);
   const world::measurement& measurement = world.measurements[measurement_index];

   return std::make_unique<delete_entity<world::measurement>>(measurement,
                                                              measurement_index);
}

}
