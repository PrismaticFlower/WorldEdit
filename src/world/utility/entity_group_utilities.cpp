#include "entity_group_utilities.hpp"
#include "world_utilities.hpp"

#include "../blocks/bounding_box.hpp"
#include "../blocks/find.hpp"
#include "../object_class.hpp"

#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include "utility/string_icompare.hpp"

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we::world {

namespace {

struct unlinked_branch_weights {
   std::string_view hub;
   std::string_view connection;

   float soldier = 0.0f;
   float hover = 0.0f;
   float small = 0.0f;
   float medium = 0.0f;
   float huge = 0.0f;
   float flyer = 0.0f;
};

struct unlinked_planning_hub {
   std::string_view name;

   float3 position;
   float radius = 1.0f;

   std::vector<unlinked_branch_weights> weights;
};

struct unlinked_planning_connection {
   std::string_view name;

   std::string_view start_hub;
   std::string_view end_hub;

   ai_path_flags flags = ai_path_flags::soldier | ai_path_flags::hover |
                         ai_path_flags::small | ai_path_flags::medium |
                         ai_path_flags::huge | ai_path_flags::flyer;

   bool jump = false;
   bool jet_jump = false;
   bool one_way = false;
   int8 dynamic_group = 0;
};

template<typename T>
auto get_placed_entity_name_impl(std::string_view name, std::span<const T> world_entities,
                                 std::span<const std::type_identity_t<T>> group_entities,
                                 const uint32 group_base_index) noexcept
   -> std::string_view
{
   if (world_entities.size() < group_base_index) return name;
   if (world_entities.size() - group_base_index < group_entities.size()) {
      return name;
   }

   for (uint32 i = 0; i < group_entities.size(); ++i) {
      if (string::iequals(name, group_entities[i].name)) {
         return world_entities[group_base_index + i].name;
      }
   }

   return name;
}

void fill_entity_group_block_materials(entity_group& group, const blocks& blocks) noexcept
{
   group.blocks.materials.reserve(max_block_materials);

   std::array<std::optional<uint8>, max_block_materials> materials_remap;

   for (block_description_box& box : group.blocks.boxes) {
      for (uint8& material : box.surface_materials) {
         if (not materials_remap[material]) {
            materials_remap[material] =
               static_cast<uint8>(group.blocks.materials.size());

            group.blocks.materials.push_back(blocks.materials[material]);
         }

         material = *materials_remap[material];
      }
   }

   for (block_description_ramp& ramp : group.blocks.ramps) {
      for (uint8& material : ramp.surface_materials) {
         if (not materials_remap[material]) {
            materials_remap[material] =
               static_cast<uint8>(group.blocks.materials.size());

            group.blocks.materials.push_back(blocks.materials[material]);
         }

         material = *materials_remap[material];
      }
   }

   for (block_description_quad& quad : group.blocks.quads) {
      for (uint8& material : quad.surface_materials) {
         if (not materials_remap[material]) {
            materials_remap[material] =
               static_cast<uint8>(group.blocks.materials.size());

            group.blocks.materials.push_back(blocks.materials[material]);
         }

         material = *materials_remap[material];
      }
   }
}

}

auto entity_group_metrics(const entity_group& group,
                          const object_class_library& object_classes) noexcept
   -> entity_group_placement_metrics
{
   float ground_distance = FLT_MAX;
   math::bounding_box group_bbox{.min = float3{FLT_MAX, FLT_MAX, FLT_MAX},
                                 .max = float3{-FLT_MAX, -FLT_MAX, -FLT_MAX}};

   for (const object& object : group.objects) {
      const math::bounding_box bboxOS =
         object_classes[object.class_handle].model->bounding_box;
      const math::bounding_box bboxGS = object.rotation * bboxOS + object.position;

      ground_distance = std::min(ground_distance, bboxOS.min.y);
      group_bbox = math::combine(group_bbox, bboxGS);
   }

   for (const path& path : group.paths) {
      for (const path::node& node : path.nodes) {
         group_bbox = math::integrate(group_bbox, node.position);
      }
   }

   for (const light& light : group.lights) {
      switch (light.light_type) {
      case light_type::directional: {
         group_bbox = math::integrate(group_bbox, light.position);

      } break;
      case light_type::point: {
         const math::bounding_box bbox{.min = light.position - light.range,
                                       .max = light.position + light.range};

         group_bbox = math::combine(bbox, group_bbox);
      } break;
      case light_type::spot: {
         const float outer_cone_radius =
            light.range * std::tan(light.outer_cone_angle * 0.5f);
         const float half_radius = outer_cone_radius * 0.5f;

         math::bounding_box bbox{.min = {-half_radius, 0.0f, -half_radius},
                                 .max = {half_radius, light.range, half_radius}};

         bbox = light.rotation * bbox + light.position;

         group_bbox = math::combine(bbox, group_bbox);
      } break;
      case light_type::directional_region_box: {
         math::bounding_box bbox{.min = {-light.region_size},
                                 .max = {light.region_size}};

         bbox = light.region_rotation * bbox + light.position;

         group_bbox = math::combine(bbox, group_bbox);
      } break;
      case light_type::directional_region_sphere: {
         const float sphere_radius = length(light.region_size);

         const math::bounding_box bbox{.min = light.position - sphere_radius,
                                       .max = light.position + sphere_radius};

         group_bbox = math::combine(bbox, group_bbox);
      } break;
      case light_type::directional_region_cylinder: {
         const float cylinder_length =
            length(float2{light.region_size.x, light.region_size.z});

         math::bounding_box bbox{.min = {-cylinder_length, -light.region_size.y,
                                         -cylinder_length},
                                 .max = {cylinder_length, light.region_size.y,
                                         cylinder_length}};

         bbox = light.region_rotation * bbox + light.position;

         group_bbox = math::combine(bbox, group_bbox);
      } break;
      }
   }

   for (const region& region : group.regions) {
      switch (region.shape) {
      case region_shape::box: {
         math::bounding_box bbox{.min = {-region.size}, .max = {region.size}};

         bbox = region.rotation * bbox + region.position;

         group_bbox = math::combine(bbox, group_bbox);
      } break;
      case region_shape::sphere: {
         const float sphere_radius = length(region.size);

         const math::bounding_box bbox{.min = region.position - sphere_radius,
                                       .max = region.position + sphere_radius};

         group_bbox = math::combine(bbox, group_bbox);
      } break;
      case region_shape::cylinder: {
         const float cylinder_length = length(float2{region.size.x, region.size.z});

         math::bounding_box bbox{.min = {-cylinder_length, -region.size.y, -cylinder_length},
                                 .max = {cylinder_length, region.size.y, cylinder_length}};

         bbox = region.rotation * bbox + region.position;

         group_bbox = math::combine(bbox, group_bbox);
      } break;
      }
   }

   for (const sector& sector : group.sectors) {
      for (auto& point : sector.points) {
         group_bbox.min.x = std::min(group_bbox.min.x, point.x);
         group_bbox.min.z = std::min(group_bbox.min.z, point.y);

         group_bbox.max.x = std::max(group_bbox.max.x, point.x);
         group_bbox.max.z = std::max(group_bbox.max.z, point.y);
      }

      group_bbox.min.y = std::min(group_bbox.min.y, sector.base);
      group_bbox.max.y = std::max(group_bbox.max.y, sector.base + sector.height);
   }

   for (const portal& portal : group.portals) {
      const float half_width = portal.width * 0.5f;
      const float half_height = portal.height * 0.5f;

      math::bounding_box bbox{.min = {-half_width, -half_height, 0.0f},
                              .max = {half_width, half_height, 0.0f}};

      bbox = portal.rotation * bbox + portal.position;

      group_bbox = math::combine(bbox, group_bbox);
   }

   for (const hintnode& hintnode : group.hintnodes) {
      group_bbox = math::integrate(group_bbox, hintnode.position);
   }

   for (const barrier& barrier : group.barriers) {
      group_bbox = math::integrate(group_bbox, barrier.position);
   }

   for (const planning_hub& hub : group.planning_hubs) {
      group_bbox = math::integrate(group_bbox, hub.position);
   }

   for (const boundary& boundary : group.boundaries) {
      group_bbox = math::integrate(group_bbox, boundary.position);
   }

   for (const measurement& measurement : group.measurements) {
      group_bbox = math::integrate(group_bbox, measurement.start);
      group_bbox = math::integrate(group_bbox, measurement.end);
   }

   for (const block_description_box& box : group.blocks.boxes) {
      const math::bounding_box bboxGS = get_bounding_box(box);

      group_bbox = math::combine(group_bbox, get_bounding_box(box));
      ground_distance = std::min(ground_distance, bboxGS.min.y);
   }

   for (const block_description_ramp& ramp : group.blocks.ramps) {
      const math::bounding_box bboxGS = get_bounding_box(ramp);

      group_bbox = math::combine(group_bbox, get_bounding_box(ramp));
      ground_distance = std::min(ground_distance, bboxGS.min.y);
   }

   for (const block_description_quad& quad : group.blocks.quads) {
      const math::bounding_box bboxGS = get_bounding_box(quad);

      group_bbox = math::combine(group_bbox, get_bounding_box(quad));
      ground_distance = std::min(ground_distance, bboxGS.min.y);
   }

   if (ground_distance == FLT_MAX) ground_distance = 0.0f;

   return {.ground_distance = ground_distance, .visual_bbox = group_bbox};
}

void centre_entity_group(entity_group& group) noexcept
{
   float3 position;
   float count = 0.0f;

   for (const object& object : group.objects) {
      position += object.position;
      count += 1.0f;
   }

   for (const path& path : group.paths) {
      for (const path::node& node : path.nodes) {
         position += node.position;
         count += 1.0f;
      }
   }

   for (const light& light : group.lights) {
      position += light.position;
      count += 1.0f;
   }

   for (const region& region : group.regions) {
      position += region.position;
      count += 1.0f;
   }

   for (const sector& sector : group.sectors) {
      for (const float2& point : sector.points) {
         position += float3{point.x, sector.base, point.y};
         count += 1.0f;
      }
   }

   for (const portal& portal : group.portals) {
      position += portal.position;
      count += 1.0f;
   }

   for (const hintnode& hintnode : group.hintnodes) {
      position += hintnode.position;
      count += 1.0f;
   }

   for (const barrier& barrier : group.barriers) {
      position += barrier.position;
      count += 1.0f;
   }

   for (const planning_hub& hub : group.planning_hubs) {
      position += hub.position;
      count += 1.0f;
   }

   for (const boundary& boundary : group.boundaries) {
      position += boundary.position;
      count += 1.0f;
   }

   for (const measurement& measurement : group.measurements) {
      position += measurement.start;
      position += measurement.end;
      count += 2.0f;
   }

   for (const block_description_box& box : group.blocks.boxes) {
      position += box.position;
      count += 1.0f;
   }

   for (const block_description_ramp& ramp : group.blocks.ramps) {
      position += ramp.position;
      count += 1.0f;
   }

   const float3 centre = position / count;

   for (object& object : group.objects) {
      object.position -= centre;
   }

   for (path& path : group.paths) {
      for (path::node& node : path.nodes) {
         node.position -= centre;
      }
   }

   for (light& light : group.lights) {
      light.position -= centre;
   }

   for (region& region : group.regions) {
      region.position -= centre;
   }

   for (sector& sector : group.sectors) {
      for (float2& point : sector.points) {
         point.x -= centre.x;
         point.y -= centre.z;
      }

      sector.base -= centre.y;
   }

   for (portal& portal : group.portals) {
      portal.position -= centre;
   }

   for (hintnode& hintnode : group.hintnodes) {
      hintnode.position -= centre;
   }

   for (barrier& barrier : group.barriers) {
      barrier.position -= centre;
   }

   for (planning_hub& hub : group.planning_hubs) {
      hub.position -= centre;
   }

   for (boundary& boundary : group.boundaries) {
      boundary.position -= centre;
   }

   for (measurement& measurement : group.measurements) {
      measurement.start -= centre;
      measurement.end -= centre;
   }

   for (block_description_box& box : group.blocks.boxes) {
      box.position -= centre;
   }

   for (block_description_ramp& ramp : group.blocks.ramps) {
      ramp.position -= centre;
   }
}

auto get_placed_entity_name(std::string_view name, std::span<const object> world_objects,
                            const entity_group& group,
                            const uint32 group_base_index) noexcept -> std::string_view
{
   return get_placed_entity_name_impl(name, world_objects, group.objects,
                                      group_base_index);
}

auto get_placed_entity_name(std::string_view name, std::span<const path> world_paths,
                            const entity_group& group,
                            const uint32 group_base_index) noexcept -> std::string_view
{
   return get_placed_entity_name_impl(name, world_paths, group.paths, group_base_index);
}

auto get_placed_entity_name(std::string_view name, std::span<const sector> world_sectors,
                            const entity_group& group,
                            const uint32 group_base_index) noexcept -> std::string_view
{
   return get_placed_entity_name_impl(name, world_sectors, group.sectors,
                                      group_base_index);
}

auto make_entity_group_from_selection(const world& world,
                                      const selection& selection) noexcept -> entity_group
{
   entity_group group;

   std::vector<unlinked_planning_hub> unlinked_hubs;
   std::vector<unlinked_planning_connection> unlinked_connections;

   for (const selected_entity& selected : selection) {
      if (selected.is<object_id>()) {
         const object* object =
            find_entity(world.objects, selected.get<object_id>());

         if (object) group.objects.push_back(*object);
      }
      else if (selected.is<light_id>()) {
         const light* light = find_entity(world.lights, selected.get<light_id>());

         if (light) group.lights.push_back(*light);
      }
      else if (selected.is<path_id_node_mask>()) {
         const auto& [id, node_mask] = selected.get<path_id_node_mask>();
         const path* world_path = find_entity(world.paths, id);

         if (world_path) {
            path& path = group.paths.emplace_back(*world_path);

            path.nodes.clear();

            const std::size_t node_count =
               std::min(world_path->nodes.size(), max_path_nodes);

            for (std::size_t i = 0; i < node_count; ++i) {
               if (not node_mask[i]) continue;

               path.nodes.push_back(world_path->nodes[i]);
            }
         }
      }
      else if (selected.is<region_id>()) {
         const region* region =
            find_entity(world.regions, selected.get<region_id>());

         if (region) group.regions.push_back(*region);
      }
      else if (selected.is<sector_id>()) {
         const sector* sector =
            find_entity(world.sectors, selected.get<sector_id>());

         if (sector) group.sectors.push_back(*sector);
      }
      else if (selected.is<portal_id>()) {
         const portal* portal =
            find_entity(world.portals, selected.get<portal_id>());

         if (portal) group.portals.push_back(*portal);
      }
      else if (selected.is<hintnode_id>()) {
         const hintnode* hintnode =
            find_entity(world.hintnodes, selected.get<hintnode_id>());

         if (hintnode) group.hintnodes.push_back(*hintnode);
      }
      else if (selected.is<barrier_id>()) {
         const barrier* barrier =
            find_entity(world.barriers, selected.get<barrier_id>());

         if (barrier) group.barriers.push_back(*barrier);
      }
      else if (selected.is<planning_hub_id>()) {
         const planning_hub* hub =
            find_entity(world.planning_hubs, selected.get<planning_hub_id>());

         if (hub) {
            unlinked_planning_hub& unlinked = unlinked_hubs.emplace_back(
               unlinked_planning_hub{.name = hub->name,
                                     .position = hub->position,
                                     .radius = hub->radius});

            unlinked.weights.reserve(hub->weights.size());

            for (const planning_branch_weights& weights : hub->weights) {
               unlinked.weights.push_back({
                  .hub = world.planning_hubs[weights.hub_index].name,
                  .connection =
                     world.planning_connections[weights.connection_index].name,
                  .soldier = weights.soldier,
                  .hover = weights.hover,
                  .small = weights.small,
                  .medium = weights.medium,
                  .huge = weights.huge,
                  .flyer = weights.flyer,
               });
            }
         }
      }
      else if (selected.is<planning_connection_id>()) {
         const planning_connection* connection =
            find_entity(world.planning_connections,
                        selected.get<planning_connection_id>());

         if (connection) {
            unlinked_connections.push_back({
               .name = connection->name,
               .start_hub = world.planning_hubs[connection->start_hub_index].name,
               .end_hub = world.planning_hubs[connection->end_hub_index].name,
               .flags = connection->flags,
               .jump = connection->jump,
               .jet_jump = connection->jet_jump,
               .one_way = connection->one_way,
               .dynamic_group = connection->dynamic_group,
            });
         }
      }
      else if (selected.is<boundary_id>()) {
         const boundary* boundary =
            find_entity(world.boundaries, selected.get<boundary_id>());

         if (boundary) group.boundaries.push_back(*boundary);
      }
      else if (selected.is<measurement_id>()) {
         const measurement* measurement =
            find_entity(world.measurements, selected.get<measurement_id>());

         if (measurement) group.measurements.push_back(*measurement);
      }
      else if (selected.is<block_id>()) {
         const block_id id = selected.get<block_id>();
         const std::optional<uint32> block_index = find_block(world.blocks, id);

         if (block_index) {
            switch (id.type()) {
            case block_type::box: {
               group.blocks.boxes.push_back(world.blocks.boxes.description[*block_index]);
            } break;
            case block_type::ramp: {
               group.blocks.ramps.push_back(world.blocks.ramps.description[*block_index]);
            } break;
            case block_type::quad: {
               group.blocks.quads.push_back(world.blocks.quads.description[*block_index]);
            } break;
            }
         }
      }
   }

   group.planning_hubs.reserve(unlinked_hubs.size());
   group.planning_connections.reserve(unlinked_connections.size());

   for (const unlinked_planning_hub& unlinked_hub : unlinked_hubs) {
      group.planning_hubs.push_back({
         .name = std::string{unlinked_hub.name},
         .position = unlinked_hub.position,
         .radius = unlinked_hub.radius,
      });
   }

   for (const unlinked_planning_connection& unlinked_connection : unlinked_connections) {
      uint32 start_hub_index = UINT32_MAX;
      uint32 end_hub_index = UINT32_MAX;

      for (uint32 i = 0; i < group.planning_hubs.size(); ++i) {
         const planning_hub& hub = group.planning_hubs[i];

         if (unlinked_connection.start_hub == hub.name) {
            start_hub_index = i;

            if (end_hub_index != UINT32_MAX) break;
         }

         if (unlinked_connection.end_hub == hub.name) {
            end_hub_index = i;

            if (start_hub_index != UINT32_MAX) break;
         }
      }

      if (start_hub_index == UINT32_MAX or end_hub_index == UINT32_MAX) {
         continue;
      }

      group.planning_connections.push_back({
         .name = std::string{unlinked_connection.name},
         .start_hub_index = start_hub_index,
         .end_hub_index = end_hub_index,
         .flags = unlinked_connection.flags,
         .jump = unlinked_connection.jump,
         .jet_jump = unlinked_connection.jet_jump,
         .one_way = unlinked_connection.one_way,
         .dynamic_group = unlinked_connection.dynamic_group,
      });
   }

   for (uint32 unlinked_index = 0; unlinked_index < unlinked_hubs.size();
        ++unlinked_index) {
      if (unlinked_hubs[unlinked_index].weights.empty()) continue;

      planning_hub& hub = group.planning_hubs[unlinked_index];

      hub.weights.reserve(unlinked_hubs[unlinked_index].weights.size());

      for (const unlinked_branch_weights& weights :
           unlinked_hubs[unlinked_index].weights) {
         uint32 hub_index = UINT32_MAX;
         uint32 connection_index = UINT32_MAX;

         for (uint32 i = 0; i < group.planning_hubs.size(); ++i) {
            if (weights.hub == group.planning_hubs[i].name) {
               hub_index = i;

               break;
            }
         }

         if (hub_index == UINT32_MAX) continue;

         for (uint32 i = 0; i < group.planning_connections.size(); ++i) {
            if (weights.connection == group.planning_connections[i].name) {
               connection_index = i;

               break;
            }
         }

         if (connection_index == UINT32_MAX) continue;

         hub.weights.push_back({
            .hub_index = hub_index,
            .connection_index = connection_index,
            .soldier = weights.soldier,
            .hover = weights.hover,
            .small = weights.small,
            .medium = weights.medium,
            .huge = weights.huge,
            .flyer = weights.flyer,
         });
      }
   }

   fill_entity_group_block_materials(group, world.blocks);

   return group;
}

auto make_entity_group_from_block_id(const blocks& blocks, const block_id id) noexcept
   -> entity_group
{
   const std::optional<uint32> block_index = find_block(blocks, id);

   if (not block_index) return {};

   entity_group group;

   switch (id.type()) {
   case block_type::box: {
      group.blocks.boxes.push_back(blocks.boxes.description[*block_index]);
   } break;
   case block_type::ramp: {
      group.blocks.ramps.push_back(blocks.ramps.description[*block_index]);
   } break;
   case block_type::quad: {
      group.blocks.quads.push_back(blocks.quads.description[*block_index]);
   } break;
   }

   fill_entity_group_block_materials(group, blocks);
   centre_entity_group(group);

   return group;
}

auto make_entity_group_from_layer(const world& world, const int32 layer) noexcept
   -> entity_group
{
   entity_group group;

   for (const object& object : world.objects) {
      if (object.layer == layer) group.objects.push_back(object);
   }

   for (const light& light : world.lights) {
      if (light.layer == layer) group.lights.push_back(light);
   }

   for (const path& path : world.paths) {
      if (path.layer == layer) group.paths.push_back(path);
   }

   for (const region& region : world.regions) {
      if (region.layer == layer) group.regions.push_back(region);
   }

   for (const hintnode& hintnode : world.hintnodes) {
      if (hintnode.layer == layer) group.hintnodes.push_back(hintnode);
   }

   for (uint32 block_index = 0; block_index < world.blocks.boxes.size(); ++block_index) {
      if (world.blocks.boxes.layer[block_index] == layer) {
         group.blocks.boxes.push_back(world.blocks.boxes.description[block_index]);
      }
   }

   for (uint32 block_index = 0; block_index < world.blocks.ramps.size(); ++block_index) {
      if (world.blocks.ramps.layer[block_index] == layer) {
         group.blocks.ramps.push_back(world.blocks.ramps.description[block_index]);
      }
   }

   for (uint32 block_index = 0; block_index < world.blocks.quads.size(); ++block_index) {
      if (world.blocks.quads.layer[block_index] == layer) {
         group.blocks.quads.push_back(world.blocks.quads.description[block_index]);
      }
   }

   fill_entity_group_block_materials(group, world.blocks);

   return group;
}

auto make_entity_group_from_world(const world& world) noexcept -> entity_group
{
   return {
      .objects = {world.objects.begin(), world.objects.end()},
      .lights = {world.lights.begin(), world.lights.end()},
      .paths = {world.paths.begin(), world.paths.end()},
      .regions = {world.regions.begin(), world.regions.end()},
      .sectors = {world.sectors.begin(), world.sectors.end()},
      .portals = {world.portals.begin(), world.portals.end()},
      .hintnodes = {world.hintnodes.begin(), world.hintnodes.end()},
      .barriers = {world.barriers.begin(), world.barriers.end()},
      .planning_hubs = {world.planning_hubs.begin(), world.planning_hubs.end()},
      .planning_connections = {world.planning_connections.begin(),
                               world.planning_connections.end()},
      .boundaries = {world.boundaries.begin(), world.boundaries.end()},
      .measurements = {world.measurements.begin(), world.measurements.end()},

      .blocks =
         {
            .boxes = {world.blocks.boxes.description.begin(),
                      world.blocks.boxes.description.end()},
            .ramps = {world.blocks.ramps.description.begin(),
                      world.blocks.ramps.description.end()},
            .quads = {world.blocks.quads.description.begin(),
                      world.blocks.quads.description.end()},

            .materials = {world.blocks.materials.begin(), world.blocks.materials.end()},
         },
   };
}

bool is_entity_group_empty(const entity_group& group) noexcept
{
   bool empty = true;

   empty &= group.objects.empty();
   empty &= group.lights.empty();
   empty &= group.paths.empty();
   empty &= group.regions.empty();
   empty &= group.sectors.empty();
   empty &= group.portals.empty();
   empty &= group.hintnodes.empty();
   empty &= group.barriers.empty();
   empty &= group.planning_hubs.empty();
   empty &= group.planning_connections.empty();
   empty &= group.boundaries.empty();
   empty &= group.measurements.empty();
   empty &= group.blocks.boxes.empty();
   empty &= group.blocks.ramps.empty();
   empty &= group.blocks.quads.empty();

   return empty;
}

bool is_entity_group_blocks_empty(const entity_group& group) noexcept
{
   bool empty = true;

   empty &= group.blocks.boxes.empty();
   empty &= group.blocks.ramps.empty();
   empty &= group.blocks.quads.empty();

   return empty;
}

}