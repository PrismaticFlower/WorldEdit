#include "delete_entity.hpp"
#include "types.hpp"
#include "utility/string_icompare.hpp"
#include "world/utility/world_utilities.hpp"

#include <vector>

namespace we::edits {

namespace {

template<typename Type>
auto get_entity_index(const std::vector<Type>& entities, const world::id<Type> id) noexcept
   -> uint32
{
   for (uint32 i = 0; i < entities.size(); ++i) {
      if (entities[i].id == id) return i;
   }

   std::terminate();
}

struct unlinked_object_property {
   uint32 object_index = 0;
   uint32 property_index = 0;
};

struct delete_object final : edit<world::edit_context> {
   struct path_property_ref {
      uint32 path_index = 0;
      uint32 property_index = 0;
      constexpr static std::string_view property_name = "EnableObject";
   };

   struct sector_entry_ref {
      uint32 sector_index = 0;
      uint32 entry_index = 0;
   };

   struct hintnode_ref {
      uint32 hintnode_index = 0;
   };

   delete_object(world::object object, uint32 object_index,
                 std::vector<path_property_ref> path_property_refs,
                 std::vector<sector_entry_ref> sector_entry_refs,
                 std::vector<hintnode_ref> hintnode_refs)
      : _object{std::move(object)},
        _object_index{object_index},
        _path_property_refs{std::move(path_property_refs)},
        _sector_entry_refs{std::move(sector_entry_refs)},
        _hintnode_refs{std::move(hintnode_refs)}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      context.world.objects.erase(context.world.objects.begin() + _object_index);

      for (const auto& [path_index, property_index] : _path_property_refs) {
         std::vector<world::path::property>& properties =
            context.world.paths[path_index].properties;

         properties.erase(properties.begin() + property_index);
      }

      for (const auto& [sector_index, entry_index] : _sector_entry_refs) {
         std::vector<std::string>& objects = context.world.sectors[sector_index].objects;

         objects.erase(objects.begin() + entry_index);
      }

      for (const auto& [hintnode_index] : _hintnode_refs) {
         context.world.hintnodes[hintnode_index].command_post = "";
      }
   }

   void revert(world::edit_context& context) const noexcept override
   {
      context.world.objects.insert(context.world.objects.begin() + _object_index,
                                   _object);

      for (const auto& [path_index, property_index] : _path_property_refs) {
         std::vector<world::path::property>& properties =
            context.world.paths[path_index].properties;

         properties.emplace(properties.begin() + property_index,
                            std::string{path_property_ref::property_name},
                            _object.name);
      }

      for (const auto& [sector_index, entry_index] : _sector_entry_refs) {
         std::vector<std::string>& objects = context.world.sectors[sector_index].objects;

         objects.insert(objects.begin() + entry_index, _object.name);
      }

      for (const auto& [hintnode_index] : _hintnode_refs) {
         context.world.hintnodes[hintnode_index].command_post = _object.name;
      }
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::object _object;
   const uint32 _object_index;
   const std::vector<path_property_ref> _path_property_refs;
   const std::vector<sector_entry_ref> _sector_entry_refs;
   const std::vector<hintnode_ref> _hintnode_refs;
};

template<typename T>
struct delete_entity final : edit<world::edit_context> {
   delete_entity(T entity, uint32 entity_index)
      : _entity{std::move(entity)}, _entity_index{entity_index}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      std::vector<T>& entities = world::select_entities<T>(context.world);

      entities.erase(entities.begin() + _entity_index);
   }

   void revert(world::edit_context& context) const noexcept override
   {
      std::vector<T>& entities = world::select_entities<T>(context.world);

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

struct delete_path_node final : edit<world::edit_context> {
   delete_path_node(world::path::node node, uint32 path_index, uint32 node_index)
      : _node{std::move(node)}, _path_index{path_index}, _node_index{node_index}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      std::vector<world::path::node>& nodes = context.world.paths[_path_index].nodes;

      nodes.erase(nodes.begin() + _node_index);
   }

   void revert(world::edit_context& context) const noexcept override
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

   void apply(world::edit_context& context) const noexcept override
   {
      context.world.paths.erase(context.world.paths.begin() + _path_index);

      for (const auto& unlinked : _unlinked_object_properties) {
         context.world.objects[unlinked.object_index]
            .instance_properties[unlinked.property_index]
            .value = "";
      }
   }

   void revert(world::edit_context& context) const noexcept override
   {
      context.world.paths.insert(context.world.paths.begin() + _path_index, _path);

      for (const auto& unlinked : _unlinked_object_properties) {
         context.world.objects[unlinked.object_index]
            .instance_properties[unlinked.property_index]
            .value = _path.name;
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
   const std::vector<unlinked_object_property> _unlinked_object_properties;
};

struct delete_region final : edit<world::edit_context> {
   delete_region(world::region region, uint32 region_index,
                 std::vector<unlinked_object_property> unlinked_object_properties)
      : _region{std::move(region)},
        _region_index{region_index},
        _unlinked_object_properties{std::move(unlinked_object_properties)}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      context.world.regions.erase(context.world.regions.begin() + _region_index);

      for (const auto& unlinked : _unlinked_object_properties) {
         context.world.objects[unlinked.object_index]
            .instance_properties[unlinked.property_index]
            .value = "";
      }
   }

   void revert(world::edit_context& context) const noexcept override
   {
      context.world.regions.insert(context.world.regions.begin() + _region_index,
                                   _region);

      for (const auto& unlinked : _unlinked_object_properties) {
         context.world.objects[unlinked.object_index]
            .instance_properties[unlinked.property_index]
            .value = _region.description;
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
   const std::vector<unlinked_object_property> _unlinked_object_properties;
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

   void apply(world::edit_context& context) const noexcept override
   {
      context.world.sectors.erase(context.world.sectors.begin() + _sector_index);

      for (const auto& unlinked : _unlinked_portals) {
         world::portal& portal = context.world.portals[unlinked.portal_index];

         if (unlinked.sector1) portal.sector1 = "";
         if (unlinked.sector2) portal.sector2 = "";
      }
   }

   void revert(world::edit_context& context) const noexcept override
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
      int index;
      world::planning_connection connection;
   };

   delete_planning_hub(world::planning_hub hub, uint32 hub_index,
                       std::vector<broken_connection> broken_connections)
      : _hub{std::move(hub)},
        _hub_index{hub_index},
        _broken_connections{std::move(broken_connections)}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      context.world.planning_hubs.erase(context.world.planning_hubs.begin() + _hub_index);

      for (auto& connection : context.world.planning_connections) {
         if (connection.start_hub_index > _hub_index) {
            connection.start_hub_index -= 1;
         }

         if (connection.end_hub_index > _hub_index) {
            connection.end_hub_index -= 1;
         }
      }

      for (const auto& broken : _broken_connections) {
         context.world.planning_connections.erase(
            context.world.planning_connections.begin() + broken.index);
      }
   }

   void revert(world::edit_context& context) const noexcept override
   {
      context.world.planning_hubs.insert(context.world.planning_hubs.begin() + _hub_index,
                                         _hub);

      for (auto& connection : context.world.planning_connections) {
         if (connection.start_hub_index >= _hub_index) {
            connection.start_hub_index += 1;
         }

         if (connection.end_hub_index >= _hub_index) {
            connection.end_hub_index += 1;
         }
      }

      for (std::ptrdiff_t i = std::ssize(_broken_connections) - 1; i >= 0; --i) {
         const auto& broken = _broken_connections[i];
         context.world.planning_connections
            .insert(context.world.planning_connections.begin() + broken.index,
                    broken.connection);
      }
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::planning_hub _hub;
   const uint32 _hub_index;
   const std::vector<broken_connection> _broken_connections;
};

}

auto make_delete_entity(world::object_id object_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 object_index = get_entity_index(world.objects, object_id);
   const world::object& object = world.objects[object_index];

   uint32 path_property_count = 0;
   uint32 sector_entry_count = 0;
   uint32 hintnode_count = 0;

   for (const auto& path : world.paths) {
      for (const auto& [key, value] : path.properties) {
         if (key == "EnableObject" and value == object.name) {
            path_property_count += 1;
         }
      }
   }

   for (const auto& sector : world.sectors) {
      for (const auto& entry : sector.objects) {
         if (entry == object.name) sector_entry_count += 1;
      }
   }

   for (const auto& hintnode : world.hintnodes) {
      if (hintnode.command_post == object.name) hintnode_count += 1;
   }

   std::vector<delete_object::path_property_ref> path_property_refs;
   std::vector<delete_object::sector_entry_ref> sector_entry_refs;
   std::vector<delete_object::hintnode_ref> hintnode_refs;

   path_property_refs.reserve(path_property_count);
   sector_entry_refs.reserve(sector_entry_count);
   hintnode_refs.reserve(hintnode_count);

   for (uint32 path_index = 0; path_index < world.paths.size(); ++path_index) {
      const world::path& path = world.paths[path_index];

      for (uint32 property_index = 0; property_index < path.properties.size();
           ++property_index) {
         const auto& [key, value] = path.properties[property_index];

         if (key == "EnableObject" and value == object.name) {
            path_property_refs.emplace_back(path_index, property_index);
         }
      }
   }

   for (uint32 sector_index = 0; sector_index < world.sectors.size(); ++sector_index) {
      const world::sector& sector = world.sectors[sector_index];

      for (uint32 entry_index = 0; entry_index < sector.objects.size(); ++entry_index) {
         if (sector.objects[entry_index] == object.name) {
            sector_entry_refs.emplace_back(sector_index, entry_index);
         }
      }
   }

   for (uint32 hintnode_index = 0; hintnode_index < world.hintnodes.size();
        ++hintnode_index) {
      const world::hintnode& hintnode = world.hintnodes[hintnode_index];

      if (hintnode.command_post == object.name) {
         hintnode_refs.emplace_back(hintnode_index);
      }
   }

   return std::make_unique<delete_object>(object, object_index,
                                          std::move(path_property_refs),
                                          std::move(sector_entry_refs),
                                          std::move(hintnode_refs));
}

auto make_delete_entity(world::light_id light_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 light_index = get_entity_index(world.lights, light_id);
   const world::light& light = world.lights[light_index];

   return std::make_unique<delete_entity<world::light>>(light, light_index);
}

auto make_delete_entity(world::path_id_node_pair path_id_node, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 path_index = get_entity_index(world.paths, path_id_node.id);
   const world::path& path = world.paths[path_index];

   if (path.nodes.size() > 1) {
      return std::make_unique<delete_path_node>(path.nodes[path_id_node.node_index],
                                                path_index,
                                                static_cast<uint32>(
                                                   path_id_node.node_index));
   }

   uint32 unlinked_object_property_count = 0;

   for (const auto& object : world.objects) {
      for (const auto& [key, value] : object.instance_properties) {
         if (string::iequals(key, "SpawnPath") or string::iequals(key, "AllyPath") or
             string::iequals(key, "TurretPath")) {
            if (value == path.name) {
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

         if (string::iequals(key, "SpawnPath") or string::iequals(key, "AllyPath") or
             string::iequals(key, "TurretPath")) {
            if (value == path.name) {
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
         if (string::iequals(key, "CaptureRegion") or
             string::iequals(key, "ControlRegion") or
             string::iequals(key, "EffectRegion") or
             string::iequals(key, "KillRegion") or
             string::iequals(key, "SpawnRegion")) {
            if (value == region.description) {
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

         if (string::iequals(key, "CaptureRegion") or
             string::iequals(key, "ControlRegion") or
             string::iequals(key, "EffectRegion") or
             string::iequals(key, "KillRegion") or
             string::iequals(key, "SpawnRegion")) {
            if (value == region.name) {
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
      if (portal.sector1 == sector.name or portal.sector2 == sector.name) {
         unlinked_portal_count += 1;
      }
   }

   std::vector<delete_sector::unlinked_portal> unlinked_portals;

   unlinked_portals.reserve(unlinked_portal_count);

   for (uint32 portal_index = 0; portal_index < world.portals.size(); ++portal_index) {
      const world::portal& portal = world.portals[portal_index];

      if (portal.sector1 == sector.name or portal.sector2 == sector.name) {
         unlinked_portals.emplace_back(portal_index, portal.sector1 == sector.name,
                                       portal.sector2 == sector.name);
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
   const world::planning_hub& planning_hub = world.planning_hubs[planning_hub_index];

   uint32 broken_connection_count = 0;

   for (const auto& connection : world.planning_connections) {
      if (connection.start_hub_index == planning_hub_index or
          connection.end_hub_index == planning_hub_index) {
         broken_connection_count += 1;
      }
   }

   std::vector<delete_planning_hub::broken_connection> broken_connections;

   broken_connections.reserve(broken_connection_count);

   int delete_offset = 0;

   for (int i = 0; i < world.planning_connections.size(); ++i) {
      const world::planning_connection& connection = world.planning_connections[i];

      if (connection.start_hub_index == planning_hub_index or
          connection.end_hub_index == planning_hub_index) {
         broken_connections.emplace_back(i - delete_offset, connection);
         delete_offset += 1;
      }
   }

   return std::make_unique<delete_planning_hub>(planning_hub, planning_hub_index,
                                                std::move(broken_connections));
}

auto make_delete_entity(world::planning_connection_id planning_connection_id,
                        const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 planning_connection_index =
      get_entity_index(world.planning_connections, planning_connection_id);
   const world::planning_connection& planning_connection =
      world.planning_connections[planning_connection_index];

   return std::make_unique<delete_entity<world::planning_connection>>(planning_connection,
                                                                      planning_connection_index);
}

auto make_delete_entity(world::boundary_id boundary_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   const uint32 boundary_index = get_entity_index(world.boundaries, boundary_id);
   const world::boundary& boundary = world.boundaries[boundary_index];

   return std::make_unique<delete_entity<world::boundary>>(boundary, boundary_index);
}

}
