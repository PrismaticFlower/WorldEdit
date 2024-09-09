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

void read_object(const assets::config::node& node, entity_group& group_out)
{
   object& object = group_out.objects.emplace_back();

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
}

void read_light(const assets::config::node& node, entity_group& group_out,
                output_stream& output)
{
   light& light = group_out.lights.emplace_back();

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
            read_object(key_node, group);
         }
         else if (string::iequals(key_node.key, "Light"sv)) {
            read_light(key_node, group, output);
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