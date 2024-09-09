#include "load_entity_group.hpp"
#include "load_failure.hpp"

#include "assets/config/io.hpp"
#include "io/error.hpp"
#include "io/read_file.hpp"
#include "utility/stopwatch.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

using namespace std::string_view_literals;

namespace we::world {

namespace {

auto read_rotation(const assets::config::node& node) -> quaternion
{
   quaternion rotation{node.values.get<float>(0), node.values.get<float>(1),
                       node.values.get<float>(2), node.values.get<float>(3)};

   rotation.x = -rotation.x;
   rotation.z = -rotation.z;

   std::swap(rotation.x, rotation.z);
   std::swap(rotation.y, rotation.w);

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

}

auto load_entity_group_from_string(const std::string_view entity_group_data,
                                   output_stream& output) -> entity_group
{
   try {
      utility::stopwatch load_timer;
      entity_group group;

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
      }

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

auto load_entity_group(const std::filesystem::path& path, output_stream& output) -> entity_group
{
   std::string file;

   try {
      utility::stopwatch load_timer;

      file = io::read_file_to_string(path);

      output.write("Loaded {} (time taken {:f}ms)\n", path.string(),
                   load_timer.elapsed_ms());
   }
   catch (io::error& e) {
      output.write("Error while loading entity group:\n   Entity Group: {}\n   "
                   "Message: \n{}\n",
                   path.string(), string::indent(2, e.what()));

      throw load_failure{e.what()};
   }

   return load_entity_group_from_string(file, output);
}

}