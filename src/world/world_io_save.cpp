
#include "world_io_save.hpp"

#include <cctype>
#include <numeric>

#include "assets/terrain/terrain_io.hpp"
#include "io/output_file.hpp"
#include "utility/srgb_conversion.hpp"

using namespace std::literals;

namespace we::world {

namespace {

auto flip_rotation(quaternion rotation) -> quaternion
{
   std::swap(rotation.x, rotation.z);
   std::swap(rotation.y, rotation.w);

   rotation.x = -rotation.x;
   rotation.z = -rotation.z;

   return rotation;
}

auto flip_position(float3 position) -> float3
{
   return {position.x, position.y, -position.z};
}

bool is_numeric_path_property(const std::string_view str) noexcept
{
   for (auto& c : str) {
      if (not std::isdigit(static_cast<unsigned char>(c)) and c != '.') {
         return false;
      }
   }

   return true;
}

void save_objects(const std::filesystem::path& path, const std::string_view layer_name,
                  const int layer_index, const world& world)
{
   io::output_file file{path};

   file.write_ln("Version(3);");
   file.write_ln("SaveType(0);\n");

   file.write_ln("LightName(\"{}.LGT\");", layer_name);

   if (layer_index == 0) {
      file.write_ln("TerrainName(\"{}.ter\");", layer_name);
      file.write_ln("SkyName(\"{}.sky\");\n", layer_name);
   }

   file.write_ln("NextSequence({});\n", world.objects.size());

   for (std::size_t i = 0; i < world.objects.size(); ++i) {
      const auto& object = world.objects[i];
      const auto rotation = flip_rotation(object.rotation);
      const auto position = flip_position(object.position);

      if (object.layer != layer_index) continue;

      file.write_ln("Object(\"{}\", \"{}\", {})", object.name, object.class_name, i);
      file.write_ln("{");

      file.write_ln("\tChildRotation({:f}, {:f}, {:f}, {:f});", rotation.w,
                    rotation.x, rotation.y, rotation.z);
      file.write_ln("\tChildPosition({:f}, {:f}, {:f});", position.x,
                    position.y, position.z);
      file.write_ln("\tTeam({});", object.team);
      file.write_ln("\tNetworkId(-1);");

      if (layer_index != 0) file.write_ln("\tLayer({});", object.layer);

      for (auto& prop : object.instance_properties) {
         file.write_ln("\t{}(\"{}\");", prop.key, prop.value);
      }

      file.write_ln("}\n");
   }
}

void save_paths(const std::filesystem::path& file_path, const int layer_index,
                const world& world)
{
   io::output_file file{file_path};

   const int layer_path_count =
      std::accumulate(world.paths.begin(), world.paths.end(), 0,
                      [=](int total, const path& path) {
                         return layer_index == path.layer ? total + 1 : total;
                      });

   file.write_ln("Version(10);");
   file.write_ln("PathCount({});\n", layer_path_count);

   for (auto& path : world.paths) {
      if (path.layer != layer_index) continue;

      file.write_ln("Path(\"{}\")", path.name);
      file.write_ln("{");

      file.write_ln("\tData(1);");
      file.write_ln("\tPathType(0);");
      file.write_ln("\tPathSpeedType(0);");
      file.write_ln("\tPathTime(0.000000);");
      file.write_ln("\tOffsetPath(0);");

      if (layer_index != 0) file.write_ln("\tLayer({});", path.layer);

      if (path.spline_type == path_spline_type::none) {
         file.write_ln("\tSplineType(\"None\");\n");
      }
      else if (path.spline_type == path_spline_type::linear) {
         file.write_ln("\tSplineType(\"Linear\");\n");
      }
      else if (path.spline_type == path_spline_type::hermite) {
         file.write_ln("\tSplineType(\"Hermite\");\n");
      }
      else if (path.spline_type == path_spline_type::catmull_rom) {
         file.write_ln("\tSplineType(\"Catmull-Rom\");\n");
      }

      file.write_ln("\tProperties({})", path.properties.size());
      file.write_ln("\t{");

      for (auto& prop : path.properties) {
         if (is_numeric_path_property(prop.value)) {
            file.write_ln("\t\t{}({});", prop.key, prop.value);
         }
         else {
            file.write_ln("\t\t{}(\"{}\");", prop.key, prop.value);
         }
      }

      file.write_ln("\t}\n");

      file.write_ln("\tNodes({})", path.nodes.size());
      file.write_ln("\t{");

      for (auto& node : path.nodes) {
         const auto rotation = flip_rotation(node.rotation);
         const auto position = flip_position(node.position);

         file.write_ln("\t\tNode()");
         file.write_ln("\t\t{");

         file.write_ln("\t\t\tPosition({:f}, {:f}, {:f});", position.x,
                       position.y, position.z);
         file.write_ln("\t\t\tKnot(0.000000);");
         file.write_ln("\t\t\tData(1);");
         file.write_ln("\t\t\tTime(1.000000);");
         file.write_ln("\t\t\tPauseTime(0.000000);");
         file.write_ln("\t\t\tRotation({:f}, {:f}, {:f}, {:f});", rotation.w,
                       rotation.x, rotation.y, rotation.z);

         file.write_ln("\t\t\tProperties({})", node.properties.size());
         file.write_ln("\t\t\t{");

         for (auto& prop : node.properties) {
            if (is_numeric_path_property(prop.value)) {
               file.write_ln("\t\t\t\t{}({});", prop.key, prop.value);
            }
            else {
               file.write_ln("\t\t\t\t{}(\"{}\");", prop.key, prop.value);
            }
         }

         file.write_ln("\t\t\t}");

         file.write_ln("\t\t}\n");
      }

      file.write_ln("\t}\n");

      file.write_ln("}\n");
   }
}

void save_regions(const std::filesystem::path& path, const int layer_index,
                  const world& world)
{
   io::output_file file{path};

   const int layer_region_count =
      std::accumulate(world.regions.begin(), world.regions.end(), 0,
                      [=](int total, const region& region) {
                         return layer_index == region.layer ? total + 1 : total;
                      });

   file.write_ln("Version(1);");
   file.write_ln("RegionCount({});\n", layer_region_count);

   for (auto& region : world.regions) {
      if (region.layer != layer_index) continue;

      const auto rotation = flip_rotation(region.rotation);
      const auto position = flip_position(region.position);

      file.write_ln("Region(\"{}\", {})", region.description,
                    static_cast<int>(region.shape));
      file.write_ln("{");

      if (layer_index != 0) file.write_ln("\tLayer({});", region.layer);

      file.write_ln("\tPosition({:f}, {:f}, {:f});", position.x, position.y,
                    position.z);
      file.write_ln("\tRotation({:f}, {:f}, {:f}, {:f});", rotation.w,
                    rotation.x, rotation.y, rotation.z);
      file.write_ln("\tSize({:f}, {:f}, {:f});", region.size.x, region.size.y,
                    region.size.z);
      file.write_ln("\tName(\"{}\");", region.name);

      // TODO: Region groups - NextIsGrouped(); support

      file.write_ln("}\n");
   }
}

void save_lights(const std::filesystem::path& path, const int layer_index,
                 const world& world)
{
   io::output_file file{path};

   for (std::size_t i = 0; i < world.lights.size(); ++i) {
      auto& light = world.lights[i];

      if (light.layer != layer_index) continue;

      const auto rotation = flip_rotation(light.rotation);
      const auto position = flip_position(light.position);

      file.write_ln("Light(\"{}\", {})", light.name, i);
      file.write_ln("{");
      file.write_ln("\tRotation({:f}, {:f}, {:f}, {:f});", rotation.w,
                    rotation.x, rotation.y, rotation.z);
      file.write_ln("\tPosition({:f}, {:f}, {:f});", position.x, position.y,
                    position.z);
      file.write_ln("\tType({});", static_cast<int>(light.light_type));
      file.write_ln("\tColor({:f}, {:f}, {:f});", light.color.x, light.color.y,
                    light.color.z);

      if (light.shadow_caster) file.write_ln("\tCastShadow();");
      if (light.static_) file.write_ln("\tStatic();");
      if (not light.texture.empty()) {
         file.write_ln("\tTexture(\"{}\")", light.texture);
      }
      if (light.specular_caster) file.write_ln("\tCastSpecular(1);");

      if (light.light_type == light_type::directional) {
         if (not light.directional_region.empty()) {
            file.write_ln("\tRegion(\"{}\");", light.directional_region);
         }

         file.write_ln("\tPS2BlendMode(0);");
         file.write_ln("\tTileUV({:f}, {:f});", light.directional_texture_tiling.x,
                       light.directional_texture_tiling.y);
         file.write_ln("\tOffsetUV({:f}, {:f});",
                       light.directional_texture_offset.x,
                       light.directional_texture_offset.y);
      }
      else if (light.light_type == light_type::point) {
         file.write_ln("\tRange({:f});", light.range);
      }
      else if (light.light_type == light_type::spot) {
         file.write_ln("\tRange({:f});", light.range);
         file.write_ln("\tCone({:f}, {:f});", light.inner_cone_angle,
                       light.outer_cone_angle);
         file.write_ln("\tPS2BlendMode(0);");
         file.write_ln("\tBidirectional(0);");
      }

      file.write_ln("}\n");
   }

   if (layer_index == 0) {
      auto ambient_sky_color =
         utility::compress_srgb(world.lighting_settings.ambient_sky_color);
      auto ambient_ground_color =
         utility::compress_srgb(world.lighting_settings.ambient_ground_color);

      file.write_ln("GlobalLights()");
      file.write_ln("{");
      file.write_ln("\tEditorGlobalDirIconSize(10);");
      file.write_ln("\tLight1(\"{}\");", world.lighting_settings.global_lights[0]);
      file.write_ln("\tLight2(\"{}\");", world.lighting_settings.global_lights[1]);
      file.write_ln("\tTop({}, {}, {});",
                    static_cast<int>(ambient_sky_color.x * 255.0f + 0.5f),
                    static_cast<int>(ambient_sky_color.y * 255.0f + 0.5f),
                    static_cast<int>(ambient_sky_color.z * 255.0f + 0.5f));
      file.write_ln("\tBottom({}, {}, {});",
                    static_cast<int>(ambient_ground_color.x * 255.0f + 0.5f),
                    static_cast<int>(ambient_ground_color.y * 255.0f + 0.5f),
                    static_cast<int>(ambient_ground_color.z * 255.0f + 0.5f));

      if (not world.lighting_settings.env_map_texture.empty()) {
         file.write_ln("\tEnvMap(\"{}\");", world.lighting_settings.env_map_texture);
      }

      file.write_ln("}");
   }
}

void save_hintnodes(const std::filesystem::path& path, const int layer_index,
                    const world& world)
{
   io::output_file file{path};

   for (auto& hint : world.hintnodes) {
      if (hint.layer != layer_index) continue;

      const auto rotation = flip_rotation(hint.rotation);
      const auto position = flip_position(hint.position);

      // the quotes around the integer are not a mistake, this is how the type is saved by ZE.
      file.write_ln("Hint(\"{}\", \"{}\")", hint.name, static_cast<int>(hint.type));
      file.write_ln("{");

      file.write_ln("\tPosition({:f}, {:f}, {:f});", position.x, position.y,
                    position.z);
      file.write_ln("\tRotation({:f}, {:f}, {:f}, {:f});", rotation.w,
                    rotation.x, rotation.y, rotation.z);

      if (hint.radius > 0.0f) file.write_ln("\tRadius({:f});", hint.radius);

      if (hint.primary_stance != stance_flags::none) {
         file.write_ln("\tPrimaryStance({});", static_cast<int>(hint.primary_stance));
      }
      if (hint.secondary_stance != stance_flags::none) {
         file.write_ln("\tSecondaryStance({});",
                       static_cast<int>(hint.secondary_stance));
      }

      file.write_ln("\tMode({});", static_cast<int>(hint.mode));

      if (not hint.command_post.empty()) {
         file.write_ln("\tCommandPost(\"{}\");", hint.command_post);
      }

      file.write_ln("}");
   }
}

void save_portals_sectors(const std::filesystem::path& path, const world& world)
{
   io::output_file file{path};

   for (auto& sector : world.sectors) {
      file.write_ln("Sector(\"{}\")", sector.name);
      file.write_ln("{");

      file.write_ln("\tBase({:f});", sector.base);
      file.write_ln("\tHeight({:f});", sector.height);

      for (auto& point : sector.points) {
         file.write_ln("\tPoint({:f}, {:f});", point.x, -point.y);
      }

      for (auto& object : sector.objects) {
         file.write_ln("\tObject(\"{}\");", object);
      }

      file.write_ln("}");
   }

   for (auto& portal : world.portals) {
      const auto rotation = flip_rotation(portal.rotation);
      const auto position = flip_position(portal.position);

      file.write_ln("Portal(\"{}\")", portal.name);
      file.write_ln("{");

      file.write_ln("\tRotation({:f}, {:f}, {:f}, {:f});", rotation.w,
                    rotation.x, rotation.y, rotation.z);
      file.write_ln("\tPosition({:f}, {:f}, {:f});", position.x, position.y,
                    position.z);

      file.write_ln("\tWidth({:f});", portal.width);
      file.write_ln("\tHeight({:f});", portal.height);

      file.write_ln("\tSector1(\"{}\");", portal.sector1);
      file.write_ln("\tSector2(\"{}\");", portal.sector2);

      file.write_ln("}");
   }
}

void save_barriers(const std::filesystem::path& path, const world& world)
{
   io::output_file file{path};

   file.write_ln("BarrierCount({});\n", world.barriers.size());

   for (auto& barrier : world.barriers) {
      file.write_ln("Barrier(\"{}\")", barrier.name);
      file.write_ln("{");

      for (auto& corner : barrier.corners) {
         file.write_ln("\tCorner({:f}, 0.000000, {:f});", corner.x, -corner.y);
      }

      file.write_ln("\tFlag({});", static_cast<int>(barrier.flags));

      file.write_ln("}\n");
   }
}

void save_boundaries(const std::filesystem::path& path, const world& world)
{
   io::output_file file{path};

   for (auto& boundary : world.boundaries) {
      file.write_ln("Boundary()", boundary.name);
      file.write_ln("{");
      file.write_ln("\tPath(\"{}\");", boundary.name);
      file.write_ln("}\n");
   }
}

/// @brief Saves a world layer.
/// @param world_dir The directory to save the layer into.
/// @param layer_name The name of the layer ie `test` or `test_conquest`.
/// @param layer_index The index of the layer, 0 is special and indicates the base layer.
/// @param world The world that is being saved.
void save_layer(const std::filesystem::path& world_dir, const std::string_view layer_name,
                const int layer_index, const world& world)
{
   save_objects(world_dir / layer_name += (layer_index == 0 ? L".wld"sv : L".lyr"sv),
                layer_name, layer_index, world);

   save_paths(world_dir / layer_name += L".pth"sv, layer_index, world);
   save_regions(world_dir / layer_name += L".rgn"sv, layer_index, world);
   save_lights(world_dir / layer_name += L".lgt"sv, layer_index, world);
   save_hintnodes(world_dir / layer_name += L".hnt"sv, layer_index, world);

   if (layer_index == 0) {
      save_boundaries(world_dir / layer_name += L".bnd"sv, world);
      save_barriers(world_dir / layer_name += L".bar"sv, world);
      save_portals_sectors(world_dir / layer_name += L".pvs"sv, world);
   }
}

void save_layer_index(const std::filesystem::path& path, const world& world)
{
   io::output_file file{path};

   file.write_ln("Version(1);");
   file.write_ln("NextID({});\n", world.layer_descriptions.size());

   for (std::size_t i = 0; i < world.layer_descriptions.size(); ++i) {
      auto& layer = world.layer_descriptions[i];

      file.write_ln("Layer(\"{}\", {}, {})", layer.name, i,
                    static_cast<int>(layer.flags));
      file.write_ln("{");
      file.write_ln("\tDescription(\"\");");
      file.write_ln("}\n");
   }

   if (not world.gamemode_descriptions.empty()) { // for possible BF1 support
      for (auto& gamemode : world.gamemode_descriptions) {
         file.write_ln("GameMode(\"{}\")", gamemode.name);
         file.write_ln("{");
         for (auto& layer : gamemode.layers) {
            file.write_ln("\tLayer({});", layer);
         }
         file.write_ln("}\n");
      }
   }
}

}

void save_world(const std::filesystem::path& path, const world& world)
{
   const auto world_dir = path.parent_path();
   const auto world_name = path.stem().string();

   save_layer_index(std::filesystem::path{path}.replace_extension(L".ldx"sv), world);

   save_layer(world_dir, world_name, 0, world);

   for (std::size_t i = 1; i < world.layer_descriptions.size(); ++i) {
      auto& layer = world.layer_descriptions[i];

      save_layer(world_dir, world_name + "_"s + layer.name,
                 static_cast<uint32>(i), world);
   }

   save_terrain(std::filesystem::path{path}.replace_extension(L".ter"sv), world.terrain);
}

}