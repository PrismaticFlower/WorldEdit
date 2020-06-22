
#include "world_io_load.hpp"
#include "assets/config/io.hpp"
#include "assets/terrain/terrain_io.hpp"
#include "exceptions.hpp"
#include "utility/make_range.hpp"
#include "utility/read_file.hpp"
#include "utility/srgb_conversion.hpp"
#include "utility/string_ops.hpp"

#include <algorithm>

#include <boost/container/flat_map.hpp>
#include <fmt/format.h>

using namespace std::literals;

namespace sk::world {

namespace {

using layer_remap = boost::container::flat_map<int, int, std::less<>>;

auto read_layer_index(const assets::config::node& node, layer_remap& layer_remap) -> int
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

   return {rotation,
           {node.at(position_key).values.get<float>(0),
            node.at(position_key).values.get<float>(1),
            -node.at(position_key).values.get<float>(2)}};
}

auto read_path_properties(const assets::config::node& node)
   -> std::vector<path::property>
{
   std::vector<path::property> properties;

   properties.reserve(node.values.get<std::size_t>(0));

   for (auto& prop : node) {
      properties.push_back({.key = prop.key, .value = prop.values.get<std::string>(0)});
   }

   return properties;
}

auto load_layer_index(const std::filesystem::path& path, output_stream& output,
                      world& world_out) -> layer_remap
{
   using namespace assets;

   try {
      config::node layer_index =
         config::read_config(utility::read_file_to_string(path));

      if (not std::any_of(layer_index.cbegin(), layer_index.cend(),
                          [](const config::key_node& node) {
                             return node.key == "Layer"sv and
                                    node.values.get<std::string_view>(0) == "[Base]"sv and
                                    node.values.get<int>(1) == 0;
                          })) {
         throw file_load_failure{"Failed to load layer index.", path.string(),
                                 "Layer index did not contain a valid entry for the [Base] layer!"s};
      }

      layer_remap layer_remap;
      layer_remap.reserve(8);

      world_out.layer_descriptions.reserve(8);
      world_out.layer_descriptions.push_back({.name = "[Base]"s});

      for (const auto& key_node : layer_index) {
         if (key_node.key == "Layer"sv) {
            if (key_node.values.get<std::string_view>(0) == "[Base]"sv) {
               continue;
            }

            layer_remap[key_node.values.get<int>(1)] =
               static_cast<int>(world_out.layer_descriptions.size());
            world_out.layer_descriptions.push_back(
               {.name = key_node.values.get<std::string>(0)});

            output.write(fmt::format("Found world layer '{}' in .ldx file\n",
                                     world_out.layer_descriptions.back().name));
         }
         else if (key_node.key == "GameMode"sv) {
            auto& gamemode = world_out.gamemode_descriptions.emplace_back();

            gamemode.name = key_node.values.get<std::string>(0);

            for (const auto& gamemode_key_node : key_node) {
               if (gamemode_key_node.key != "Layer"sv) continue;

               gamemode.layers.push_back(gamemode_key_node.values.get<int>(0));
            }

            output.write(
               fmt::format("Found gamemode '{}' in .ldx file\n", gamemode.name));
         }
      }

      for (auto& gamemode : world_out.gamemode_descriptions) {
         for (auto& layer : gamemode.layers) {
            layer = layer_remap[layer];
         }
      }

      return layer_remap;
   }
   catch (std::exception& e) {
      throw file_load_failure{"Failed to load layer index.", path.string(), e.what()};
   }
}

void load_objects(const std::filesystem::path& path, output_stream& output,
                  world& world_out, layer_remap& layer_remap)
{
   using namespace assets;

   try {
      for (auto& key_node : config::read_config(utility::read_file_to_string(path))) {
         if (key_node.key != "Object"sv) continue;

         auto& object = world_out.objects.emplace_back();

         object.name = key_node.values.get<std::string>(0);
         object.class_name = lowercase_string{key_node.values.get<std::string>(1)};
         std::tie(object.rotation, object.position) =
            read_location(key_node, "ChildRotation"sv, "ChildPosition"sv);

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

         output.write(fmt::format("Loaded world object '{}' with class '{}'\n"sv,
                                  object.name, object.class_name));
      }
   }
   catch (std::exception& e) {
      throw file_load_failure{"Failed to load layer objects.", path.string(),
                              e.what()};
   }
}

void load_lights(const std::filesystem::path& path, output_stream& output,
                 world& world_out, layer_remap& layer_remap)
{
   using namespace assets;

   try {
      for (auto& key_node : config::read_config(utility::read_file_to_string(path))) {
         if (key_node.key == "Light"sv) {
            auto& light = world_out.lights.emplace_back();

            light.name = key_node.values.get<std::string>(0);
            light.layer = read_layer_index(key_node, layer_remap);
            std::tie(light.rotation, light.position) =
               read_location(key_node, "Rotation"sv, "Position"sv);

            switch (const auto type = key_node.at("Type"sv).values.get<int>(0); type) {
            case 1:
            case 2:
            case 3:
               light.light_type = light_type{type};
               break;
            default:
               output.write(fmt::format("Warning! World light '{}' has invalid light type! Defaulting to point light.\n"sv,
                                        light.name));
               light.light_type = light_type::point;
            }

            light.color = {key_node.at("Color"sv).values.get<float>(0),
                           key_node.at("Color"sv).values.get<float>(1),
                           key_node.at("Color"sv).values.get<float>(2)};

            light.static_ = key_node.contains("Static"sv);
            light.shadow_caster = key_node.contains("CastShadow"sv);
            light.specular_caster = key_node.contains("CastSpecular"sv);

            if (auto texture = key_node.find("Texture"sv); texture != key_node.cend()) {
               light.texture = texture->values.get<std::string>(0);

               switch (const auto addressing = texture->values.get<int>(1); addressing) {
               case 0:
               case 1:
                  light.texture_addressing = texture_addressing{addressing};
                  break;
               default:
                  output.write(fmt::format("Warning! World light '{}' has invalid texture addressing mode! Defaulting to clamp.\n"sv,
                                           light.name));
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
               light.directional_region = region->values.get<std::string>(0);
            }

            if (auto range = key_node.find("Range"sv); range != key_node.cend()) {
               light.range = range->values.get<float>(0);
            }

            if (auto cone = key_node.find("Cone"sv); cone != key_node.cend()) {
               light.inner_cone_angle = cone->values.get<float>(0);
               light.outer_cone_angle = cone->values.get<float>(1);
            }

            output.write(fmt::format("Loaded world light '{}'\n"sv, light.name));
         }
         else if (key_node.key == "GlobalLights"sv) {
            world_out.lighting_settings.global_lights[0] =
               key_node.at("Light1"sv).values.get<std::string>(0);
            world_out.lighting_settings.global_lights[1] =
               key_node.at("Light2"sv).values.get<std::string>(0);
            world_out.lighting_settings.ambient_sky_color = utility::decompress_srgb(
               float3{key_node.at("Top"sv).values.get<float>(0) / 255.0f,
                      key_node.at("Top"sv).values.get<float>(1) / 255.0f,
                      key_node.at("Top"sv).values.get<float>(2) / 255.0f});
            world_out.lighting_settings.ambient_ground_color = utility::decompress_srgb(
               float3{key_node.at("Bottom"sv).values.get<float>(0) / 255.0f,
                      key_node.at("Bottom"sv).values.get<float>(1) / 255.0f,
                      key_node.at("Bottom"sv).values.get<float>(2) / 255.0f});

            if (auto env_map = key_node.find("EnvMap"sv); env_map != key_node.cend()) {
               world_out.lighting_settings.env_map_texture =
                  env_map->values.get<std::string>(0);
            }
         }
      }
   }
   catch (std::exception& e) {
      throw file_load_failure{"Failed to load layer objects.", path.string(),
                              e.what()};
   }
}

void load_paths(const std::filesystem::path& filepath, output_stream& output,
                world& world_out, layer_remap& layer_remap)
{
   using namespace assets;

   try {
      for (auto& key_node :
           config::read_config(utility::read_file_to_string(filepath))) {
         if (key_node.key != "Path"sv) continue;

         auto& path = world_out.paths.emplace_back();

         path.name = key_node.values.get<std::string>(0);
         path.layer = read_layer_index(key_node, layer_remap);
         path.properties = read_path_properties(key_node.at("Properties"sv));

         // path nodes
         {
            auto& path_nodes = key_node.at("Nodes"sv);
            path.nodes.reserve(path_nodes.values.get<std::size_t>(0));

            for (auto& node : path_nodes) {
               auto& path_node = path.nodes.emplace_back();

               std::tie(path_node.rotation, path_node.position) =
                  read_location(node, "Rotation"sv, "Position"sv);

               path_node.properties = read_path_properties(node.at("Properties"sv));
            }
         }

         output.write(fmt::format("Loaded world path '{}'\n"sv, path.name));
      }
   }
   catch (std::exception& e) {
      throw file_load_failure{"Failed to load layer paths.", filepath.string(),
                              e.what()};
   }
}

void load_regions(const std::filesystem::path& filepath, output_stream& output,
                  world& world_out, layer_remap& layer_remap)
{
   using namespace assets;

   try {
      for (auto& key_node :
           config::read_config(utility::read_file_to_string(filepath))) {
         if (key_node.key != "Region"sv) continue;

         auto& region = world_out.regions.emplace_back();

         region.name = key_node.at("Name"sv).values.get<std::string>(0);
         region.layer = read_layer_index(key_node, layer_remap);
         std::tie(region.rotation, region.position) =
            read_location(key_node, "Rotation"sv, "Position"sv);
         region.size = {key_node.at("Size"sv).values.get<float>(0),
                        key_node.at("Size"sv).values.get<float>(1),
                        key_node.at("Size"sv).values.get<float>(2)};
         region.description = key_node.values.get<std::string>(0);

         switch (const int shape = key_node.values.get<int>(1); shape) {
         case 0:
         case 1:
         case 2:
            region.shape = static_cast<region_shape>(shape);
            break;
         default:
            output.write(fmt::format("Warning! World region '{}' has invalid shape! Defaulting to box.\n"sv,
                                     region.name));
            region.shape = region_shape::box;
         }

         output.write(fmt::format("Loaded world region '{}'\n"sv, region.name));
      }
   }
   catch (std::exception& e) {
      throw file_load_failure{"Failed to load layer regions.",
                              filepath.string(), e.what()};
   }
}

void load_portals_sectors(const std::filesystem::path& filepath, output_stream& output,
                          world& world_out, layer_remap& layer_remap)
{
   using namespace assets;

   try {
      for (auto& key_node :
           config::read_config(utility::read_file_to_string(filepath))) {
         if (key_node.key == "Sector"sv) {
            auto& sector = world_out.sectors.emplace_back();

            sector.name = key_node.values.get<std::string>(0);
            sector.layer = read_layer_index(key_node, layer_remap);

            for (auto& sector_prop : key_node) {
               if (sector_prop.key == "Base"sv) {
                  sector.base = sector_prop.values.get<float>(0);
               }
               else if (sector_prop.key == "Height"sv) {
                  sector.height = sector_prop.values.get<float>(0);
               }
               else if (sector_prop.key == "Point"sv) {
                  sector.points.push_back({sector_prop.values.get<float>(0),
                                           sector_prop.values.get<float>(1)});
               }
               else if (sector_prop.key == "Object"sv) {
                  sector.objects.emplace_back(sector_prop.values.get<std::string>(0));
               }
            }

            output.write(fmt::format("Loaded world sector '{}'\n"sv, sector.name));
         }
         else if (key_node.key == "Portal"sv) {
            auto& portal = world_out.portals.emplace_back();

            portal.name = key_node.values.get<std::string>(0);
            portal.layer = read_layer_index(key_node, layer_remap);
            std::tie(portal.rotation, portal.position) =
               read_location(key_node, "Rotation"sv, "Position"sv);
            portal.width = key_node.at("Width"sv).values.get<float>(0);
            portal.height = key_node.at("Height"sv).values.get<float>(0);

            if (auto sector = key_node.find("Sector1"sv); sector != key_node.cend()) {
               portal.sector1 = sector->values.get<std::string>(0);
            }

            if (auto sector = key_node.find("Sector2"sv); sector != key_node.cend()) {
               portal.sector1 = sector->values.get<std::string>(0);
            }

            output.write(fmt::format("Loaded world portal '{}'\n"sv, portal.name));
         }
      }
   }
   catch (std::exception& e) {
      throw file_load_failure{"Failed to load layer portals and sectors.",
                              filepath.string(), e.what()};
   }
}

void load_barriers(const std::filesystem::path& filepath, output_stream& output,
                   world& world_out, layer_remap& layer_remap)
{
   using namespace assets;

   try {
      for (auto& key_node :
           config::read_config(utility::read_file_to_string(filepath))) {
         if (key_node.key != "Barrier"sv) continue;

         auto& barrier = world_out.barriers.emplace_back();

         barrier.name = key_node.values.get<std::string>(0);
         barrier.layer = read_layer_index(key_node, layer_remap);
         barrier.flags = ai_path_flags{key_node.at("Flag"sv).values.get<int>(0)};

         const auto is_corner = [](const config::key_node& child_key_node) {
            return child_key_node.key == "Corner"sv;
         };

         auto corner = std::find_if(key_node.cbegin(), key_node.cend(), is_corner);

         for (int i = 0; i < 4; ++i) {
            if (corner != key_node.cend()) {
               barrier.corners[i] = {corner->values.get<float>(0),
                                     -corner->values.get<float>(2)};

               corner = std::find_if(corner + 1, key_node.cend(), is_corner);
            }
            else {
               output.write(fmt::format("Warning! World barrier '{}' is missing one or more corners!\n"sv,
                                        barrier.name));
               break;
            }
         }

         output.write(fmt::format("Loaded world barrier '{}'\n"sv, barrier.name));
      }
   }
   catch (std::exception& e) {
      throw file_load_failure{"Failed to load layer barriers.",
                              filepath.string(), e.what()};
   }
}

void load_boundaries(const std::filesystem::path& filepath,
                     output_stream& output, world& world_out)
{
   using namespace assets;

   try {
      for (auto& key_node :
           config::read_config(utility::read_file_to_string(filepath))) {
         if (key_node.key != "Boundary"sv) continue;

         for (auto& child_key_node : key_node) {
            if (child_key_node.key != "Path"sv) continue;

            auto& boundary = world_out.boundaries.emplace_back();

            boundary.name = child_key_node.values.get<std::string>(0);

            output.write(fmt::format("Loaded world boundary '{}'\n"sv, boundary.name));
         }
      }
   }
   catch (std::exception& e) {
      throw file_load_failure{"Failed to load layer boundaries.",
                              filepath.string(), e.what()};
   }
}

void load_layer(const std::filesystem::path& world_dir,
                const std::string_view layer_name, const std::string_view world_ext,
                output_stream& output, world& world_out, layer_remap& layer_remap)
{
   load_objects(world_dir / layer_name += world_ext, output, world_out, layer_remap);

   if (const auto paths_path = world_dir / layer_name += ".pth"sv;
       std::filesystem::exists(paths_path)) {
      load_paths(paths_path, output, world_out, layer_remap);
   }

   if (const auto regions_path = world_dir / layer_name += ".rgn"sv;
       std::filesystem::exists(regions_path)) {
      load_regions(regions_path, output, world_out, layer_remap);
   }

   if (const auto lights_path = world_dir / layer_name += ".lgt"sv;
       std::filesystem::exists(lights_path)) {
      load_lights(lights_path, output, world_out, layer_remap);
   }

   if (const auto pvs_path = world_dir / layer_name += ".pvs"sv;
       std::filesystem::exists(pvs_path)) {
      load_portals_sectors(pvs_path, output, world_out, layer_remap);
   }

   if (const auto bar_path = world_dir / layer_name += ".bar"sv;
       std::filesystem::exists(bar_path)) {
      load_barriers(bar_path, output, world_out, layer_remap);
   }

   if (const auto bnd_path = world_dir / layer_name += ".bnd"sv;
       std::filesystem::exists(bnd_path)) {
      load_boundaries(bnd_path, output, world_out);
   }
}

}

auto load_world(const std::filesystem::path& path, output_stream& output) -> world
{
   world world;

   world.name = path.stem().string();

   const auto world_dir = path.parent_path();

   try {
      auto layer_remap =
         load_layer_index(world_dir / world.name += ".ldx"sv, output, world);

      load_layer(world_dir, world.name, ".wld"sv, output, world, layer_remap);

      for (const auto& layer :
           utility::make_range(world.layer_descriptions.begin() + 1,
                               world.layer_descriptions.end())) {
         load_layer(world_dir, fmt::format("{}_{}"sv, world.name, layer.name),
                    ".lyr"sv, output, world, layer_remap);
      }

      world.terrain = read_terrain(
         utility::read_file_to_bytes(world_dir / world.name += ".ter"sv));
   }
   catch (file_load_failure& load_failure) {
      output.write(
         fmt::format("Error while loading world:\n   File: {}\n   Message: \n{}\n"sv,
                     load_failure.filepath,
                     utility::string::indent(2, load_failure.description)));

      throw;
   }

   return world;
}
}