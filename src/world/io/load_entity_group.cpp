#include "load_entity_group.hpp"
#include "load_failure.hpp"

#include "assets/config/io.hpp"
#include "io/error.hpp"
#include "io/read_file.hpp"
#include "math/vector_funcs.hpp"
#include "utility/stopwatch.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include <charconv>

using namespace std::string_view_literals;

namespace we::world {

namespace {

struct unlinked_branch_weight {
   std::string start_hub;
   std::string end_hub;
   float weight = 0.0f;
   std::string connection;
   ai_path_flags flag;
};

struct unlinked_connection {
   std::string name;

   std::string start_hub;
   std::string end_hub;

   ai_path_flags flags = ai_path_flags::soldier | ai_path_flags::hover |
                         ai_path_flags::small | ai_path_flags::medium |
                         ai_path_flags::huge | ai_path_flags::flyer;

   bool jump = false;
   bool jet_jump = false;
   bool one_way = false;
   int8 dynamic_group = 0;
};

auto read_rotation(const assets::config::node& node) -> quaternion
{
   quaternion rotation{node.values.get<float>(0), node.values.get<float>(1),
                       node.values.get<float>(2), node.values.get<float>(3)};

   rotation.x = -rotation.x;
   rotation.z = -rotation.z;

   std::swap(rotation.x, rotation.z);
   std::swap(rotation.y, rotation.w);

   if (rotation.w == 0.0f and rotation.x == 0.0f and rotation.y == 0.0f and
       rotation.z == 0.0f) {
      rotation = {};
   }

   return rotation;
}

auto read_position(const assets::config::node& node) -> float3
{
   return {node.values.get<float>(0), node.values.get<float>(1),
           -node.values.get<float>(2)};
}

auto read_path_properties(const assets::config::node& node)
   -> std::vector<path::property>
{
   std::vector<path::property> properties;

   properties.reserve(node.values.get<std::size_t>(0));

   for (auto& prop : node) {
      if (prop.values.empty()) {
         properties.push_back({.key = prop.key, .value = ""});
      }
      else {
         properties.push_back(
            {.key = prop.key,
             .value = std::visit(
                [](const auto& v) noexcept -> std::string {
                   if constexpr (std::is_same_v<decltype(v), const std::string&>) {
                      return v;
                   }
                   else {
                      return std::to_string(v);
                   }
                },
                prop.values.at(0))});
      }
   }

   return properties;
}

auto read_hintnode_type(const std::string_view value) -> hintnode_type
{
   if (value.empty()) return hintnode_type::snipe;

   if (value[0] >= '0' or value[0] <= '8') {
      return static_cast<hintnode_type>(value[0] - '0');
   }

   int int_value = 0;

   // We don't mind failure here as we'll just return 0.
   (void)std::from_chars(value.data(), value.data() + value.size(), int_value);

   return static_cast<hintnode_type>(int_value);
}

auto read_object(const assets::config::node& node) -> object
{
   object object;

   object.name = node.values.get<std::string>(0);
   object.class_name = lowercase_string{node.values.get<std::string>(1)};

   for (auto& obj_prop : node) {
      if (string::iequals(obj_prop.key, "ChildRotation"sv)) {
         object.rotation = read_rotation(obj_prop);
      }
      else if (string::iequals(obj_prop.key, "ChildPosition"sv)) {
         object.position = read_position(obj_prop);
      }
      else if (string::iequals(obj_prop.key, "Team"sv)) {
         object.team = obj_prop.values.get<int>(0);
      }
      else if (string::iequals(obj_prop.key, "Layer"sv) or
               string::iequals(obj_prop.key, "SeqNo"sv) or
               string::iequals(obj_prop.key, "NetworkId"sv) or
               string::iequals(obj_prop.key, "GeometryFile"sv)) {
         continue;
      }
      else {
         object.instance_properties.push_back(
            {.key = obj_prop.key, .value = obj_prop.values.get<std::string>(0)});
      }
   }

   return object;
}

auto read_light(const assets::config::node& node, output_stream& output) -> light
{
   light light;

   light.name = node.values.get<std::string>(0);

   for (auto& light_prop : node) {
      if (string::iequals(light_prop.key, "Rotation"sv)) {
         light.rotation = read_rotation(light_prop);
      }
      else if (string::iequals(light_prop.key, "Position"sv)) {
         light.position = read_position(light_prop);
      }
      else if (string::iequals(light_prop.key, "Type"sv)) {
         switch (const int type = light_prop.values.get<int>(0); type) {
         case 1:
         case 2:
         case 3:
         case 4:
         case 5:
         case 6:
            light.light_type = static_cast<light_type>(type);
            break;
         default:
            output.write("Warning! World light '{}' has invalid light type! "
                         "Defaulting to point light.\n",
                         light.name);
            light.light_type = light_type::point;
         }
      }
      else if (string::iequals(light_prop.key, "Color"sv)) {
         light.color = {light_prop.values.get<float>(0),
                        light_prop.values.get<float>(1),
                        light_prop.values.get<float>(2)};
      }
      else if (string::iequals(light_prop.key, "Static"sv)) {
         light.static_ = true;
      }
      else if (string::iequals(light_prop.key, "CastShadow"sv)) {
         light.shadow_caster = true;
      }
      else if (string::iequals(light_prop.key, "CastSpecular"sv)) {
         light.specular_caster = true;
      }
      else if (string::iequals(light_prop.key, "Texture"sv)) {
         light.texture = light_prop.values.get<std::string>(0);

         if (light_prop.values.size() >= 2) {
            switch (const int addressing = light_prop.values.get<int>(1); addressing) {
            case 0:
            case 1:
               light.texture_addressing = static_cast<texture_addressing>(addressing);
               break;
            default:
               output.write("Warning! World light '{}' has invalid texture "
                            "addressing mode! Defaulting to clamp.\n",
                            light.name);
               light.texture_addressing = texture_addressing::clamp;
            }
         }
         else {
            output.write("Warning! World light '{}' has no texture "
                         "addressing mode! Defaulting to clamp.\n",
                         light.name);
            light.texture_addressing = texture_addressing::clamp;
         }
      }
      else if (string::iequals(light_prop.key, "TileUV"sv)) {
         light.directional_texture_tiling = {light_prop.values.get<float>(0),
                                             light_prop.values.get<float>(1)};
      }
      else if (string::iequals(light_prop.key, "OffsetUV"sv)) {
         light.directional_texture_offset = {light_prop.values.get<float>(0),
                                             light_prop.values.get<float>(1)};
      }
      else if (string::iequals(light_prop.key, "Range"sv)) {
         light.range = light_prop.values.get<float>(0);
      }
      else if (string::iequals(light_prop.key, "Cone"sv)) {
         light.inner_cone_angle = light_prop.values.get<float>(0);
         light.outer_cone_angle = light_prop.values.get<float>(1);
      }
      else if (string::iequals(light_prop.key, "Bidirectional"sv)) {
         light.bidirectional = light_prop.values.get<int>(0) != 0;
      }
      else if (string::iequals(light_prop.key, "PS2BlendMode"sv)) {
         switch (int mode = light_prop.values.get<int>(0); mode) {
         case 0:
         case 1:
         case 2:
            light.ps2_blend_mode = static_cast<ps2_blend_mode>(mode);
            break;
         default:
            output
               .write("Warning! World light '{}' has invalid PS2 Blend Mode! "
                      "Defaulting to add.\n",
                      light.name);
            light.ps2_blend_mode = ps2_blend_mode::add;
         }
      }
      else if (string::iequals(light_prop.key, "RegionName"sv)) {
         light.region_name = light_prop.values.get<std::string>(0);
      }
      else if (string::iequals(light_prop.key, "RegionRotation"sv)) {
         light.region_rotation = read_rotation(light_prop);
      }
      else if (string::iequals(light_prop.key, "RegionSize"sv)) {
         light.region_size = {light_prop.values.get<float>(0),
                              light_prop.values.get<float>(1),
                              light_prop.values.get<float>(2)};
      }
   }

   return light;
}

auto read_path(const assets::config::node& node) -> path
{
   path path;

   path.name = node.values.get<std::string>(0);

   if (string::istarts_with(path.name, "type_")) {
      if (string::istarts_with(path.name, "type_entitypath")) {
         path.type = path_type::entity_follow;
      }
      else if (string::istarts_with(path.name, "type_entityformation")) {
         path.type = path_type::formation;
      }
      else if (string::istarts_with(path.name, "type_patrolpath")) {
         path.type = path_type::patrol;
      }

      path.name = string::split_first_of_exclusive(path.name, " ")[1];
   }

   for (auto& path_prop : node) {
      if (string::iequals(path_prop.key, "Properties"sv)) {
         path.properties = read_path_properties(path_prop);
      }
      else if (string::iequals(path_prop.key, "SplineType"sv)) {
         if (const auto spline = path_prop.values.get<std::string_view>(0);
             string::iequals(spline, "None"sv)) {
            path.spline_type = path_spline_type::none;
         }
         else if (string::iequals(spline, "Linear"sv)) {
            path.spline_type = path_spline_type::linear;
         }
         else if (string::iequals(spline, "Hermite"sv)) {
            path.spline_type = path_spline_type::hermite;
         }
         else if (string::iequals(spline, "Catmull-Rom"sv)) {
            path.spline_type = path_spline_type::catmull_rom;
         }
      }
      else if (string::iequals(path_prop.key, "Nodes"sv)) {
         path.nodes.reserve(path_prop.values.get<std::size_t>(0));

         for (auto& child_node_prop : path_prop) {
            path::node& path_node = path.nodes.emplace_back();

            for (auto& node_prop : child_node_prop) {
               if (string::iequals(node_prop.key, "Rotation"sv)) {
                  path_node.rotation = read_rotation(node_prop);
               }
               else if (string::iequals(node_prop.key, "Position"sv)) {
                  path_node.position = read_position(node_prop);
               }
               else if (string::iequals(node_prop.key, "Properties"sv)) {
                  path_node.properties = read_path_properties(node_prop);
               }
            }
         }

         if (path.nodes.size() >= max_path_nodes) {
            throw load_failure{fmt::format("Path '{}' has too many nodes "
                                           "for WorldEdit to handle.\n"
                                           "   Path Node Count: {}\n",
                                           "   Max Supported Count: {}\n", path.name,
                                           path.nodes.size(), max_path_nodes)};
         }
      }
   }

   return path;
}

auto read_region(const assets::config::node& node, output_stream& output) -> region
{
   region region;

   region.description = node.values.get<std::string>(0);
   switch (const int shape = node.values.get<int>(1); shape) {
   case 0:
   case 1:
   case 2:
      region.shape = static_cast<region_shape>(shape);
      break;
   default:
      output.write("Warning! World region '{}' has invalid shape! "
                   "Defaulting to box.\n",
                   region.name);
      region.shape = region_shape::box;
   }

   for (auto& region_prop : node) {
      if (string::iequals(region_prop.key, "Position"sv)) {
         region.position = read_position(region_prop);
      }
      else if (string::iequals(region_prop.key, "Rotation"sv)) {
         region.rotation = read_rotation(region_prop);
      }
      else if (string::iequals(region_prop.key, "Size"sv)) {
         region.size = {region_prop.values.get<float>(0),
                        region_prop.values.get<float>(1),
                        region_prop.values.get<float>(2)};
      }
      else if (string::iequals(region_prop.key, "Name"sv)) {
         region.name = region_prop.values.get<std::string>(0);
      }
   }

   return region;
}

auto read_sector(const assets::config::node& node) -> sector
{
   sector sector;

   sector.name = node.values.get<std::string>(0);

   for (auto& sector_prop : node) {
      if (string::iequals(sector_prop.key, "Base"sv)) {
         sector.base = sector_prop.values.get<float>(0);
      }
      else if (string::iequals(sector_prop.key, "Height"sv)) {
         sector.height = sector_prop.values.get<float>(0);
      }
      else if (string::iequals(sector_prop.key, "Point"sv)) {
         sector.points.push_back({sector_prop.values.get<float>(0),
                                  -sector_prop.values.get<float>(1)});
      }
      else if (string::iequals(sector_prop.key, "Object"sv)) {
         sector.objects.emplace_back(sector_prop.values.get<std::string>(0));
      }
   }

   return sector;
}

auto read_portal(const assets::config::node& node) -> portal
{
   portal portal;

   portal.name = node.values.get<std::string>(0);

   for (auto& portal_prop : node) {
      if (string::iequals(portal_prop.key, "Position"sv)) {
         portal.position = read_position(portal_prop);
      }
      else if (string::iequals(portal_prop.key, "Rotation"sv)) {
         portal.rotation = read_rotation(portal_prop);
      }
      else if (string::iequals(portal_prop.key, "Width"sv)) {
         portal.width = portal_prop.values.get<float>(0);
      }
      else if (string::iequals(portal_prop.key, "Height"sv)) {
         portal.height = portal_prop.values.get<float>(0);
      }
      else if (string::iequals(portal_prop.key, "Sector1"sv)) {
         portal.sector1 = portal_prop.values.get<std::string>(0);
      }
      else if (string::iequals(portal_prop.key, "Sector2"sv)) {
         portal.sector2 = portal_prop.values.get<std::string>(0);
      }
   }

   return portal;
}

auto read_hintnode(const assets::config::node& node) -> hintnode
{
   hintnode hintnode;

   hintnode.name = node.values.get<std::string>(0);
   hintnode.type = read_hintnode_type(node.values.get<std::string>(1));

   for (auto& hintnode_prop : node) {
      if (string::iequals(hintnode_prop.key, "Position"sv)) {
         hintnode.position = read_position(hintnode_prop);
      }
      else if (string::iequals(hintnode_prop.key, "Rotation"sv)) {
         hintnode.rotation = read_rotation(hintnode_prop);
      }
      else if (string::iequals(hintnode_prop.key, "Radius"sv)) {
         hintnode.radius = hintnode_prop.values.get<float>(0);
      }
      else if (string::iequals(hintnode_prop.key, "PrimaryStance"sv)) {
         hintnode.primary_stance =
            static_cast<stance_flags>(hintnode_prop.values.get<int>(0));
      }
      else if (string::iequals(hintnode_prop.key, "SecondaryStance"sv)) {
         hintnode.secondary_stance =
            static_cast<stance_flags>(hintnode_prop.values.get<int>(0));
      }
      else if (string::iequals(hintnode_prop.key, "Mode"sv)) {
         hintnode.mode =
            static_cast<hintnode_mode>(hintnode_prop.values.get<int>(0));
      }
      else if (string::iequals(hintnode_prop.key, "CommandPost"sv)) {
         hintnode.command_post = hintnode_prop.values.get<std::string>(0);
      }
   }

   return hintnode;
}

auto read_barrier(const assets::config::node& node) -> barrier
{
   barrier barrier;

   barrier.name = node.values.get<std::string>(0);

   std::array<float3, 4> corners;
   uint32 corner_index = 0;

   for (auto& barrier_prop : node) {
      if (string::iequals(barrier_prop.key, "Flag"sv)) {
         barrier.flags = ai_path_flags{barrier_prop.values.get<int>(0)};
      }
      else if (string::iequals(barrier_prop.key, "Corner"sv) and
               corner_index < corners.size()) {
         corners[corner_index] = read_position(barrier_prop);
         corner_index += 1;
      }
   }

   barrier.position = (corners[0] + corners[1] + corners[2] + corners[3]) / 4.0f;
   barrier.size = float2{distance(float2{corners[0].x, corners[0].z},
                                  float2{corners[3].x, corners[3].z}),
                         distance(float2{corners[0].x, corners[0].z},
                                  float2{corners[1].x, corners[1].z})} *
                  0.5f;
   barrier.rotation_angle =
      std::atan2(corners[1].x - corners[0].x, corners[1].z - corners[0].z);

   return barrier;
}

auto read_hub(const assets::config::node& node,
              std::vector<unlinked_branch_weight>& out_branch_weights) -> planning_hub
{
   planning_hub hub;

   hub.name = node.values.get<std::string>(0);

   for (auto& hub_prop : node) {
      if (string::iequals(hub_prop.key, "Pos"sv)) {
         hub.position = read_position(hub_prop);
      }
      else if (string::iequals(hub_prop.key, "Radius"sv)) {
         hub.radius = hub_prop.values.get<float>(0);
      }
      else if (string::iequals(hub_prop.key, "BranchWeight"sv)) {
         out_branch_weights.push_back(
            {.start_hub = hub.name,
             .end_hub = hub_prop.values.get<std::string>(0),
             .weight = hub_prop.values.get<float>(1),
             .connection = hub_prop.values.get<std::string>(2),
             .flag = static_cast<ai_path_flags>(hub_prop.values.get<int>(3))});
      }
   }

   return hub;
}

auto read_connection(const assets::config::node& node) -> unlinked_connection
{
   unlinked_connection connection;

   connection.name = node.values.get<std::string>(0);

   for (auto& connection_prop : node) {
      if (string::iequals(connection_prop.key, "Start"sv)) {
         connection.start_hub = connection_prop.values.get<std::string>(0);
      }
      else if (string::iequals(connection_prop.key, "End"sv)) {
         connection.end_hub = connection_prop.values.get<std::string>(0);
      }
      else if (string::iequals(connection_prop.key, "Flag"sv)) {
         connection.flags =
            static_cast<ai_path_flags>(connection_prop.values.get<int>(0));
      }
      else if (string::iequals(connection_prop.key, "Dynamic"sv)) {
         connection.dynamic_group = connection_prop.values.get<int8>(0);
      }
      else if (string::iequals(connection_prop.key, "Jump"sv)) {
         connection.jump = true;
      }
      else if (string::iequals(connection_prop.key, "JetJump"sv)) {
         connection.jet_jump = true;
      }
      else if (string::iequals(connection_prop.key, "OneWay"sv)) {
         connection.one_way = true;
      }
   }

   return connection;
}

auto read_boundary(const assets::config::node& node) -> boundary
{
   boundary boundary;

   boundary.name = node.values.get<std::string>(0);

   float3 min_node = {FLT_MAX, FLT_MAX, FLT_MAX};
   float3 max_node = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

   bool has_node = false;

   for (auto& boundary_prop : node) {
      if (string::iequals(boundary_prop.key, "Node"sv)) {
         const float3 position = read_position(boundary_prop);

         min_node = min(min_node, position);
         max_node = max(max_node, position);

         has_node = true;
      }
   }

   if (has_node) {
      const float3 size = abs(max_node - min_node) / 2.0f;

      boundary.position = (min_node + max_node) / 2.0f;
      boundary.size = {size.x, size.z};
   }

   return boundary;
}

auto read_measurement(const assets::config::node& node) -> measurement
{
   measurement measurement;

   measurement.name = node.values.get<std::string>(0);

   for (auto& measurement_prop : node) {
      if (string::iequals(measurement_prop.key, "Start"sv)) {
         measurement.start = read_position(measurement_prop);
      }
      else if (string::iequals(measurement_prop.key, "End"sv)) {
         measurement.end = read_position(measurement_prop);
      }
   }

   return measurement;
}

void read_blocks_boxes(const assets::config::node& node, entity_group& group_out)
{
   for (const auto& key_node : node) {
      if (not string::iequals(key_node.key, "Box")) continue;

      block_description_box box;

      for (const auto& prop : key_node) {
         if (string::iequals(prop.key, "Rotation")) {
            box.rotation = {prop.values.get<float>(0), prop.values.get<float>(1),
                            prop.values.get<float>(2), prop.values.get<float>(3)};
         }
         else if (string::iequals(prop.key, "Position")) {
            box.position = {prop.values.get<float>(0), prop.values.get<float>(1),
                            prop.values.get<float>(2)};
         }
         else if (string::iequals(prop.key, "Size")) {
            box.size = {prop.values.get<float>(0), prop.values.get<float>(1),
                        prop.values.get<float>(2)};
         }
         else if (string::iequals(prop.key, "SurfaceMaterials")) {
            for (uint32 i = 0; i < box.surface_materials.size(); ++i) {
               box.surface_materials[i] = prop.values.get<uint8>(i);
            }
         }
         else if (string::iequals(prop.key, "SurfaceTextureMode")) {
            for (uint32 i = 0; i < box.surface_texture_mode.size(); ++i) {
               const uint8 texture_mode = prop.values.get<uint8>(i);

               switch (texture_mode) {
               case static_cast<uint8>(block_texture_mode::tangent_space_xyz):
               case static_cast<uint8>(block_texture_mode::world_space_auto):
               case static_cast<uint8>(block_texture_mode::world_space_zy):
               case static_cast<uint8>(block_texture_mode::world_space_xz):
               case static_cast<uint8>(block_texture_mode::world_space_xy):
               case static_cast<uint8>(block_texture_mode::unwrapped):
                  box.surface_texture_mode[i] = block_texture_mode{texture_mode};
                  break;
               }
            }
         }
         else if (string::iequals(prop.key, "SurfaceTextureRotation")) {
            for (uint32 i = 0; i < box.surface_texture_rotation.size(); ++i) {
               const uint8 rotation = prop.values.get<uint8>(i);

               switch (rotation) {
               case static_cast<uint8>(block_texture_rotation::d0):
               case static_cast<uint8>(block_texture_rotation::d90):
               case static_cast<uint8>(block_texture_rotation::d180):
               case static_cast<uint8>(block_texture_rotation::d270):
                  box.surface_texture_rotation[i] = block_texture_rotation{rotation};
                  break;
               }
            }
         }
         else if (string::iequals(prop.key, "SurfaceTextureScale")) {
            for (uint32 i = 0; i < box.surface_texture_scale.size(); ++i) {
               box.surface_texture_scale[i] =
                  {std::clamp(prop.values.get<int8>(i * 2 + 0),
                              block_min_texture_scale, block_max_texture_scale),
                   std::clamp(prop.values.get<int8>(i * 2 + 1),
                              block_min_texture_scale, block_max_texture_scale)};
            }
         }
         else if (string::iequals(prop.key, "SurfaceTextureOffset")) {
            for (uint32 i = 0; i < box.surface_texture_offset.size(); ++i) {
               box.surface_texture_offset[i] =
                  {std::min(prop.values.get<uint16>(i * 2 + 0), block_max_texture_offset),
                   std::min(prop.values.get<uint16>(i * 2 + 1), block_max_texture_offset)};
            }
         }
      }

      group_out.blocks.boxes.push_back(box);
   }
}

void read_blocks_ramps(const assets::config::node& node, entity_group& group_out)
{
   for (const auto& key_node : node) {
      if (not string::iequals(key_node.key, "Ramp")) continue;

      block_description_ramp ramp;

      for (const auto& prop : key_node) {
         if (string::iequals(prop.key, "Rotation")) {
            ramp.rotation = {prop.values.get<float>(0), prop.values.get<float>(1),
                             prop.values.get<float>(2), prop.values.get<float>(3)};
         }
         else if (string::iequals(prop.key, "Position")) {
            ramp.position = {prop.values.get<float>(0), prop.values.get<float>(1),
                             prop.values.get<float>(2)};
         }
         else if (string::iequals(prop.key, "Size")) {
            ramp.size = {prop.values.get<float>(0), prop.values.get<float>(1),
                         prop.values.get<float>(2)};
         }
         else if (string::iequals(prop.key, "SurfaceMaterials")) {
            for (uint32 i = 0; i < ramp.surface_materials.size(); ++i) {
               ramp.surface_materials[i] = prop.values.get<uint8>(i);
            }
         }
         else if (string::iequals(prop.key, "SurfaceTextureMode")) {
            for (uint32 i = 0; i < ramp.surface_texture_mode.size(); ++i) {
               const uint8 texture_mode = prop.values.get<uint8>(i);

               switch (texture_mode) {
               case static_cast<uint8>(block_texture_mode::tangent_space_xyz):
               case static_cast<uint8>(block_texture_mode::world_space_auto):
               case static_cast<uint8>(block_texture_mode::world_space_zy):
               case static_cast<uint8>(block_texture_mode::world_space_xz):
               case static_cast<uint8>(block_texture_mode::world_space_xy):
               case static_cast<uint8>(block_texture_mode::unwrapped):
                  ramp.surface_texture_mode[i] = block_texture_mode{texture_mode};
                  break;
               }
            }
         }
         else if (string::iequals(prop.key, "SurfaceTextureRotation")) {
            for (uint32 i = 0; i < ramp.surface_texture_rotation.size(); ++i) {
               const uint8 rotation = prop.values.get<uint8>(i);

               switch (rotation) {
               case static_cast<uint8>(block_texture_rotation::d0):
               case static_cast<uint8>(block_texture_rotation::d90):
               case static_cast<uint8>(block_texture_rotation::d180):
               case static_cast<uint8>(block_texture_rotation::d270):
                  ramp.surface_texture_rotation[i] = block_texture_rotation{rotation};
                  break;
               }
            }
         }
         else if (string::iequals(prop.key, "SurfaceTextureScale")) {
            for (uint32 i = 0; i < ramp.surface_texture_scale.size(); ++i) {
               ramp.surface_texture_scale[i] =
                  {std::clamp(prop.values.get<int8>(i * 2 + 0),
                              block_min_texture_scale, block_max_texture_scale),
                   std::clamp(prop.values.get<int8>(i * 2 + 1),
                              block_min_texture_scale, block_max_texture_scale)};
            }
         }
         else if (string::iequals(prop.key, "SurfaceTextureOffset")) {
            for (uint32 i = 0; i < ramp.surface_texture_offset.size(); ++i) {
               ramp.surface_texture_offset[i] =
                  {std::min(prop.values.get<uint16>(i * 2 + 0), block_max_texture_offset),
                   std::min(prop.values.get<uint16>(i * 2 + 1), block_max_texture_offset)};
            }
         }
      }

      group_out.blocks.ramps.push_back(ramp);
   }
}

void read_blocks_materials(const assets::config::node& node, entity_group& group_out)
{
   for (const auto& key_node : node) {
      if (not string::iequals(key_node.key, "Material")) continue;

      block_material material;

      for (const auto& prop : key_node) {
         if (string::iequals(prop.key, "Name")) {
            material.name = prop.values.get<std::string>(0);
         }
         else if (string::iequals(prop.key, "DiffuseMap")) {
            material.diffuse_map = prop.values.get<std::string>(0);
         }
         else if (string::iequals(prop.key, "NormalMap")) {
            material.normal_map = prop.values.get<std::string>(0);
         }
         else if (string::iequals(prop.key, "DetailMap")) {
            material.detail_map = prop.values.get<std::string>(0);
         }
         else if (string::iequals(prop.key, "EnvMap")) {
            material.env_map = prop.values.get<std::string>(0);
         }
         else if (string::iequals(prop.key, "DetailTiling")) {
            material.detail_tiling = {prop.values.get<uint8>(0),
                                      prop.values.get<uint8>(1)};
         }
         else if (string::iequals(prop.key, "TileNormalMap")) {
            material.tile_normal_map = prop.values.get<uint8>(0) != 0;
         }
         else if (string::iequals(prop.key, "SpecularLighting")) {
            material.specular_lighting = prop.values.get<uint8>(0) != 0;
         }
         else if (string::iequals(prop.key, "SpecularColor")) {
            material.specular_color = {prop.values.get<float>(0),
                                       prop.values.get<float>(1),
                                       prop.values.get<float>(2)};
         }
         else if (string::iequals(prop.key, "FoleyFXGroup")) {
            const uint8 foley_group = prop.values.get<uint8>(0);

            switch (foley_group) {
            case static_cast<uint8>(block_foley_group::stone):
            case static_cast<uint8>(block_foley_group::dirt):
            case static_cast<uint8>(block_foley_group::grass):
            case static_cast<uint8>(block_foley_group::metal):
            case static_cast<uint8>(block_foley_group::snow):
            case static_cast<uint8>(block_foley_group::terrain):
            case static_cast<uint8>(block_foley_group::water):
            case static_cast<uint8>(block_foley_group::wood):
               material.foley_group = block_foley_group{foley_group};
               break;
            }
         }
      }

      group_out.blocks.materials.push_back(std::move(material));
   }
}

auto link_connections(std::vector<unlinked_connection> unlinked_connections,
                      const std::vector<planning_hub>& hubs,
                      output_stream& output) -> std::vector<planning_connection>
{
   std::vector<planning_connection> connections;
   connections.reserve(unlinked_connections.size());

   for (unlinked_connection& unlinked : unlinked_connections) {
      uint32 start_hub_index = UINT32_MAX;
      uint32 end_hub_index = UINT32_MAX;

      for (uint32 i = 0; i < hubs.size(); ++i) {
         if (string::iequals(hubs[i].name, unlinked.start_hub)) {
            start_hub_index = i;

            if (end_hub_index != UINT32_MAX) break;
         }

         if (string::iequals(hubs[i].name, unlinked.end_hub)) {
            end_hub_index = i;

            if (start_hub_index != UINT32_MAX) break;
         }
      }

      if (start_hub_index == UINT32_MAX) {
         output.write("Warning! Connection '{}' in entity group references "
                      "nonexistent hub '{}'. Discarding connection.\n",
                      unlinked.name, unlinked.start_hub);

         continue;
      }

      if (end_hub_index == UINT32_MAX) {
         output.write("Warning! Connection '{}' in entity group references "
                      "nonexistent hub '{}'. Discarding connection.\n",
                      unlinked.name, unlinked.end_hub);

         continue;
      }

      connections.push_back({.name = std::move(unlinked.name),
                             .start_hub_index = start_hub_index,
                             .end_hub_index = end_hub_index,
                             .flags = unlinked.flags,
                             .jump = unlinked.jump,
                             .jet_jump = unlinked.jet_jump,
                             .one_way = unlinked.one_way,
                             .dynamic_group = unlinked.dynamic_group});
   }

   return connections;
}

void add_branch_weights(const std::vector<unlinked_branch_weight>& unlinked_branch_weights,
                        entity_group& group_out, output_stream& output)
{
   for (const unlinked_branch_weight& branch_weight : unlinked_branch_weights) {
      uint32 start_hub_index = UINT32_MAX;
      uint32 end_hub_index = UINT32_MAX;
      uint32 connection_index = UINT32_MAX;

      for (uint32 i = 0; i < group_out.planning_hubs.size(); ++i) {
         if (string::iequals(group_out.planning_hubs[i].name, branch_weight.start_hub)) {
            start_hub_index = i;

            if (end_hub_index != UINT32_MAX) break;
         }

         if (string::iequals(group_out.planning_hubs[i].name, branch_weight.end_hub)) {
            end_hub_index = i;

            if (start_hub_index != UINT32_MAX) break;
         }
      }

      for (uint32 i = 0; i < group_out.planning_connections.size(); ++i) {
         if (string::iequals(group_out.planning_connections[i].name,
                             branch_weight.connection)) {
            connection_index = i;
         }
      }

      if (start_hub_index == UINT32_MAX) {
         output.write("Warning! Hub '{}' for branch weight has gone missing.\n",
                      branch_weight.start_hub);

         continue;
      }

      if (end_hub_index == UINT32_MAX) {
         output.write("Warning! Branch weight for hub '{}' targets "
                      "nonexistent hub '{}'. Discarding branch weight.\n",
                      branch_weight.start_hub, branch_weight.end_hub);

         continue;
      }

      if (connection_index == UINT32_MAX) {
         output.write("Warning! Branch weight for hub '{}' references "
                      "nonexistent connection '{}'.\n",
                      branch_weight.start_hub, branch_weight.connection);

         continue;
      }

      const planning_connection& connection =
         group_out.planning_connections[connection_index];
      planning_hub& hub = group_out.planning_hubs[start_hub_index];

      if (connection.start_hub_index != start_hub_index and
          connection.end_hub_index != start_hub_index) {
         output.write("BranchWeight for hub '{}' is invalid. Referenced "
                      "connection '{}' is not joined to the hub.\n",
                      hub.name, connection.name);

         continue;
      }

      planning_branch_weights& weights =
         [&hub, end_hub_index, connection_index]() noexcept -> planning_branch_weights& {
         for (planning_branch_weights& weights : hub.weights) {
            if (weights.hub_index == end_hub_index and
                weights.connection_index == connection_index) {
               return weights;
            }
         }

         return hub.weights.emplace_back(
            planning_branch_weights{.hub_index = end_hub_index,
                                    .connection_index = connection_index});
      }();

      switch (branch_weight.flag) {
      case ai_path_flags::soldier:
         weights.soldier = branch_weight.weight;
         break;
      case ai_path_flags::hover:
         weights.hover = branch_weight.weight;
         break;
      case ai_path_flags::small:
         weights.small = branch_weight.weight;
         break;
      case ai_path_flags::medium:
         weights.medium = branch_weight.weight;
         break;
      case ai_path_flags::huge:
         weights.huge = branch_weight.weight;
         break;
      case ai_path_flags::flyer:
         weights.flyer = branch_weight.weight;
         break;
      default:
         output.write("BranchWeight for Hub '{}' (targetting hub '{}' "
                      "through connection '{}') is invalid. It must have "
                      "one and only one "
                      "path flag set.\n",
                      hub.name, branch_weight.end_hub, branch_weight.connection);
      }
   }
}

void clamp_block_material_indices(entity_group& group_out, output_stream& output) noexcept
{
   const std::size_t max_material = group_out.blocks.materials.size();

   for (uint32 block_index = 0; block_index < group_out.blocks.boxes.size();
        ++block_index) {
      block_description_box& box = group_out.blocks.boxes[block_index];

      for (uint8& material : box.surface_materials) {
         if (material >= max_material) {
            output.write("Warning! Box block '{}' has out of range material "
                         "index. Setting to zero.\n",
                         block_index);

            material = 0;
         }
      }
   }

   for (uint32 block_index = 0; block_index < group_out.blocks.ramps.size();
        ++block_index) {
      block_description_ramp& ramp = group_out.blocks.ramps[block_index];

      for (uint8& material : ramp.surface_materials) {
         if (material >= max_material) {
            output.write("Warning! Ramp block '{}' has out of range material "
                         "index. Setting to zero.\n",
                         block_index);

            material = 0;
         }
      }
   }
}

}

auto load_entity_group_from_string(const std::string_view entity_group_data,
                                   output_stream& output) -> entity_group
{
   try {
      utility::stopwatch load_timer;
      entity_group group;

      std::vector<unlinked_branch_weight> branch_weights;
      std::vector<unlinked_connection> unlinked_connections;

      for (auto& key_node : assets::config::read_config(entity_group_data)) {
         if (string::iequals(key_node.key, "Object"sv)) {
            group.objects.emplace_back(read_object(key_node));
         }
         else if (string::iequals(key_node.key, "Light"sv)) {
            group.lights.emplace_back(read_light(key_node, output));
         }
         else if (string::iequals(key_node.key, "Path"sv)) {
            group.paths.emplace_back(read_path(key_node));
         }
         else if (string::iequals(key_node.key, "Region"sv)) {
            group.regions.emplace_back(read_region(key_node, output));
         }
         else if (string::iequals(key_node.key, "Sector"sv)) {
            group.sectors.emplace_back(read_sector(key_node));
         }
         else if (string::iequals(key_node.key, "Portal"sv)) {
            group.portals.emplace_back(read_portal(key_node));
         }
         else if (string::iequals(key_node.key, "Hint"sv)) {
            group.hintnodes.emplace_back(read_hintnode(key_node));
         }
         else if (string::iequals(key_node.key, "Barrier"sv)) {
            group.barriers.emplace_back(read_barrier(key_node));
         }
         else if (string::iequals(key_node.key, "Hub"sv)) {
            group.planning_hubs.emplace_back(read_hub(key_node, branch_weights));
         }
         else if (string::iequals(key_node.key, "Connection"sv)) {
            unlinked_connections.emplace_back(read_connection(key_node));
         }
         else if (string::iequals(key_node.key, "Boundary"sv)) {
            group.boundaries.emplace_back(read_boundary(key_node));
         }
         else if (string::iequals(key_node.key, "Measurement"sv)) {
            group.measurements.emplace_back(read_measurement(key_node));
         }
         else if (string::iequals(key_node.key, "BlocksBoxes"sv)) {
            group.blocks.boxes.reserve(key_node.values.get<uint32>(0));

            read_blocks_boxes(key_node, group);
         }
         else if (string::iequals(key_node.key, "BlocksRamps"sv)) {
            group.blocks.ramps.reserve(key_node.values.get<uint32>(0));

            read_blocks_ramps(key_node, group);
         }
         else if (string::iequals(key_node.key, "BlocksMaterials"sv)) {
            group.blocks.materials.reserve(key_node.values.get<uint32>(0));

            read_blocks_materials(key_node, group);
         }
      }

      group.planning_connections = link_connections(std::move(unlinked_connections),
                                                    group.planning_hubs, output);

      add_branch_weights(branch_weights, group, output);

      clamp_block_material_indices(group, output);

      output.write("Loaded entity group (time taken {:f}ms)\n",
                   load_timer.elapsed_ms());

      return group;
   }
   catch (std::exception& e) {
      output.write("Error while loading entity group:\n   Message: \n{}\n",
                   string::indent(2, e.what()));

      throw load_failure{e.what()};
   }
}

auto load_entity_group(const io::path& path, output_stream& output) -> entity_group
{
   std::string file;

   try {
      utility::stopwatch load_timer;

      file = io::read_file_to_string(path);

      output.write("Loaded {} (time taken {:f}ms)\n", path.string_view(),
                   load_timer.elapsed_ms());
   }
   catch (io::error& e) {
      output.write("Error while loading entity group:\n   Entity Group: {}\n   "
                   "Message: \n{}\n",
                   path.string_view(), string::indent(2, e.what()));

      throw load_failure{e.what()};
   }

   return load_entity_group_from_string(file, output);
}

}