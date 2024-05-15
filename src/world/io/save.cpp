
#include "save.hpp"
#include "assets/req/io.hpp"
#include "math/vector_funcs.hpp"
#include "save_effects.hpp"
#include "utility/string_icompare.hpp"
#include "world/utility/boundary_nodes.hpp"

#include <cctype>
#include <cstddef>
#include <numeric>

#include <absl/container/flat_hash_map.h>
#include <absl/container/inlined_vector.h>

#include "assets/terrain/terrain_io.hpp"
#include "io/output_file.hpp"

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
   if (str.empty()) return false;

   for (auto& c : str) {
      if (not std::isdigit(static_cast<unsigned char>(c)) and c != '.') {
         return false;
      }
   }

   return true;
}

bool is_regional_light(const light& light) noexcept
{
   switch (light.light_type) {
   case light_type::directional_region_box:
   case light_type::directional_region_sphere:
   case light_type::directional_region_cylinder:
      return not light.region_name.empty();
   default:
      return false;
   }
}

auto flatten_light_type(const light& light) noexcept -> light_type
{
   switch (light.light_type) {
   case light_type::directional_region_box:
   case light_type::directional_region_sphere:
   case light_type::directional_region_cylinder:
      return light_type::directional;
   default:
      return light.light_type;
   }
}

auto light_region_shape(const light& light) noexcept -> region_shape
{
   switch (light.light_type) {
   case light_type::directional_region_box:
      return region_shape::box;
   case light_type::directional_region_sphere:
      return region_shape::sphere;
   case light_type::directional_region_cylinder:
      return region_shape::cylinder;
   default:
      return region_shape::box;
   }
}

auto make_barrier_corners(const barrier& barrier) noexcept -> std::array<float3, 4>
{
   const double rot_sin = std::sin(double{barrier.rotation_angle});
   const double rot_cos = -std::cos(double{barrier.rotation_angle});

   constexpr std::array<float2, 4> base_corners{float2{-1.0f, 1.0f},
                                                float2{-1.0f, -1.0f},
                                                float2{1.0f, -1.0f},
                                                float2{1.0f, 1.0f}};

   std::array<float3, 4> cornersWS{};

   for (std::size_t i = 0; i < cornersWS.size(); ++i) {
      const float2 cornerOS = base_corners[i] * barrier.size;

      cornersWS[i] =
         float3{static_cast<float>(cornerOS.x * rot_cos - cornerOS.y * rot_sin), 0.0f,
                static_cast<float>(cornerOS.x * rot_sin + cornerOS.y * rot_cos)} +
         barrier.position;
   }

   return cornersWS;
}

struct hub_branch_weight_ref {
   std::string_view end_hub;
   float weight = 0.0f;
   std::string_view connection;
   ai_path_flags flag;
};

struct sequence_numbers {
   int objects = 0;
   int lights = 0;
};

auto get_hub_branch_weight_refs(const world& world)
   -> absl::flat_hash_map<uint32, absl::InlinedVector<hub_branch_weight_ref, 12>>
{
   absl::flat_hash_map<uint32, absl::InlinedVector<hub_branch_weight_ref, 12>> refs;
   refs.reserve(64);

   for (auto& connection : world.planning_connections) {
      const auto process_direction = [&](const planning_branch_weights& weights,
                                         const uint32 start_hub_index,
                                         const uint32 end_hub_index) {
         const auto process_weight = [&](const float weight, const ai_path_flags flag) {
            if (weight != 0.0f) {
               refs[start_hub_index].push_back(
                  {.end_hub = world.planning_hubs[end_hub_index].name,
                   .weight = weight,
                   .connection = connection.name,
                   .flag = flag});
            }
         };

         process_weight(weights.flyer, ai_path_flags::flyer);
         process_weight(weights.huge, ai_path_flags::huge);
         process_weight(weights.medium, ai_path_flags::medium);
         process_weight(weights.small, ai_path_flags::small);
         process_weight(weights.hover, ai_path_flags::hover);
         process_weight(weights.soldier, ai_path_flags::soldier);
      };

      process_direction(connection.forward_weights, connection.start_hub_index,
                        connection.end_hub_index);
      process_direction(connection.backward_weights, connection.end_hub_index,
                        connection.start_hub_index);
   }

   return refs;
}

void save_objects(const std::filesystem::path& path,
                  const std::string_view layer_name, const int layer_index,
                  const world& world, sequence_numbers& sequence_numbers)
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

   for (const auto& object : world.objects) {
      const auto rotation = flip_rotation(object.rotation);
      const auto position = flip_position(object.position);
      const int sequence_number = sequence_numbers.objects++;

      if (object.layer != layer_index) continue;

      file.write_ln("Object(\"{}\", \"{}\", {})", object.name,
                    object.class_name, sequence_number);
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
                      }) +
      (layer_index == 0 ? static_cast<int>(world.boundaries.size()) : 0);

   file.write_ln("Version(10);");
   file.write_ln("PathCount({});\n", layer_path_count);

   for (auto& path : world.paths) {
      if (path.layer != layer_index) continue;

      const std::string name_prefix = [&] {
         switch (path.type) {
         default:
         case path_type::none:
            return ""s;
         case path_type::entity_follow:
            return "type_EntityPath "s;
         case path_type::formation:
            return "type_EntityFormation "s;
         case path_type::patrol:
            return "type_PatrolPath "s;
         }
      }();

      file.write_ln("Path(\"{}{}\")", name_prefix, path.name);
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

   if (layer_index != 0) return;

   for (auto& boundary : world.boundaries) {
      file.write_ln("Path(\"{}\")", boundary.name);
      file.write_ln("{");

      file.write_ln("\tData(0);");
      file.write_ln("\tPathType(0);");
      file.write_ln("\tPathSpeedType(0);");
      file.write_ln("\tPathTime(0.000000);");
      file.write_ln("\tOffsetPath(0);");
      file.write_ln("\tSplineType(\"Hermite\");\n");

      file.write_ln("\tProperties(0)");
      file.write_ln("\t{");
      file.write_ln("\t}\n");

      const std::array<float3, 12> nodes = get_boundary_nodes(boundary);

      file.write_ln("\tNodes({})", nodes.size());
      file.write_ln("\t{");

      for (auto& node : nodes) {
         file.write_ln("\t\tNode()");
         file.write_ln("\t\t{");

         file.write_ln("\t\t\tPosition({:f}, {:f}, {:f});", node.x, node.y,
                       -node.z);
         file.write_ln("\t\t\tKnot(0.000000);");
         file.write_ln("\t\t\tData(0);");
         file.write_ln("\t\t\tTime(1.000000);");
         file.write_ln("\t\t\tPauseTime(0.000000);");
         file.write_ln(
            "\t\t\tRotation(1.000000, 0.000000, 0.000000, 0.000000);");

         file.write_ln("\t\t\tProperties(0)");
         file.write_ln("\t\t\t{");
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
                      }) +
      std::accumulate(world.lights.begin(), world.lights.end(), 0,
                      [=](int total, const light& light) {
                         return is_regional_light(light) and
                                      layer_index == light.layer
                                   ? total + 1
                                   : total;
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

   for (auto& light : world.lights) {
      if (not is_regional_light(light)) continue;

      if (light.layer != layer_index) continue;

      const auto rotation = flip_rotation(light.region_rotation);
      const auto position = flip_position(light.position);

      file.write_ln("Region(\"{}\", {})", light.region_name,
                    static_cast<int>(light_region_shape(light)));
      file.write_ln("{");

      if (layer_index != 0) file.write_ln("\tLayer({});", light.layer);

      file.write_ln("\tPosition({:f}, {:f}, {:f});", position.x, position.y,
                    position.z);
      file.write_ln("\tRotation({:f}, {:f}, {:f}, {:f});", rotation.w,
                    rotation.x, rotation.y, rotation.z);
      file.write_ln("\tSize({:f}, {:f}, {:f});", light.region_size.x,
                    light.region_size.y, light.region_size.z);
      file.write_ln("\tName(\"{}\");", light.region_name);

      file.write_ln("}\n");
   }
}

void save_lights(const std::filesystem::path& path, const int layer_index,
                 const world& world, sequence_numbers& sequence_numbers)
{
   io::output_file file{path};

   for (const auto& light : world.lights) {
      if (light.layer != layer_index) continue;

      const auto rotation = flip_rotation(light.rotation);
      const auto position = flip_position(light.position);
      const light_type light_type = flatten_light_type(light);

      file.write_ln("Light(\"{}\", {})", light.name, sequence_numbers.lights++);
      file.write_ln("{");
      file.write_ln("\tRotation({:f}, {:f}, {:f}, {:f});", rotation.w,
                    rotation.x, rotation.y, rotation.z);
      file.write_ln("\tPosition({:f}, {:f}, {:f});", position.x, position.y,
                    position.z);
      file.write_ln("\tType({});", static_cast<int>(light_type));
      file.write_ln("\tColor({:f}, {:f}, {:f});", light.color.x, light.color.y,
                    light.color.z);

      if (light.shadow_caster) file.write_ln("\tCastShadow();");
      if (light.static_) file.write_ln("\tStatic();");
      if (not light.texture.empty()) {
         file.write_ln("\tTexture(\"{}\", {});", light.texture,
                       static_cast<int>(light.texture_addressing));
      }
      if (light.specular_caster) file.write_ln("\tCastSpecular(1);");

      if (light_type == light_type::directional) {
         if (not light.region_name.empty()) {
            file.write_ln("\tRegion(\"{}\");", light.region_name);
         }

         file.write_ln("\tPS2BlendMode(0);");
         file.write_ln("\tTileUV({:f}, {:f});", light.directional_texture_tiling.x,
                       light.directional_texture_tiling.y);
         file.write_ln("\tOffsetUV({:f}, {:f});",
                       light.directional_texture_offset.x,
                       light.directional_texture_offset.y);
      }
      else if (light_type == light_type::point) {
         file.write_ln("\tRange({:f});", light.range);
      }
      else if (light_type == light_type::spot) {
         file.write_ln("\tRange({:f});", light.range);
         file.write_ln("\tCone({:f}, {:f});", light.inner_cone_angle,
                       light.outer_cone_angle);
         file.write_ln("\tPS2BlendMode(0);");
         file.write_ln("\tBidirectional({});", light.bidirectional ? 1 : 0);
      }

      file.write_ln("}\n");
   }

   if (layer_index == 0) {
      const float3 ambient_sky_color = world.global_lights.ambient_sky_color;
      const float3 ambient_ground_color = world.global_lights.ambient_ground_color;

      file.write_ln("GlobalLights()");
      file.write_ln("{");
      file.write_ln("\tEditorGlobalDirIconSize(10);");
      file.write_ln("\tLight1(\"{}\");", world.global_lights.global_light_1);
      file.write_ln("\tLight2(\"{}\");", world.global_lights.global_light_2);
      file.write_ln("\tTop({}, {}, {});",
                    static_cast<int>(ambient_sky_color.x * 255.0f + 0.5f),
                    static_cast<int>(ambient_sky_color.y * 255.0f + 0.5f),
                    static_cast<int>(ambient_sky_color.z * 255.0f + 0.5f));
      file.write_ln("\tBottom({}, {}, {});",
                    static_cast<int>(ambient_ground_color.x * 255.0f + 0.5f),
                    static_cast<int>(ambient_ground_color.y * 255.0f + 0.5f),
                    static_cast<int>(ambient_ground_color.z * 255.0f + 0.5f));

      if (not world.global_lights.env_map_texture.empty()) {
         file.write_ln("\tEnvMap(\"{}\");", world.global_lights.env_map_texture);
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

      for (auto& corner : make_barrier_corners(barrier)) {
         file.write_ln("\tCorner({:f}, {:f}, {:f});", corner.x, corner.y,
                       -corner.z);
      }

      file.write_ln("\tFlag({});", static_cast<int>(barrier.flags));

      file.write_ln("}\n");
   }
}

void save_planning(const std::filesystem::path& path, const world& world)
{
   const absl::flat_hash_map<uint32, absl::InlinedVector<hub_branch_weight_ref, 12>> hub_branch_weights =
      get_hub_branch_weight_refs(world);

   io::output_file file{path};

   for (uint32 i = 0; i < world.planning_hubs.size(); ++i) {
      const auto& hub = world.planning_hubs[i];

      file.write_ln("");
      file.write_ln("Hub(\"{}\")", hub.name);
      file.write_ln("{");

      file.write_ln("\tPos({:f}, {:f}, {:f});", hub.position.x, hub.position.y,
                    -hub.position.z);

      file.write_ln("\tRadius({:f});", hub.radius);

      if (auto branch_weights_it = hub_branch_weights.find(i);
          branch_weights_it != hub_branch_weights.end()) {
         for (const auto& branch_weight : branch_weights_it->second) {
            file.write_ln("\tBranchWeight(\"{}\",{:f},\"{}\",{});",
                          branch_weight.end_hub, branch_weight.weight,
                          branch_weight.connection,
                          static_cast<int>(branch_weight.flag));
         }
      }

      file.write_ln("}");
   }

   for (auto& connection : world.planning_connections) {
      file.write_ln("");
      file.write_ln("Connection(\"{}\")", connection.name);
      file.write_ln("{");

      file.write_ln("\tStart(\"{}\");",
                    world.planning_hubs[connection.start_hub_index].name);
      file.write_ln("\tEnd(\"{}\");",
                    world.planning_hubs[connection.end_hub_index].name);
      file.write_ln("\tFlag({});", static_cast<int>(connection.flags));

      if (connection.dynamic_group != 0) {
         file.write_ln("\tDynamic({});", static_cast<int>(connection.dynamic_group));
      }

      if (connection.jump) file.write_ln("\tJump();");
      if (connection.jet_jump) file.write_ln("\tJetJump();");
      if (connection.one_way) file.write_ln("\tJOneWay();");

      file.write_ln("}");
   }
}

void save_boundaries(const std::filesystem::path& path, const world& world)
{
   io::output_file file{path};

   file.write_ln("Boundary()");
   file.write_ln("{");

   for (auto& boundary : world.boundaries) {
      file.write_ln("\tPath(\"{}\");", boundary.name);
   }

   file.write_ln("}\n");
}

void save_measurements(const std::filesystem::path& path, const world& world)
{
   io::output_file file{path};

   file.write_ln("MeasurementCount({});\n", world.measurements.size());

   for (auto& measurement : world.measurements) {
      file.write_ln("Measurement(\"{}\")", measurement.name);
      file.write_ln("{");

      file.write_ln("\tStart({:f}, {:f}, {:f});", measurement.start.x,
                    measurement.start.y, -measurement.start.z);
      file.write_ln("\tEnd({:f}, {:f}, {:f});", measurement.end.x,
                    measurement.end.y, -measurement.end.z);

      file.write_ln("}\n");
   }
}

void save_animations(const std::filesystem::path& path, const world& world)
{
   io::output_file file{path};

   for (auto& animation : world.animations) {
      file.write_ln("Animation(\"{}\", {:.2f}, {}, {})", animation.name,
                    animation.runtime, animation.loop ? "1" : "0",
                    animation.local_translation ? "1" : "0");
      file.write_ln("{");

      for (auto& key : animation.position_keys) {
         file.write_ln("\tAddPositionKey({:.2f}, {:.2f}, {:.2f}, {:.2f}, {}, "
                       "{:.2f}, {:.2f}, {:.2f}, {:.2f}, {:.2f}, {:.2f});",
                       key.time, key.position.x, key.position.y, key.position.z,
                       static_cast<int>(key.transition), key.tangent.x,
                       key.tangent.y, key.tangent.z, key.tangent_next.x,
                       key.tangent_next.y, key.tangent_next.z);
      }

      for (auto& key : animation.rotation_keys) {
         file.write_ln("\tAddRotationKey({:.2f}, {:.2f}, {:.2f}, {:.2f}, {}, "
                       "{:.2f}, {:.2f}, {:.2f}, {:.2f}, {:.2f}, {:.2f});",
                       key.time, key.rotation.x, key.rotation.y, key.rotation.z,
                       static_cast<int>(key.transition), key.tangent.x,
                       key.tangent.y, key.tangent.z, key.tangent_next.x,
                       key.tangent_next.y, key.tangent_next.z);
      }

      file.write_ln("}\n");
   }

   for (auto& group : world.animation_groups) {
      file.write_ln("AnimationGroup(\"{}\", {}, {})", group.name,
                    group.play_when_level_begins ? "1" : "0",
                    group.stops_when_object_is_controlled ? "1" : "0");
      file.write_ln("{");

      if (group.disable_hierarchies) {
         file.write_ln("\tDisableHierarchies();");
      }

      for (auto& entry : group.entries) {
         file.write_ln("\tAnimation(\"{}\", \"{}\");", entry.animation, entry.object);
      }

      file.write_ln("}\n");
   }

   for (auto& hierarchy : world.animation_hierarchies) {
      file.write_ln("Hierarchy(\"{}\")", hierarchy.root_object);
      file.write_ln("{");

      for (auto& object : hierarchy.objects) {
         file.write_ln("\tObj(\"{}\");", object);
      }

      file.write_ln("}\n");
   }
}

/// @brief Saves a world layer.
/// @param world_dir The directory to save the layer into.
/// @param layer_name The name of the layer ie `test` or `test_conquest`.
/// @param layer_index The index of the layer, 0 is special and indicates the base layer.
/// @param world The world that is being saved.
void save_layer(const std::filesystem::path& world_dir,
                const std::string_view layer_name, const int layer_index,
                const world& world, sequence_numbers& sequence_numbers)
{
   save_objects(world_dir / layer_name += (layer_index == 0 ? L".wld"sv : L".lyr"sv),
                layer_name, layer_index, world, sequence_numbers);

   save_paths(world_dir / layer_name += L".pth"sv, layer_index, world);
   save_regions(world_dir / layer_name += L".rgn"sv, layer_index, world);
   save_lights(world_dir / layer_name += L".lgt"sv, layer_index, world, sequence_numbers);
   save_hintnodes(world_dir / layer_name += L".hnt"sv, layer_index, world);

   if (layer_index == 0) {
      save_boundaries(world_dir / layer_name += L".bnd"sv, world);
      save_barriers(world_dir / layer_name += L".bar"sv, world);
      save_planning(world_dir / layer_name += L".pln"sv, world);
      save_portals_sectors(world_dir / layer_name += L".pvs"sv, world);
      save_measurements(world_dir / layer_name += L".msr"sv, world);
      save_animations(world_dir / layer_name += L".anm"sv, world);
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

   if (not world.game_modes.empty()) { // for possible BF1 support
      for (auto& game_mode : world.game_modes) {
         file.write_ln("GameMode(\"{}\")", game_mode.name);
         file.write_ln("{");
         for (auto& layer : game_mode.layers) {
            file.write_ln("\tLayer({});", layer);
         }
         file.write_ln("}\n");
      }
   }
}

void save_requirements(const std::filesystem::path& world_dir,
                       const std::string_view world_name, const world& world)
{
   if (not world.requirements.empty()) {
      assets::req::save(world_dir / fmt::format("{}.req", world_name),
                        world.requirements);
   }

   for (std::size_t i = 1; i < world.game_modes.size(); ++i) {
      if (world.game_modes[i].requirements.empty()) continue;

      assets::req::save(world_dir / fmt::format("{}_{}.mrq", world_name,
                                                world.game_modes[i].name),
                        world.game_modes[i].requirements);
   }
}

void garbage_collect_files(const std::filesystem::path& world_dir,
                           const std::string_view world_name, const world& world)
{
   constexpr std::array<std::string_view, 5> layer_files{"lyr", "pth", "rgn",
                                                         "lgt", "hnt"};

   for (const auto& layer : world.deleted_layers) {
      for (const auto& file : layer_files) {
         try {
            [[maybe_unused]] std::error_code ec{};

            std::filesystem::remove(world_dir / fmt::format("{}_{}.{}", world_name,
                                                            layer, file),
                                    ec);
         }
         catch (std::exception&) {
            // ... No need to care about this.
         }
      }
   }

   for (const auto& game_mode : world.deleted_game_modes) {
      try {
         [[maybe_unused]] std::error_code ec{};

         std::filesystem::remove(world_dir / fmt::format("{}_{}.mrq", world_name, game_mode),
                                 ec);
      }
      catch (std::exception&) {
         // ... No need to care about this.
      }
   }
}

}

void save_world(const std::filesystem::path& path, const world& world,
                const std::span<const terrain_cut> terrain_cuts)
{
   const auto world_dir = path.parent_path();
   const auto world_name = path.stem().string();

   sequence_numbers sequence_numbers;

   garbage_collect_files(world_dir, world_name, world);

   save_layer_index(std::filesystem::path{path}.replace_extension(L".ldx"sv), world);

   save_layer(world_dir, world_name, 0, world, sequence_numbers);

   for (std::size_t i = 1; i < world.layer_descriptions.size(); ++i) {
      auto& layer = world.layer_descriptions[i];

      save_layer(world_dir, world_name + "_"s + layer.name,
                 static_cast<uint32>(i), world, sequence_numbers);
   }

   save_terrain(std::filesystem::path{path}.replace_extension(L".ter"sv),
                world.terrain, terrain_cuts);
   save_requirements(world_dir, world_name, world);
   save_effects(std::filesystem::path{path}.replace_extension(L".fx"sv), world.effects);
}
}