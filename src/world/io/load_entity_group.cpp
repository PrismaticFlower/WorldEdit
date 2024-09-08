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

}

auto load_entity_group_from_string(const std::string_view entity_group_data,
                                   output_stream& output) -> entity_group
{
   try {
      utility::stopwatch load_timer;
      entity_group group;

      for (auto& key_node : assets::config::read_config(entity_group_data)) {
         if (key_node.key == "Object"sv) {
            auto& object = group.objects.emplace_back();

            object.name = key_node.values.get<std::string>(0);
            object.class_name =
               lowercase_string{key_node.values.get<std::string>(1)};

            for (auto& obj_prop : key_node) {
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
                     {.key = obj_prop.key,
                      .value = obj_prop.values.get<std::string>(0)});
               }
            }
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