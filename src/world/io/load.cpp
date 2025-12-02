
#include "load.hpp"
#include "layer_remap.hpp"
#include "load_blocks.hpp"
#include "load_effects.hpp"
#include "load_failure.hpp"

#include "assets/config/io.hpp"
#include "assets/req/io.hpp"
#include "assets/terrain/terrain_io.hpp"

#include "io/read_file.hpp"

#include "math/vector_funcs.hpp"

#include "utility/stopwatch.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include <algorithm>
#include <cmath>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <fmt/core.h>

using namespace std::literals;

namespace we::world {

namespace {

// This can provide more clues as to when something went wrong with loading but also makes it 50x to 100x slower.
constexpr bool verbose_output = false;

void throw_layer_load_failure(std::string_view type, const io::path& filepath,
                              std::exception& e)
{
   throw load_failure{fmt::format("Failed to load layer {}.\n   "
                                  "File: {}\n   Message: \n{}\n",
                                  type, filepath.string_view(),
                                  string::indent(2, e.what()))};
}

void throw_planning_missing_hub(std::string_view hub, std::string_view connection)
{
   throw load_failure{fmt::format("Failed to path planning. A hub referenced "
                                  "by a connection was missing!.\n   "
                                  "Hub: {}\n   Connection: {}\n",
                                  hub, connection)};
}

template<typename T>
void check_space(std::string_view name, pinned_vector<T>& vec)
{
   if (vec.size() == vec.max_size()) {
      throw load_failure{
         fmt::format("Too many {} for WorldEdit to handle. \n   "
                     "Max Supported Count: {}\n",
                     name, vec.max_size())};
   }
}

auto read_layer_index(const assets::config::node& node,
                      const layer_remap& layer_remap) -> int8
{
   if (const auto layer_it = node.find("Layer"sv); layer_it != node.cend()) {
      return layer_remap[layer_it->values.get<int>(0)];
   }

   return 0;
}

auto read_location(const assets::config::node& node, const std::string_view rotation_key,
                   const std::string_view position_key) -> std::pair<quaternion, float3>
{
   quaternion rotation{node.at(rotation_key).values.get<float>(0),
                       node.at(rotation_key).values.get<float>(1),
                       node.at(rotation_key).values.get<float>(2),
                       node.at(rotation_key).values.get<float>(3)};

   rotation.x = -rotation.x;
   rotation.z = -rotation.z;

   std::swap(rotation.x, rotation.z);
   std::swap(rotation.y, rotation.w);

   if (rotation.w == 0.0f and rotation.x == 0.0f and rotation.y == 0.0f and
       rotation.z == 0.0f) {
      rotation = {};
   }

   return {rotation,
           {node.at(position_key).values.get<float>(0),
            node.at(position_key).values.get<float>(1),
            -node.at(position_key).values.get<float>(2)}};
}

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

auto read_animation_transition(const int transition, std::string_view name,
                               output_stream& output) -> animation_transition
{
   if (transition == 0) return animation_transition::pop;
   if (transition == 1) return animation_transition::linear;
   if (transition == 2) return animation_transition::spline;

   output.write("Warning! Animation '{}' has a key with invalid transition! "
                "Defaulting to linear.\n",
                name);

   return animation_transition::linear;
}

auto load_layer_index(const io::path& path, output_stream& output, world& world_out)
   -> layer_remap
{
   using namespace assets;

   try {
      config::node layer_index = config::read_config(io::read_file_to_string(path));

      if (not std::any_of(layer_index.cbegin(), layer_index.cend(),
                          [](const config::key_node& node) {
                             return node.key == "Layer"sv and
                                    node.values.get<std::string_view>(0) == "[Base]"sv and
                                    node.values.get<int>(1) == 0;
                          })) {
         throw load_failure{
            fmt::format("Failed to load layer index.\n   File: {}\n   Layer "
                        "index did "
                        "not contain a valid entry for the [Base] layer!\n",
                        path.string_view())};
      }

      layer_remap layer_remap;

      world_out.layer_descriptions.reserve(8);
      world_out.layer_descriptions.push_back({.name = "[Base]"s});

      for (const auto& key_node : layer_index) {
         if (key_node.key == "Layer"sv) {
            if (key_node.values.get<std::string_view>(0) == "[Base]"sv) {
               continue;
            }

            if (world_out.layer_descriptions.size() == max_layers) {
               throw load_failure{
                  fmt::format("Failed to load layer index.\n   "
                              "   Message:\n      Too many layers! Max "
                              "Supported Layers: {}\n",
                              max_layers)};
            }

            const int index = key_node.values.get<int>(1);

            if (index < 0 or index >= max_layers) {
               throw load_failure{
                  fmt::format("Failed to load layer index.\n   "
                              "   Message:\n      Layer {} has unsupported "
                              "index: {}. Max Supported Index: {}\n",
                              key_node.values.get<std::string_view>(0), index,
                              max_layers - 1)};
            }

            layer_remap.set(index,
                            static_cast<int8>(world_out.layer_descriptions.size()));

            world_out.layer_descriptions.push_back(
               {.name = key_node.values.get<std::string>(0)});

            output.write("Found world layer '{}' in .ldx file\n",
                         world_out.layer_descriptions.back().name);
         }
         else if (key_node.key == "GameMode"sv) {
            std::string name = key_node.values.get<std::string>(0);

            if (string::iequals(name, "Common")) {
               for (const auto& game_mode_key_node : key_node) {
                  if (game_mode_key_node.key != "Layer"sv) continue;

                  world_out.common_layers.push_back(
                     game_mode_key_node.values.get<int>(0));
               }
            }
            else {
               auto& game_mode = world_out.game_modes.emplace_back();

               game_mode.name = std::move(name);

               for (const auto& game_mode_key_node : key_node) {
                  if (game_mode_key_node.key != "Layer"sv) continue;

                  game_mode.layers.push_back(game_mode_key_node.values.get<int>(0));
               }

               output.write("Found game_mode '{}' in .ldx file\n", game_mode.name);
            }
         }
      }

      for (int& layer : world_out.common_layers) layer = layer_remap[layer];

      for (auto& game_mode : world_out.game_modes) {
         for (auto& layer : game_mode.layers) {
            layer = layer_remap[layer];
         }
      }

      if (world_out.layer_descriptions.size() > max_layers) {
         throw load_failure{fmt::format(
            "Failed to load layer index.\n   File: {}\n   Too many layers!\n   "
            "   Max Supported: {}\n      World Count: {}\n",
            path.string_view(), max_layers, world_out.layer_descriptions.size())};
      }

      return layer_remap;
   }
   catch (std::exception& e) {
      throw load_failure{fmt::format("Failed to layer index.\n   "
                                     "File: {}\n   Message: {}\n",
                                     path.string_view(), string::indent(2, e.what()))};
   }
}

void load_objects(const io::path& path, const std::string_view layer_name,
                  output_stream& output, world& world_out, const layer_remap& layer_remap)
{
   using namespace assets;

   utility::stopwatch load_timer;

   try {
      for (auto& key_node : config::read_config(io::read_file_to_string(path))) {
         if (key_node.key != "Object"sv) continue;

         check_space("objects", world_out.objects);

         auto& object = world_out.objects.emplace_back();

         object.name = key_node.values.get<std::string>(0);
         object.class_name = lowercase_string{key_node.values.get<std::string>(1)};
         std::tie(object.rotation, object.position) =
            read_location(key_node, "ChildRotation"sv, "ChildPosition"sv);
         object.id = world_out.next_id.objects.aquire();

         for (auto& obj_prop : key_node) {
            if (obj_prop.key == "ChildRotation"sv or obj_prop.key == "ChildPosition"sv) {
               continue;
            }
            else if (obj_prop.key == "Team"sv) {
               object.team = obj_prop.values.get<int>(0);
            }
            else if (obj_prop.key == "Layer"sv) {
               object.layer = layer_remap[obj_prop.values.get<int>(0)];
            }
            else if (obj_prop.key == "SeqNo"sv or obj_prop.key == "NetworkId"sv) {
               continue;
            }
            else {
               object.instance_properties.push_back(
                  {.key = obj_prop.key, .value = obj_prop.values.get<std::string>(0)});
            }
         }

         if (verbose_output) {
            output.write("Loaded world object '{}' with class '{}'\n",
                         object.name, object.class_name);
         }
      }
   }
   catch (std::exception& e) {
      throw_layer_load_failure("objects", path.string_view(), e);
   }

   output.write("Loaded layer '{}' objects (time taken {:f}ms)\n", layer_name,
                load_timer.elapsed_ms());
}

void load_lights(const io::path& path, const std::string_view layer_name,
                 output_stream& output, world& world_out, const int8 layer)
{
   using namespace assets;

   utility::stopwatch load_timer;

   try {
      for (auto& key_node : config::read_config(io::read_file_to_string(path))) {
         if (key_node.key == "Light"sv) {
            check_space("lights", world_out.lights);

            auto& light = world_out.lights.emplace_back();

            light.name = key_node.values.get<std::string>(0);
            light.layer = layer;
            std::tie(light.rotation, light.position) =
               read_location(key_node, "Rotation"sv, "Position"sv);

            switch (const auto type = key_node.at("Type"sv).values.get<int>(0); type) {
            case 1:
            case 2:
            case 3:
               light.light_type = static_cast<light_type>(type);
               break;
            default:
               output.write("Warning! World light '{}' has invalid light type! "
                            "Defaulting to point light.\n",
                            light.name);
               light.light_type = light_type::point;
            }

            light.color = {key_node.at("Color"sv).values.get<float>(0),
                           key_node.at("Color"sv).values.get<float>(1),
                           key_node.at("Color"sv).values.get<float>(2)};

            light.static_ = key_node.contains("Static"sv);
            light.shadow_caster = key_node.contains("CastShadow"sv);
            light.specular_caster = key_node.contains("CastSpecular"sv);
            light.id = world_out.next_id.lights.aquire();

            if (auto texture = key_node.find("Texture"sv); texture != key_node.cend()) {
               light.texture = texture->values.get<std::string>(0);

               if (texture->values.size() >= 2) {
                  switch (const auto addressing = texture->values.get<int>(1);
                          addressing) {
                  case 0:
                  case 1:
                     light.texture_addressing =
                        static_cast<texture_addressing>(addressing);
                     break;
                  default:
                     output
                        .write("Warning! World light '{}' has invalid texture "
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

               light.texture = texture->values.get<std::string>(0);
            }

            if (auto tile_uv = key_node.find("TileUV"sv); tile_uv != key_node.cend()) {
               light.directional_texture_tiling = {tile_uv->values.get<float>(0),
                                                   tile_uv->values.get<float>(1)};
            }

            if (auto offset_uv = key_node.find("OffsetUV"sv);
                offset_uv != key_node.cend()) {
               light.directional_texture_offset = {offset_uv->values.get<float>(0),
                                                   offset_uv->values.get<float>(1)};
            }

            if (auto region = key_node.find("Region"sv); region != key_node.cend()) {
               light.region_name = region->values.get<std::string>(0);
            }

            if (auto range = key_node.find("Range"sv); range != key_node.cend()) {
               light.range = range->values.get<float>(0);
            }

            if (auto cone = key_node.find("Cone"sv); cone != key_node.cend()) {
               light.inner_cone_angle = cone->values.get<float>(0);
               light.outer_cone_angle = cone->values.get<float>(1);
            }

            if (auto bidirectional = key_node.find("Bidirectional"sv);
                bidirectional != key_node.cend()) {
               light.bidirectional = bidirectional->values.get<int>(0) != 0;
            }

            if (auto blend_mode = key_node.find("PS2BlendMode"sv);
                blend_mode != key_node.cend()) {
               switch (int mode = blend_mode->values.get<int>(0); mode) {
               case 0:
               case 1:
               case 2:
                  light.ps2_blend_mode = static_cast<ps2_blend_mode>(mode);
                  break;
               default:
                  output.write(
                     "Warning! World light '{}' has invalid PS2 Blend Mode! "
                     "Defaulting to add.\n",
                     light.name);
                  light.ps2_blend_mode = ps2_blend_mode::add;
               }
            }

            if (verbose_output) {
               output.write("Loaded world light '{}'\n", light.name);
            }
         }
         else if (key_node.key == "GlobalLights"sv) {
            world_out.global_lights.global_light_1 =
               key_node.at("Light1"sv).values.get<std::string>(0);
            world_out.global_lights.global_light_2 =
               key_node.at("Light2"sv).values.get<std::string>(0);
            world_out.global_lights.ambient_sky_color =
               float3{key_node.at("Top"sv).values.get<float>(0) / 255.0f,
                      key_node.at("Top"sv).values.get<float>(1) / 255.0f,
                      key_node.at("Top"sv).values.get<float>(2) / 255.0f};
            world_out.global_lights.ambient_ground_color =
               float3{key_node.at("Bottom"sv).values.get<float>(0) / 255.0f,
                      key_node.at("Bottom"sv).values.get<float>(1) / 255.0f,
                      key_node.at("Bottom"sv).values.get<float>(2) / 255.0f};

            if (auto env_map = key_node.find("EnvMap"sv); env_map != key_node.cend()) {
               world_out.global_lights.env_map_texture =
                  env_map->values.get<std::string>(0);
            }
         }
      }
   }
   catch (std::exception& e) {
      throw_layer_load_failure("lights", path.string_view(), e);
   }

   output.write("Loaded layer '{}' lights (time taken {:f}ms)\n", layer_name,
                load_timer.elapsed_ms());
}

void load_paths(const io::path& filepath, const std::string_view layer_name,
                output_stream& output, world& world_out, const layer_remap& layer_remap)
{
   using namespace assets;

   utility::stopwatch load_timer;

   try {
      for (auto& key_node : config::read_config(io::read_file_to_string(filepath))) {
         if (key_node.key != "Path"sv) continue;

         check_space("paths", world_out.paths);

         auto& path = world_out.paths.emplace_back();

         path.name = key_node.values.get<std::string>(0);
         path.id = world_out.next_id.paths.aquire();

         if (path.name.starts_with("type_")) {
            const lowercase_string lowercase_name{path.name};

            if (lowercase_name.starts_with("type_entitypath")) {
               path.type = path_type::entity_follow;
            }
            else if (lowercase_name.starts_with("type_entityformation")) {
               path.type = path_type::formation;
            }
            else if (lowercase_name.starts_with("type_patrolpath")) {
               path.type = path_type::patrol;
            }

            path.name = string::split_first_of_exclusive(path.name, " ")[1];
         }

         for (auto& child_key_node : key_node) {
            if (child_key_node.key == "Layer"sv) {
               path.layer = layer_remap[child_key_node.values.get<int>(0)];
            }
            else if (child_key_node.key == "Properties"sv) {
               path.properties = read_path_properties(child_key_node);
            }
            else if (child_key_node.key == "SplineType"sv) {
               if (const auto spline = child_key_node.values.get<std::string_view>(0);
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
            else if (child_key_node.key == "Nodes"sv) {
               auto& path_nodes = key_node.at("Nodes"sv);
               path.nodes.reserve(path_nodes.values.get<std::size_t>(0));

               for (auto& node : path_nodes) {
                  auto& path_node = path.nodes.emplace_back();

                  for (auto& node_child : node) {
                     if (node_child.key == "Rotation"sv) {
                        path_node.rotation = read_rotation(node_child);
                     }
                     else if (node_child.key == "Position"sv) {
                        path_node.position = read_position(node_child);
                     }
                     else if (node_child.key == "Properties"sv) {
                        path_node.properties = read_path_properties(node_child);
                     }
                  }
               }

               if (path.nodes.size() >= max_path_nodes) {
                  throw load_failure{fmt::format("Path '{}' has too many nodes "
                                                 "for WorldEdit to handle.\n"
                                                 "   Path Node Count: {}\n",
                                                 "   Max Supported Count: {}\n",
                                                 path.name, path.nodes.size(),
                                                 max_path_nodes)};
               }
            }
         }

         if (verbose_output) {
            output.write("Loaded world path '{}'\n", path.name);
         }
      }
   }
   catch (std::exception& e) {
      throw_layer_load_failure("paths", filepath.string_view(), e);
   }

   output.write("Loaded layer '{}' paths (time taken {:f}ms)\n", layer_name,
                load_timer.elapsed_ms());
}

void load_regions(const io::path& filepath, const std::string_view layer_name,
                  output_stream& output, world& world_out, const layer_remap& layer_remap)
{
   using namespace assets;

   utility::stopwatch load_timer;

   try {
      for (auto& key_node : config::read_config(io::read_file_to_string(filepath))) {
         if (key_node.key != "Region"sv) continue;

         check_space("regions", world_out.regions);

         auto& region = world_out.regions.emplace_back();

         region.name = key_node.contains("Name"sv)
                          ? key_node.at("Name"sv).values.get<std::string>(0)
                          : ""s;
         region.layer = read_layer_index(key_node, layer_remap);
         std::tie(region.rotation, region.position) =
            read_location(key_node, "Rotation"sv, "Position"sv);
         region.size = {key_node.at("Size"sv).values.get<float>(0),
                        key_node.at("Size"sv).values.get<float>(1),
                        key_node.at("Size"sv).values.get<float>(2)};
         region.description = key_node.values.get<std::string>(0);
         region.id = world_out.next_id.regions.aquire();

         switch (const int shape = key_node.values.get<int>(1); shape) {
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

         if (verbose_output) {
            output.write("Loaded world region '{}'\n", region.name);
         }
      }
   }
   catch (std::exception& e) {
      throw_layer_load_failure("regions", filepath.string_view(), e);
   }

   output.write("Loaded layer '{}' regions (time taken {:f}ms)\n", layer_name,
                load_timer.elapsed_ms());
}

void load_portals_sectors(const io::path& filepath, output_stream& output, world& world_out)
{
   using namespace assets;

   utility::stopwatch load_timer;

   try {
      for (auto& key_node : config::read_config(io::read_file_to_string(filepath))) {
         if (key_node.key == "Sector"sv) {
            check_space("sectors", world_out.sectors);

            auto& sector = world_out.sectors.emplace_back();

            sector.name = key_node.values.get<std::string>(0);
            sector.id = world_out.next_id.sectors.aquire();

            for (auto& sector_prop : key_node) {
               if (sector_prop.key == "Base"sv) {
                  sector.base = sector_prop.values.get<float>(0);
               }
               else if (sector_prop.key == "Height"sv) {
                  sector.height = sector_prop.values.get<float>(0);
               }
               else if (sector_prop.key == "Point"sv) {
                  sector.points.push_back({sector_prop.values.get<float>(0),
                                           -sector_prop.values.get<float>(1)});
               }
               else if (sector_prop.key == "Object"sv) {
                  sector.objects.emplace_back(sector_prop.values.get<std::string>(0));
               }
            }

            if (verbose_output) {
               output.write("Loaded world sector '{}'\n", sector.name);
            }
         }
         else if (key_node.key == "Portal"sv) {
            check_space("portals", world_out.portals);

            auto& portal = world_out.portals.emplace_back();

            portal.name = key_node.values.get<std::string>(0);
            std::tie(portal.rotation, portal.position) =
               read_location(key_node, "Rotation"sv, "Position"sv);
            portal.width = key_node.at("Width"sv).values.get<float>(0);
            portal.height = key_node.at("Height"sv).values.get<float>(0);
            portal.id = world_out.next_id.portals.aquire();

            if (auto sector = key_node.find("Sector1"sv); sector != key_node.cend()) {
               portal.sector1 = sector->values.get<std::string>(0);
            }

            if (auto sector = key_node.find("Sector2"sv); sector != key_node.cend()) {
               portal.sector2 = sector->values.get<std::string>(0);
            }

            if (verbose_output) {
               output.write("Loaded world portal '{}'\n", portal.name);
            }
         }
      }
   }
   catch (std::exception& e) {
      throw_layer_load_failure("portals and sectors", filepath.string_view(), e);
   }

   output.write("Loaded world portals and sectors (time taken {:f}ms)\n",
                load_timer.elapsed_ms());
}

void load_barriers(const io::path& filepath, output_stream& output, world& world_out)
{
   using namespace assets;

   utility::stopwatch load_timer;

   try {
      for (auto& key_node : config::read_config(io::read_file_to_string(filepath))) {
         if (key_node.key != "Barrier"sv) continue;

         check_space("barriers", world_out.barriers);

         auto& barrier = world_out.barriers.emplace_back();

         barrier.name = key_node.values.get<std::string>(0);
         barrier.flags = ai_path_flags{key_node.at("Flag"sv).values.get<int>(0)};
         barrier.id = world_out.next_id.barriers.aquire();

         const auto is_corner = [](const config::key_node& child_key_node) {
            return child_key_node.key == "Corner"sv;
         };

         auto corner = std::find_if(key_node.cbegin(), key_node.cend(), is_corner);

         std::array<float3, 4> corners;

         for (int i = 0; i < 4; ++i) {
            if (corner != key_node.cend()) {
               corners[i] = {corner->values.get<float>(0),
                             corner->values.get<float>(1),
                             -corner->values.get<float>(2)};

               corner = std::find_if(corner + 1, key_node.cend(), is_corner);
            }
            else {
               output.write("Warning! World barrier '{}' is missing one or "
                            "more corners!\n",
                            barrier.name);
               break;
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

         if (verbose_output) {
            output.write("Loaded world barrier '{}'\n", barrier.name);
         }
      }
   }
   catch (std::exception& e) {
      throw_layer_load_failure("barriers", filepath.string_view(), e);
   }

   output.write("Loaded world barriers (time taken {:f}ms)\n",
                load_timer.elapsed_ms());
}

void load_planning(const io::path& filepath, output_stream& output, world& world_out)
{
   using namespace assets;

   utility::stopwatch load_timer;

   struct branch_weight {
      std::string start_hub;
      std::string end_hub;
      float weight = 0.0f;
      std::string connection;
      ai_path_flags flag;
   };

   std::vector<branch_weight> branch_weights;
   branch_weights.reserve(1024);

   try {
      const config::node planning =
         config::read_config(io::read_file_to_string(filepath));

      for (auto& key_node : planning) {
         if (key_node.key == "Hub"sv) {
            check_space("AI planning hubs", world_out.planning_hubs);

            planning_hub& hub = world_out.planning_hubs.emplace_back();

            hub.name = key_node.values.get<std::string>(0);
            hub.id = world_out.next_id.planning_hubs.aquire();

            for (auto& child_key_node : key_node) {
               if (child_key_node.key == "Pos"sv) {
                  hub.position = {child_key_node.values.get<float>(0),
                                  child_key_node.values.get<float>(1),
                                  -child_key_node.values.get<float>(2)};
               }
               else if (child_key_node.key == "Radius"sv) {
                  hub.radius = {child_key_node.values.get<float>(0)};
               }
               else if (child_key_node.key == "BranchWeight"sv) {
                  branch_weights.push_back(
                     branch_weight{.start_hub = hub.name,
                                   .end_hub = child_key_node.values.get<std::string>(0),
                                   .weight = child_key_node.values.get<float>(1),
                                   .connection =
                                      child_key_node.values.get<std::string>(2),
                                   .flag = static_cast<ai_path_flags>(
                                      child_key_node.values.get<int>(3))});
               };

               if (verbose_output) {
                  output.write("Loaded world planning hub '{}'\n", hub.name);
               }
            }
         }
      }

      absl::flat_hash_map<std::string_view, uint32> hub_map;
      hub_map.reserve(world_out.planning_hubs.size());

      for (uint32 i = 0; i < world_out.planning_hubs.size(); ++i) {
         const planning_hub& hub = world_out.planning_hubs[i];

         hub_map.emplace(hub.name, i);
      }

      for (auto& key_node : planning) {
         if (key_node.key == "Connection") {
            check_space("AI planning connections", world_out.planning_connections);

            planning_connection& connection =
               world_out.planning_connections.emplace_back();

            connection.name = key_node.values.get<std::string>(0);
            connection.id = world_out.next_id.planning_connections.aquire();

            for (auto& child_key_node : key_node) {
               if (child_key_node.key == "Start"sv) {
                  const std::string_view hub_name =
                     child_key_node.values.get<std::string_view>(0);

                  auto hub_it = hub_map.find(hub_name);

                  if (hub_it == hub_map.end()) {
                     throw_planning_missing_hub(hub_name, connection.name);
                  }

                  connection.start_hub_index = hub_it->second;
               }
               else if (child_key_node.key == "End"sv) {
                  const std::string_view hub_name =
                     child_key_node.values.get<std::string_view>(0);

                  auto hub_it = hub_map.find(hub_name);

                  if (hub_it == hub_map.end()) {
                     throw_planning_missing_hub(hub_name, connection.name);
                  }

                  connection.end_hub_index = hub_it->second;
               }
               else if (child_key_node.key == "Flag"sv) {
                  connection.flags =
                     static_cast<ai_path_flags>(child_key_node.values.get<int>(0));
               }
               else if (child_key_node.key == "Dynamic"sv) {
                  connection.dynamic_group = child_key_node.values.get<int8>(0);
               }
               else if (child_key_node.key == "Jump"sv) {
                  connection.jump = true;
               }
               else if (child_key_node.key == "JetJump"sv) {
                  connection.jet_jump = true;
               }
               else if (child_key_node.key == "OneWay"sv) {
                  connection.one_way = true;
               }

               if (verbose_output) {
                  output.write("Loaded world planning connection '{}'\n",
                               connection.name);
               }
            }
         }
      }

      absl::flat_hash_map<std::string_view, uint32> connection_map;
      connection_map.reserve(world_out.planning_connections.size());

      for (uint32 i = 0; i < world_out.planning_connections.size(); ++i) {
         const planning_connection& connection = world_out.planning_connections[i];

         connection_map.emplace(connection.name, i);
      }

      for (auto& branch_weight : branch_weights) {
         const uint32 start_hub_index = hub_map[branch_weight.start_hub];

         const auto end_hub_iter = hub_map.find(branch_weight.end_hub);

         if (end_hub_iter == hub_map.end()) {
            output.write("BranchWeight for hub '{}' is invalid. Can "
                         "not find referenced hub '{}'.\n",
                         branch_weight.start_hub, branch_weight.end_hub);

            continue;
         }

         const auto connection_iter = connection_map.find(branch_weight.connection);

         if (connection_iter == connection_map.end()) {
            output.write("BranchWeight for hub '{}' is invalid. Can "
                         "not find referenced connection '{}'.\n",
                         branch_weight.start_hub, branch_weight.connection);

            continue;
         }

         const uint32 end_hub_index = end_hub_iter->second;
         const uint32 connection_index = connection_iter->second;

         const planning_connection& connection =
            world_out.planning_connections[connection_index];
         planning_hub& hub = world_out.planning_hubs[start_hub_index];

         if (connection.start_hub_index != start_hub_index and
             connection.end_hub_index != start_hub_index) {
            output.write("BranchWeight for hub '{}' is invalid. Referenced "
                         "connection '{}' is not joined to the hub.\n",
                         hub.name, connection.name);

            continue;
         }

         planning_branch_weights& weights =
            [&hub, end_hub_index,
             connection_index]() noexcept -> planning_branch_weights& {
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
   catch (std::exception& e) {
      throw_layer_load_failure("planning", filepath.string_view(), e);
   }

   output.write("Loaded world AI planning (time taken {:f}ms)\n",
                load_timer.elapsed_ms());
}

void load_boundaries(const io::path& filepath, output_stream& output, world& world_out)
{
   using namespace assets;

   utility::stopwatch load_timer;

   try {
      for (auto& key_node : config::read_config(io::read_file_to_string(filepath))) {
         if (key_node.key != "Boundary"sv) continue;

         for (auto& child_key_node : key_node) {
            if (child_key_node.key != "Path"sv) continue;

            check_space("boundaries", world_out.boundaries);

            auto& boundary = world_out.boundaries.emplace_back();

            boundary.name = child_key_node.values.get<std::string>(0);
            boundary.id = world_out.next_id.boundaries.aquire();

            if (verbose_output) {
               output.write("Loaded world boundary '{}'\n", boundary.name);
            }
         }
      }
   }
   catch (std::exception& e) {
      throw_layer_load_failure("boundaries", filepath.string_view(), e);
   }

   output.write("Loaded world boundaries (time taken {:f}ms)\n",
                load_timer.elapsed_ms());
}

void load_hintnodes(const io::path& filepath, const std::string_view layer_name,
                    output_stream& output, world& world_out, const int8 layer)
{
   using namespace assets;

   utility::stopwatch load_timer;

   try {
      for (auto& key_node : config::read_config(io::read_file_to_string(filepath))) {
         if (key_node.key != "Hint"sv) continue;

         check_space("hint nodes", world_out.hintnodes);

         auto& hint = world_out.hintnodes.emplace_back();

         hint.name = key_node.values.get<std::string>(0);
         hint.layer = layer;
         hint.type = static_cast<hintnode_type>(
            std::stol(key_node.values.get<std::string>(1)));
         std::tie(hint.rotation, hint.position) =
            read_location(key_node, "Rotation"sv, "Position"sv);
         hint.id = world_out.next_id.hintnodes.aquire();

         for (auto& prop : key_node) {
            if (prop.key == "Radius"sv) {
               hint.radius = prop.values.get<float>(0);
            }
            else if (prop.key == "PrimaryStance"sv) {
               hint.primary_stance =
                  static_cast<stance_flags>(prop.values.get<int>(0));
            }
            else if (prop.key == "SecondaryStance"sv) {
               hint.secondary_stance =
                  static_cast<stance_flags>(prop.values.get<int>(0));
            }
            else if (prop.key == "Mode"sv) {
               hint.mode = static_cast<hintnode_mode>(prop.values.get<int>(0));
            }
            else if (prop.key == "CommandPost"sv) {
               hint.command_post = prop.values.get<std::string>(0);
            }
         }

         if (verbose_output) {
            output.write("Loaded world hint node '{}'\n", hint.name);
         }
      }
   }
   catch (std::exception& e) {
      throw_layer_load_failure("hint nodes", filepath.string_view(), e);
   }

   output.write("Loaded layer '{}' hint nodes (time taken {:f}ms)\n",
                layer_name, load_timer.elapsed_ms());
}

void load_animations(const io::path& filepath, output_stream& output, world& world_out)
{
   using namespace assets;

   utility::stopwatch load_timer;

   try {
      for (auto& key_node : config::read_config(io::read_file_to_string(filepath))) {
         if (key_node.key == "Animation"sv) {
            check_space("animations", world_out.animations);

            auto& animation = world_out.animations.emplace_back();

            animation.name = key_node.values.get<std::string>(0);
            animation.runtime = key_node.values.get<float>(1);
            animation.loop = key_node.values.get<int>(2) != 0;
            animation.local_translation = key_node.values.get<int>(3) != 0;
            animation.id = world_out.next_id.animations.aquire();

            for (auto& child_key_node : key_node) {
               if (child_key_node.key == "AddPositionKey"sv) {
                  animation.position_keys.push_back({
                     .time = child_key_node.values.get<float>(0),

                     .position = {child_key_node.values.get<float>(1),
                                  child_key_node.values.get<float>(2),
                                  child_key_node.values.get<float>(3)},

                     .transition =
                        read_animation_transition(child_key_node.values.get<int>(4),
                                                  animation.name, output),

                     .tangent = {child_key_node.values.get<float>(5),
                                 child_key_node.values.get<float>(6),
                                 child_key_node.values.get<float>(7)},

                     .tangent_next = {child_key_node.values.get<float>(8),
                                      child_key_node.values.get<float>(9),
                                      child_key_node.values.get<float>(10)},
                  });
               }
               else if (child_key_node.key == "AddRotationKey"sv) {
                  animation.rotation_keys.push_back({
                     .time = child_key_node.values.get<float>(0),

                     .rotation = {child_key_node.values.get<float>(1),
                                  child_key_node.values.get<float>(2),
                                  child_key_node.values.get<float>(3)},

                     .transition =
                        read_animation_transition(child_key_node.values.get<int>(4),
                                                  animation.name, output),

                     .tangent = {child_key_node.values.get<float>(5),
                                 child_key_node.values.get<float>(6),
                                 child_key_node.values.get<float>(7)},

                     .tangent_next = {child_key_node.values.get<float>(8),
                                      child_key_node.values.get<float>(9),
                                      child_key_node.values.get<float>(10)},
                  });
               }
            }

            if (verbose_output) {
               output.write("Loaded animation '{}'\n", animation.name);
            }
         }
         else if (key_node.key == "AnimationGroup"sv) {
            check_space("animation groups", world_out.animation_groups);

            auto& group = world_out.animation_groups.emplace_back();

            group.name = key_node.values.get<std::string>(0);
            group.play_when_level_begins = key_node.values.get<int>(1) != 0;
            group.stops_when_object_is_controlled = key_node.values.get<int>(2) != 0;
            group.id = world_out.next_id.animation_groups.aquire();

            for (auto& child_key_node : key_node) {
               if (child_key_node.key == "DisableHierarchies"sv) {
                  group.disable_hierarchies = true;
               }
               else if (child_key_node.key == "Animation"sv) {
                  group.entries.push_back({
                     .animation = child_key_node.values.get<std::string>(0),
                     .object = child_key_node.values.get<std::string>(1),
                  });
               }
            }

            if (verbose_output) {
               output.write("Loaded animation group '{}'\n", group.name);
            }
         }
         else if (key_node.key == "Hierarchy"sv) {
            check_space("animation hierarchies", world_out.animation_hierarchies);

            auto& hierarchy = world_out.animation_hierarchies.emplace_back();

            hierarchy.root_object = key_node.values.get<std::string>(0);
            hierarchy.id = world_out.next_id.animation_hierarchies.aquire();

            for (auto& child_key_node : key_node) {
               if (child_key_node.key == "Obj"sv) {
                  hierarchy.objects.push_back(
                     child_key_node.values.get<std::string>(0));
               }
            }

            if (verbose_output) {
               output
                  .write("Loaded animation hierarchy with root object '{}'\n",
                         hierarchy.root_object);
            }
         }
      }
   }
   catch (std::exception& e) {
      throw_layer_load_failure("animations", filepath.string_view(), e);
   }

   output.write("Loaded world animations (time taken {:f}ms)\n",
                load_timer.elapsed_ms());
}

void load_measurements(const io::path& filepath, output_stream& output, world& world_out)
{
   using namespace assets;

   utility::stopwatch load_timer;

   try {
      for (auto& key_node : config::read_config(io::read_file_to_string(filepath))) {
         if (key_node.key == "MeasurementCount"sv) {
            world_out.measurements.reserve(world_out.measurements.size() +
                                           key_node.values.get<std::size_t>(0));
         }
         else if (key_node.key == "Measurement"sv) {
            check_space("measurements", world_out.measurements);

            auto& measurement = world_out.measurements.emplace_back();

            measurement.name = key_node.values.get<std::string>(0);
            measurement.id = world_out.next_id.measurements.aquire();

            for (auto& child_key_node : key_node) {
               if (child_key_node.key == "Start"sv) {
                  measurement.start = {child_key_node.values.get<float>(0),
                                       child_key_node.values.get<float>(1),
                                       -child_key_node.values.get<float>(2)};
               }
               else if (child_key_node.key == "End"sv) {
                  measurement.end = {child_key_node.values.get<float>(0),
                                     child_key_node.values.get<float>(1),
                                     -child_key_node.values.get<float>(2)};
               }
            }

            if (verbose_output) {
               output.write("Loaded measurement '{}'\n", measurement.name);
            }
         }
      }
   }
   catch (std::exception& e) {
      throw_layer_load_failure("measurements", filepath.string_view(), e);
   }

   output.write("Loaded world measurements (time taken {:f}ms)\n",
                load_timer.elapsed_ms());
}

void load_layer(const io::path& world_dir, const std::string_view layer_name,
                const std::string_view world_ext, output_stream& output,
                world& world_out, const layer_remap& layer_remap, const int8 layer)
{
   load_objects(io::compose_path(world_dir, layer_name, world_ext), layer_name,
                output, world_out, layer_remap);

   if (const auto paths_path = io::compose_path(world_dir, layer_name, ".pth"sv);
       io::exists(paths_path)) {
      load_paths(paths_path, layer_name, output, world_out, layer_remap);
   }

   if (const auto regions_path = io::compose_path(world_dir, layer_name, ".rgn"sv);
       io::exists(regions_path)) {
      load_regions(regions_path, layer_name, output, world_out, layer_remap);
   }

   if (const auto lights_path = io::compose_path(world_dir, layer_name, ".lgt"sv);
       io::exists(lights_path)) {
      load_lights(lights_path, layer_name, output, world_out, layer);
   }

   if (const auto hnt_path = io::compose_path(world_dir, layer_name, ".hnt"sv);
       io::exists(hnt_path)) {
      load_hintnodes(hnt_path, layer_name, output, world_out, layer);
   }

   if (layer == 0) {
      if (const auto pvs_path = io::compose_path(world_dir, layer_name, ".pvs"sv);
          io::exists(pvs_path)) {
         load_portals_sectors(pvs_path, output, world_out);
      }

      if (const auto bar_path = io::compose_path(world_dir, layer_name, ".bar"sv);
          io::exists(bar_path)) {
         load_barriers(bar_path, output, world_out);
      }

      if (const auto pln_path = io::compose_path(world_dir, layer_name, ".pln"sv);
          io::exists(pln_path)) {
         load_planning(pln_path, output, world_out);
      }

      if (const auto bnd_path = io::compose_path(world_dir, layer_name, ".bnd"sv);
          io::exists(bnd_path)) {
         load_boundaries(bnd_path, output, world_out);
      }

      if (const auto anm_path = io::compose_path(world_dir, layer_name, ".anm"sv);
          io::exists(anm_path)) {
         load_animations(anm_path, output, world_out);
      }

      if (const auto msr_path = io::compose_path(world_dir, layer_name, ".msr"sv);
          io::exists(msr_path)) {
         load_measurements(msr_path, output, world_out);
      }
   }
}

void load_requirements_files(const io::path& world_dir, world& world_out,
                             output_stream& output)
{
   if (const auto req_path = io::compose_path(world_dir, world_out.name, ".req"sv);
       io::exists(req_path)) {
      try {
         utility::stopwatch load_timer;

         world_out.requirements =
            assets::req::read(io::read_file_to_string(req_path));

         output.write("Loaded {}.req (time taken {:f}ms)\n", world_out.name,
                      load_timer.elapsed_ms());
      }
      catch (std::exception& e) {
         auto message =
            fmt::format("Error while loading {}.req:\n   Message: \n{}\n",
                        world_out.name, string::indent(2, e.what()));

         output.write(message);

         throw load_failure{message};
      }
   }

   for (std::size_t i = 0; i < world_out.game_modes.size(); ++i) {
      const std::string file_name =
         fmt::format("{}_{}", world_out.name, world_out.game_modes[i].name);

      if (const auto mrq_path = io::compose_path(world_dir, file_name, ".mrq");
          io::exists(mrq_path)) {
         try {
            utility::stopwatch load_timer;

            world_out.game_modes[i].requirements =
               assets::req::read(io::read_file_to_string(mrq_path));

            output.write("Loaded {} (time taken {:f}ms)\n", file_name,
                         load_timer.elapsed_ms());
         }
         catch (std::exception& e) {
            auto message =
               fmt::format("Error while loading {}:\n   Message: \n{}\n",
                           file_name, string::indent(2, e.what()));

            output.write(message);

            throw load_failure{message};
         }
      }
   }
}

auto load_configuration(const io::path& filepath, output_stream& output) -> configuration
{
   using namespace assets;

   utility::stopwatch load_timer;

   configuration configuration;

   try {
      for (auto& key_node : config::read_config(io::read_file_to_string(filepath))) {
         if (key_node.key == "SaveBF1Format"sv) {
            configuration.save_bf1_format = key_node.values.get<int>(0) != 0;
         }
         else if (key_node.key == "SaveEffects"sv) {
            configuration.save_effects = key_node.values.get<int>(0) != 0;
         }
         else if (key_node.key == "SaveBlocksIntoLayer"sv) {
            configuration.save_blocks_into_layer = key_node.values.get<int>(0) != 0;
         }
      }
   }
   catch (std::exception& e) {
      throw load_failure{fmt::format("Failed to load world configuration.\n   "
                                     "File: {}\n   Message: \n{}\n",
                                     filepath.string_view(),
                                     string::indent(2, e.what()))};
   }

   output.write("Loaded world configuration (time taken {:f}ms)\n",
                load_timer.elapsed_ms());

   return configuration;
}

void convert_light_regions(world& world)
{
   absl::flat_hash_map<std::string_view, region*> regions;
   regions.reserve(world.regions.size());

   for (auto& region : world.regions) {
      regions.emplace(region.description, &region);
   }

   absl::flat_hash_set<region_id> regions_to_remove;
   regions_to_remove.reserve(64);

   for (auto& light : world.lights) {
      if (light.region_name.empty()) continue;
      if (not regions.contains(light.region_name)) continue;

      region& region = *regions[light.region_name];

      light.position = regions[light.region_name]->position;
      light.region_rotation = regions[light.region_name]->rotation;
      light.region_size = regions[light.region_name]->size;

      switch (region.shape) {
      case region_shape::box:
         light.light_type = light_type::directional_region_box;
         break;
      case region_shape::sphere:
         light.light_type = light_type::directional_region_sphere;
         break;
      case region_shape::cylinder:
         light.light_type = light_type::directional_region_cylinder;
         break;
      }

      regions_to_remove.emplace(region.id);
   }

   erase_if(world.regions, [&](const region& region) {
      return regions_to_remove.contains(region.id);
   });
}

void convert_boundaries(world& world, output_stream& output)
{
   for (auto& boundary : world.boundaries) {
      auto path = std::find_if(world.paths.begin(), world.paths.end(),
                               [&](const we::world::path& path) {
                                  return path.name == boundary.name;
                               });

      if (path == world.paths.end()) {
         output.write("Warning! Boundary '{}' is missing it's path. The "
                      "boundary will not be loaded and will disappear from the "
                      "world when saved.\n");

         continue;
      }

      boundary.points.reserve(path->nodes.size());

      for (auto& node : path->nodes) {
         boundary.points.push_back(node.position);
      }

      world.paths.erase(path);
   }
}

void ensure_common_game_mode(world& world) noexcept
{
   if (not world.game_modes.empty()) return;

   world.game_modes.push_back({.name = "Common"s});

   for (int i = 0; i < world.layer_descriptions.size(); ++i) {
      world.game_modes[0].layers.push_back(i);
   }
}

void strip_blocks_layer_reference(world& world) noexcept
{
   const std::string blocks_layer_name = fmt::format("{}_{}", world.name, "WE_blocks");

   for (requirement_list& list : world.requirements) {
      if (string::iequals(list.file_type, "world") and
          list.platform == assets::req::platform::all) {
         for (std::size_t index = 0; index < list.entries.size(); ++index) {
            if (string::iequals(list.entries[index], blocks_layer_name)) {
               list.entries.erase(list.entries.begin() + index);

               return;
            }
         }
      }
   }
}

}

auto load_world(const io::path& path, const configuration& default_configuration,
                output_stream& output) -> world
{
   world world = {.name = std::string{path.stem()},
                  .configuration = default_configuration};

   const io::path world_dir = path.parent_path();

   try {
      const layer_remap layer_remap =
         load_layer_index(io::compose_path(world_dir, world.name, ".ldx"sv),
                          output, world);

      load_layer(world_dir, world.name, ".wld"sv, output, world, layer_remap, 0);

      for (std::size_t i = 1; i < world.layer_descriptions.size(); ++i) {
         auto layer = world.layer_descriptions[i];

         load_layer(world_dir, fmt::format("{}_{}", world.name, layer.name),
                    ".lyr"sv, output, world, layer_remap, static_cast<int8>(i));
      }

      convert_light_regions(world);
      convert_boundaries(world, output);
      ensure_common_game_mode(world);

      if (const auto ter_path = io::compose_path(world_dir, world.name, ".ter"sv);
          io::exists(ter_path)) {
         try {
            utility::stopwatch load_timer;

            world.terrain = read_terrain(io::read_file_to_bytes(ter_path));

            output.write("Loaded world terrain (time taken {:f}ms)\n",
                         load_timer.elapsed_ms());
         }
         catch (std::exception& e) {
            auto message =
               fmt::format("Error while loading terrain:\n   Message: \n{}\n",
                           string::indent(2, e.what()));

            output.write(message);

            throw load_failure{message};
         }
      }
      else {
         output.write(
            "World terrain file is missing. World will have default terrain.");
      }

      load_requirements_files(world_dir, world, output);

      if (const auto fx_path = io::compose_path(world_dir, world.name, ".fx"sv);
          io::exists(fx_path)) {
         try {
            utility::stopwatch load_timer;

            world.effects = load_effects(io::read_file_to_string(fx_path), output);

            output.write("Loaded {}.fx (time taken {:f}ms)\n", world.name,
                         load_timer.elapsed_ms());
         }
         catch (std::exception& e) {
            auto message =
               fmt::format("Error while loading {}.fx:\n   Message: \n{}\n",
                           world.name, string::indent(2, e.what()));

            output.write(message);

            throw load_failure{message};
         }
      }

      if (const auto blk_path = io::compose_path(world_dir, world.name, ".blk"sv);
          io::exists(blk_path)) {
         world.blocks = load_blocks(blk_path, layer_remap, output);
      }

      if (const auto configuration_path =
             io::compose_path(world_dir, world.name, ".WorldEdit"sv);
          io::exists(configuration_path)) {
         world.configuration = load_configuration(configuration_path, output);
      }

      strip_blocks_layer_reference(world);
   }
   catch (load_failure& failure) {
      output
         .write("Error while loading world:\n   World: {}\n   Message: \n{}\n",
                path.string_view(), string::indent(2, failure.what()));

      throw;
   }

   return world;
}
}
