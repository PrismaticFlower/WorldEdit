#include "save_entity_group.hpp"
#include "world/utility/boundary_nodes.hpp"

#include "io/output_file.hpp"
#include "math/vector_funcs.hpp"

#include <iterator>

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

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

struct string_buffer {
   string_buffer()
   {
      _str.reserve(65536);
   }

   template<typename... Args>
   void write_ln(const fmt::format_string<Args...> str, const Args&... args) noexcept
   {
      vwrite_ln(str, fmt::make_format_args(args...));
   }

   void write_ln(const std::string_view write_str) noexcept
   {
      _str += write_str;

      _str.push_back('\n');
   }

   void vwrite_ln(const fmt::string_view format, fmt::format_args args) noexcept
   {
      fmt::vformat_to(std::back_insert_iterator{_str}, format, args);

      _str.push_back('\n');
   }

   auto finish() noexcept -> std::string
   {
      return std::move(_str);
   }

private:
   std::string _str;
};

template<typename File>
void save_entity_group_impl(File& file, const entity_group& group)
{
   for (const object& object : group.objects) {
      const quaternion rotation = flip_rotation(object.rotation);
      const float3 position = flip_position(object.position);

      file.write_ln("Object(\"{}\", \"{}\")", object.name, object.class_name);
      file.write_ln("{");

      file.write_ln("\tChildRotation({:f}, {:f}, {:f}, {:f});", rotation.w,
                    rotation.x, rotation.y, rotation.z);
      file.write_ln("\tChildPosition({:f}, {:f}, {:f});", position.x,
                    position.y, position.z);
      file.write_ln("\tTeam({});", object.team);

      for (auto& prop : object.instance_properties) {
         file.write_ln("\t{}(\"{}\");", prop.key, prop.value);
      }

      file.write_ln("}\n");
   }

   for (const light& light : group.lights) {
      const quaternion rotation = flip_rotation(light.rotation);
      const float3 position = flip_position(light.position);

      file.write_ln("Light(\"{}\")", light.name);
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
         file.write_ln("\tTexture(\"{}\", {});", light.texture,
                       static_cast<int>(light.texture_addressing));
      }
      if (light.specular_caster) file.write_ln("\tCastSpecular(1);");

      if (light.light_type == light_type::directional) {
         file.write_ln("\tPS2BlendMode({});", static_cast<int>(light.ps2_blend_mode));
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
         file.write_ln("\tPS2BlendMode({});", static_cast<int>(light.ps2_blend_mode));
         file.write_ln("\tBidirectional({});", light.bidirectional ? 1 : 0);
      }
      else if (light.light_type == light_type::directional_region_box or
               light.light_type == light_type::directional_region_sphere or
               light.light_type == light_type::directional_region_cylinder) {
         file.write_ln("\tPS2BlendMode({});", static_cast<int>(light.ps2_blend_mode));
         file.write_ln("\tTileUV({:f}, {:f});", light.directional_texture_tiling.x,
                       light.directional_texture_tiling.y);
         file.write_ln("\tOffsetUV({:f}, {:f});",
                       light.directional_texture_offset.x,
                       light.directional_texture_offset.y);

         const quaternion region_rotation = flip_rotation(light.region_rotation);

         file.write_ln("\tRegionName(\"{}\");", light.region_name);
         file.write_ln("\tRegionRotation({:f}, {:f}, {:f}, {:f});",
                       region_rotation.w, region_rotation.x, region_rotation.y,
                       region_rotation.z);
         file.write_ln("\tRegionSize({:f}, {:f}, {:f});", light.region_size.x,
                       light.region_size.y, light.region_size.z);
      }

      file.write_ln("}\n");
   }

   for (const path& path : group.paths) {
      const std::string_view name_prefix = [&] {
         switch (path.type) {
         default:
         case path_type::none:
            return "";
         case path_type::entity_follow:
            return "type_EntityPath ";
         case path_type::formation:
            return "type_EntityFormation ";
         case path_type::patrol:
            return "type_PatrolPath ";
         }
      }();

      file.write_ln("Path(\"{}{}\")", name_prefix, path.name);
      file.write_ln("{");

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

      for (const path::node& node : path.nodes) {
         const quaternion rotation = flip_rotation(node.rotation);
         const float3 position = flip_position(node.position);

         file.write_ln("\t\tNode()");
         file.write_ln("\t\t{");

         file.write_ln("\t\t\tPosition({:f}, {:f}, {:f});", position.x,
                       position.y, position.z);
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

   for (const region& region : group.regions) {
      const quaternion rotation = flip_rotation(region.rotation);
      const float3 position = flip_position(region.position);

      file.write_ln("Region(\"{}\", {})", region.description,
                    static_cast<int>(region.shape));
      file.write_ln("{");

      file.write_ln("\tPosition({:f}, {:f}, {:f});", position.x, position.y,
                    position.z);
      file.write_ln("\tRotation({:f}, {:f}, {:f}, {:f});", rotation.w,
                    rotation.x, rotation.y, rotation.z);
      file.write_ln("\tSize({:f}, {:f}, {:f});", region.size.x, region.size.y,
                    region.size.z);
      file.write_ln("\tName(\"{}\");", region.name);

      file.write_ln("}\n");
   }

   for (const sector& sector : group.sectors) {
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

      file.write_ln("}\n");
   }

   for (const portal& portal : group.portals) {
      const quaternion rotation = flip_rotation(portal.rotation);
      const float3 position = flip_position(portal.position);

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

      file.write_ln("}\n");
   }

   for (const hintnode& hint : group.hintnodes) {
      const quaternion rotation = flip_rotation(hint.rotation);
      const float3 position = flip_position(hint.position);

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

      file.write_ln("}\n");
   }

   for (const barrier& barrier : group.barriers) {
      file.write_ln("Barrier(\"{}\")", barrier.name);
      file.write_ln("{");

      for (auto& corner : make_barrier_corners(barrier)) {
         file.write_ln("\tCorner({:f}, {:f}, {:f});", corner.x, corner.y,
                       -corner.z);
      }

      file.write_ln("\tFlag({});", static_cast<int>(barrier.flags));

      file.write_ln("}\n");
   }

   for (uint32 i = 0; i < group.planning_hubs.size(); ++i) {
      const world::planning_hub& hub = group.planning_hubs[i];

      file.write_ln("Hub(\"{}\")", hub.name);
      file.write_ln("{");

      file.write_ln("\tPos({:f}, {:f}, {:f});", hub.position.x, hub.position.y,
                    -hub.position.z);

      file.write_ln("\tRadius({:f});", hub.radius);

      for (auto& weights : hub.weights) {
         const std::string_view target_hub =
            group.planning_hubs[weights.hub_index].name;
         const std::string_view connection_name =
            group.planning_connections[weights.connection_index].name;

         if (weights.flyer > 0.0f) {
            file.write_ln("\tBranchWeight(\"{}\",{:f},\"{}\",{});", target_hub,
                          weights.flyer, connection_name,
                          static_cast<int>(ai_path_flags::flyer));
         }

         if (weights.huge > 0.0f) {
            file.write_ln("\tBranchWeight(\"{}\",{:f},\"{}\",{});", target_hub,
                          weights.huge, connection_name,
                          static_cast<int>(ai_path_flags::huge));
         }

         if (weights.medium > 0.0f) {
            file.write_ln("\tBranchWeight(\"{}\",{:f},\"{}\",{});", target_hub,
                          weights.medium, connection_name,
                          static_cast<int>(ai_path_flags::medium));
         }

         if (weights.small > 0.0f) {
            file.write_ln("\tBranchWeight(\"{}\",{:f},\"{}\",{});", target_hub,
                          weights.small, connection_name,
                          static_cast<int>(ai_path_flags::small));
         }

         if (weights.hover > 0.0f) {
            file.write_ln("\tBranchWeight(\"{}\",{:f},\"{}\",{});", target_hub,
                          weights.hover, connection_name,
                          static_cast<int>(ai_path_flags::hover));
         }

         if (weights.soldier > 0.0f) {
            file.write_ln("\tBranchWeight(\"{}\",{:f},\"{}\",{});", target_hub,
                          weights.soldier, connection_name,
                          static_cast<int>(ai_path_flags::soldier));
         }
      }

      file.write_ln("}\n");
   }

   for (const planning_connection& connection : group.planning_connections) {
      file.write_ln("Connection(\"{}\")", connection.name);
      file.write_ln("{");

      file.write_ln("\tStart(\"{}\");",
                    group.planning_hubs[connection.start_hub_index].name);
      file.write_ln("\tEnd(\"{}\");",
                    group.planning_hubs[connection.end_hub_index].name);
      file.write_ln("\tFlag({});", static_cast<int>(connection.flags));

      if (connection.dynamic_group != 0) {
         file.write_ln("\tDynamic({});", static_cast<int>(connection.dynamic_group));
      }

      if (connection.jump) file.write_ln("\tJump();");
      if (connection.jet_jump) file.write_ln("\tJetJump();");
      if (connection.one_way) file.write_ln("\tOneWay();");

      file.write_ln("}\n");
   }

   for (const boundary& boundary : group.boundaries) {
      file.write_ln("Boundary(\"{}\")", boundary.name);
      file.write_ln("{");

      for (const float3& node : get_boundary_nodes(boundary)) {
         file.write_ln("\tNode({:f}, {:f}, {:f});", node.x, node.y, -node.z);
      }

      file.write_ln("}\n");
   }

   for (const measurement& measurement : group.measurements) {
      file.write_ln("Measurement(\"{}\")", measurement.name);
      file.write_ln("{");

      file.write_ln("\tStart({:f}, {:f}, {:f});", measurement.start.x,
                    measurement.start.y, -measurement.start.z);
      file.write_ln("\tEnd({:f}, {:f}, {:f});", measurement.end.x,
                    measurement.end.y, -measurement.end.z);

      file.write_ln("}\n");
   }

   if (not group.blocks.boxes.empty()) {
      file.write_ln("BlocksBoxes({})", group.blocks.boxes.size());
      file.write_ln("{");

      for (const block_description_box& box : group.blocks.boxes) {
         file.write_ln("   Box()");
         file.write_ln("   {");

         file.write_ln("      Rotation({}, {}, {}, {});", box.rotation.w,
                       box.rotation.x, box.rotation.y, box.rotation.z);
         file.write_ln("      Position({}, {}, {});", box.position.x,
                       box.position.y, box.position.z);
         file.write_ln("      Size({}, {}, {});", box.size.x, box.size.y,
                       box.size.z);
         file.write_ln("      SurfaceMaterials({}, {}, {}, {}, {}, {});",
                       box.surface_materials[0], box.surface_materials[1],
                       box.surface_materials[2], box.surface_materials[3],
                       box.surface_materials[4], box.surface_materials[5]);
         file.write_ln("      SurfaceTextureMode({}, {}, {}, {}, {}, {});",
                       std::to_underlying(box.surface_texture_mode[0]),
                       std::to_underlying(box.surface_texture_mode[1]),
                       std::to_underlying(box.surface_texture_mode[2]),
                       std::to_underlying(box.surface_texture_mode[3]),
                       std::to_underlying(box.surface_texture_mode[4]),
                       std::to_underlying(box.surface_texture_mode[5]));
         file.write_ln("      SurfaceTextureRotation({}, {}, {}, {}, {}, {});",
                       std::to_underlying(box.surface_texture_rotation[0]),
                       std::to_underlying(box.surface_texture_rotation[1]),
                       std::to_underlying(box.surface_texture_rotation[2]),
                       std::to_underlying(box.surface_texture_rotation[3]),
                       std::to_underlying(box.surface_texture_rotation[4]),
                       std::to_underlying(box.surface_texture_rotation[5]));
         file.write_ln(
            "      SurfaceTextureScale({}, {}, {}, {}, {}, {}, {}, {}, {}, "
            "{}, {}, {});",
            box.surface_texture_scale[0][0], box.surface_texture_scale[0][1],
            box.surface_texture_scale[1][0], box.surface_texture_scale[1][1],
            box.surface_texture_scale[2][0], box.surface_texture_scale[2][1],
            box.surface_texture_scale[3][0], box.surface_texture_scale[3][1],
            box.surface_texture_scale[4][0], box.surface_texture_scale[4][1],
            box.surface_texture_scale[5][0], box.surface_texture_scale[5][1]);
         file.write_ln(
            "      SurfaceTextureOffset({}, {}, {}, {}, {}, {}, {}, {}, {}, "
            "{}, {}, {});",
            box.surface_texture_offset[0][0], box.surface_texture_offset[0][1],
            box.surface_texture_offset[1][0], box.surface_texture_offset[1][1],
            box.surface_texture_offset[2][0], box.surface_texture_offset[2][1],
            box.surface_texture_offset[3][0], box.surface_texture_offset[3][1],
            box.surface_texture_offset[4][0], box.surface_texture_offset[4][1],
            box.surface_texture_offset[5][0], box.surface_texture_offset[5][1]);
         file.write_ln("   }");
      }

      file.write_ln("}\n");
   }

   if (not group.blocks.ramps.empty()) {
      file.write_ln("BlocksRamps({})", group.blocks.ramps.size());
      file.write_ln("{");

      for (const block_description_ramp& ramp : group.blocks.ramps) {
         file.write_ln("   Ramp()");
         file.write_ln("   {");

         file.write_ln("      Rotation({}, {}, {}, {});", ramp.rotation.w,
                       ramp.rotation.x, ramp.rotation.y, ramp.rotation.z);
         file.write_ln("      Position({}, {}, {});", ramp.position.x,
                       ramp.position.y, ramp.position.z);
         file.write_ln("      Size({}, {}, {});", ramp.size.x, ramp.size.y,
                       ramp.size.z);
         file.write_ln("      SurfaceMaterials({}, {}, {}, {}, {});",
                       ramp.surface_materials[0], ramp.surface_materials[1],
                       ramp.surface_materials[2], ramp.surface_materials[3],
                       ramp.surface_materials[4]);
         file.write_ln("      SurfaceTextureMode({}, {}, {}, {}, {});",
                       std::to_underlying(ramp.surface_texture_mode[0]),
                       std::to_underlying(ramp.surface_texture_mode[1]),
                       std::to_underlying(ramp.surface_texture_mode[2]),
                       std::to_underlying(ramp.surface_texture_mode[3]),
                       std::to_underlying(ramp.surface_texture_mode[4]));
         file.write_ln("      SurfaceTextureRotation({}, {}, {}, {}, {});",
                       std::to_underlying(ramp.surface_texture_rotation[0]),
                       std::to_underlying(ramp.surface_texture_rotation[1]),
                       std::to_underlying(ramp.surface_texture_rotation[2]),
                       std::to_underlying(ramp.surface_texture_rotation[3]),
                       std::to_underlying(ramp.surface_texture_rotation[4]));
         file.write_ln("      SurfaceTextureScale({}, {}, {}, {}, {}, {}, {}, "
                       "{}, {}, {});",
                       ramp.surface_texture_scale[0][0],
                       ramp.surface_texture_scale[0][1],
                       ramp.surface_texture_scale[1][0],
                       ramp.surface_texture_scale[1][1],
                       ramp.surface_texture_scale[2][0],
                       ramp.surface_texture_scale[2][1],
                       ramp.surface_texture_scale[3][0],
                       ramp.surface_texture_scale[3][1],
                       ramp.surface_texture_scale[4][0],
                       ramp.surface_texture_scale[4][1]);
         file.write_ln("      SurfaceTextureOffset({}, {}, {}, {}, {}, {}, {}, "
                       "{}, {}, {});",
                       ramp.surface_texture_offset[0][0],
                       ramp.surface_texture_offset[0][1],
                       ramp.surface_texture_offset[1][0],
                       ramp.surface_texture_offset[1][1],
                       ramp.surface_texture_offset[2][0],
                       ramp.surface_texture_offset[2][1],
                       ramp.surface_texture_offset[3][0],
                       ramp.surface_texture_offset[3][1],
                       ramp.surface_texture_offset[4][0],
                       ramp.surface_texture_offset[4][1]);
         file.write_ln("   }");
      }

      file.write_ln("}\n");
   }

   if (not group.blocks.quads.empty()) {
      file.write_ln("BlocksQuads({})", group.blocks.quads.size());
      file.write_ln("{");

      for (const block_description_quad& quad : group.blocks.quads) {
         file.write_ln("   Quad()");
         file.write_ln("   {");

         file.write_ln("      Vertex0({}, {}, {});", quad.vertices[0].x,
                       quad.vertices[0].y, quad.vertices[0].z);
         file.write_ln("      Vertex1({}, {}, {});", quad.vertices[1].x,
                       quad.vertices[1].y, quad.vertices[1].z);
         file.write_ln("      Vertex2({}, {}, {});", quad.vertices[2].x,
                       quad.vertices[2].y, quad.vertices[2].z);
         file.write_ln("      Vertex3({}, {}, {});", quad.vertices[3].x,
                       quad.vertices[3].y, quad.vertices[3].z);
         file.write_ln("      QuadSplit({});", std::to_underlying(quad.quad_split));
         file.write_ln("      SurfaceMaterials({});", quad.surface_materials[0]);
         file.write_ln("      SurfaceTextureMode({});",
                       std::to_underlying(quad.surface_texture_mode[0]));
         file.write_ln("      SurfaceTextureRotation({});",
                       std::to_underlying(quad.surface_texture_rotation[0]));
         file.write_ln("      SurfaceTextureScale({}, {});",
                       quad.surface_texture_scale[0][0],
                       quad.surface_texture_scale[0][1]);
         file.write_ln("      SurfaceTextureOffset({}, {});",
                       quad.surface_texture_offset[0][0],
                       quad.surface_texture_offset[0][1]);
         file.write_ln("   }");
      }

      file.write_ln("}\n");
   }

   if (not group.blocks.custom.description.empty()) {
      file.write_ln("BlocksCustom({})", group.blocks.custom.description.size());
      file.write_ln("{");

      for (const block_description_custom& block : group.blocks.custom.description) {
         switch (block.mesh_description.type) {
         case block_custom_mesh_type::stairway: {
            file.write_ln("   Stairway()");
         } break;
         case block_custom_mesh_type::stairway_floating: {
            file.write_ln("   StairwayFloating()");
         } break;
         case block_custom_mesh_type::ring: {
            file.write_ln("   Ring()");
         } break;
         case block_custom_mesh_type::beveled_box: {
            file.write_ln("   BeveledBox()");
         } break;
         case block_custom_mesh_type::curve: {
            file.write_ln("   CubicCurve()");
         } break;
         case block_custom_mesh_type::cylinder: {
            file.write_ln("   Cylinder()");
         } break;
         case block_custom_mesh_type::cone: {
            file.write_ln("   Cone()");
         } break;
         case block_custom_mesh_type::arch: {
            file.write_ln("   Arch()");
         } break;
         }

         file.write_ln("   {");

         file.write_ln("      Rotation({}, {}, {}, {});", block.rotation.w,
                       block.rotation.x, block.rotation.y, block.rotation.z);
         file.write_ln("      Position({}, {}, {});", block.position.x,
                       block.position.y, block.position.z);

         switch (block.mesh_description.type) {
         case block_custom_mesh_type::stairway: {
            const block_custom_mesh_description_stairway& stairway =
               block.mesh_description.stairway;

            file.write_ln("      Size({}, {}, {});", stairway.size.x,
                          stairway.size.y, stairway.size.z);
            file.write_ln("      StepHeight({});", stairway.step_height);
            file.write_ln("      FirstStepOffset({});", stairway.first_step_offset);
         } break;
         case block_custom_mesh_type::stairway_floating: {
            const block_custom_mesh_description_stairway_floating& stairway =
               block.mesh_description.stairway_floating;

            file.write_ln("      Size({}, {}, {});", stairway.size.x,
                          stairway.size.y, stairway.size.z);
            file.write_ln("      StepHeight({});", stairway.step_height);
            file.write_ln("      FirstStepOffset({});", stairway.first_step_offset);
         } break;
         case block_custom_mesh_type::ring: {
            const world::block_custom_mesh_description_ring& ring =
               block.mesh_description.ring;

            file.write_ln("      InnerRadius({});", ring.inner_radius);
            file.write_ln("      OuterRadius({});", ring.outer_radius);
            file.write_ln("      Height({});", ring.height);
            file.write_ln("      Segments({});", ring.segments);
            if (ring.flat_shading) file.write_ln("      FlatShading();");
            file.write_ln("      TextureLoops({});", ring.texture_loops);
         } break;
         case block_custom_mesh_type::beveled_box: {
            const world::block_custom_mesh_description_beveled_box& box =
               block.mesh_description.beveled_box;

            file.write_ln("      Size({}, {}, {});", box.size.x, box.size.y,
                          box.size.z);
            file.write_ln("      Amount({});", box.amount);
            file.write_ln("      BevelTop({:d});", box.bevel_top);
            file.write_ln("      BevelSides({:d});", box.bevel_sides);
            file.write_ln("      BevelBottom({:d});", box.bevel_bottom);
         } break;
         case block_custom_mesh_type::curve: {
            const world::block_custom_mesh_description_curve& curve =
               block.mesh_description.curve;

            file.write_ln("      Width({});", curve.width);
            file.write_ln("      Height({});", curve.height);
            file.write_ln("      Segments({});", curve.segments);
            file.write_ln("      TextureLoops({});", curve.texture_loops);

            file.write_ln("      P0({}, {}, {});", curve.p0.x, curve.p0.y,
                          curve.p0.z);
            file.write_ln("      P1({}, {}, {});", curve.p1.x, curve.p1.y,
                          curve.p1.z);
            file.write_ln("      P2({}, {}, {});", curve.p2.x, curve.p2.y,
                          curve.p2.z);
            file.write_ln("      P3({}, {}, {});", curve.p3.x, curve.p3.y,
                          curve.p3.z);
         } break;
         case block_custom_mesh_type::cylinder: {
            const world::block_custom_mesh_description_cylinder& cylinder =
               block.mesh_description.cylinder;

            file.write_ln("      Size({}, {}, {});", cylinder.size.x,
                          cylinder.size.y, cylinder.size.z);
            file.write_ln("      Segments({});", cylinder.segments);
            if (cylinder.flat_shading) file.write_ln("      FlatShading();");
            file.write_ln("      TextureLoops({});", cylinder.texture_loops);
         } break;
         case block_custom_mesh_type::cone: {
            const world::block_custom_mesh_description_cone& cone =
               block.mesh_description.cone;

            file.write_ln("      Size({}, {}, {});", cone.size.x, cone.size.y,
                          cone.size.z);
            file.write_ln("      Segments({});", cone.segments);
            if (cone.flat_shading) file.write_ln("      FlatShading();");
         } break;
         case block_custom_mesh_type::arch: {
            const world::block_custom_mesh_description_arch& arch =
               block.mesh_description.arch;

            file.write_ln("      Size({}, {}, {});", arch.size.x, arch.size.y,
                          arch.size.z);
            file.write_ln("      CrownLength({});", arch.crown_length);
            file.write_ln("      CrownHeight({});", arch.crown_height);
            file.write_ln("      CurveHeight({});", arch.curve_height);
            file.write_ln("      SpanLength({});", arch.span_length);
            file.write_ln("      Segments({});", arch.segments);
         } break;
         }

         file.write_ln("      SurfaceMaterials({}, {}, {}, {}, {}, {});",
                       block.surface_materials[0], block.surface_materials[1],
                       block.surface_materials[2], block.surface_materials[3],
                       block.surface_materials[4], block.surface_materials[5]);
         file.write_ln("      SurfaceTextureMode({}, {}, {}, {}, {}, {});",
                       std::to_underlying(block.surface_texture_mode[0]),
                       std::to_underlying(block.surface_texture_mode[1]),
                       std::to_underlying(block.surface_texture_mode[2]),
                       std::to_underlying(block.surface_texture_mode[3]),
                       std::to_underlying(block.surface_texture_mode[4]),
                       std::to_underlying(block.surface_texture_mode[5]));
         file.write_ln("      SurfaceTextureRotation({}, {}, {}, {}, {}, {});",
                       std::to_underlying(block.surface_texture_rotation[0]),
                       std::to_underlying(block.surface_texture_rotation[1]),
                       std::to_underlying(block.surface_texture_rotation[2]),
                       std::to_underlying(block.surface_texture_rotation[3]),
                       std::to_underlying(block.surface_texture_rotation[4]),
                       std::to_underlying(block.surface_texture_rotation[5]));
         file.write_ln(
            "      SurfaceTextureScale({}, {}, {}, {}, {}, {}, {}, {}, {}, "
            "{}, {}, {});",
            block.surface_texture_scale[0][0], block.surface_texture_scale[0][1],
            block.surface_texture_scale[1][0], block.surface_texture_scale[1][1],
            block.surface_texture_scale[2][0], block.surface_texture_scale[2][1],
            block.surface_texture_scale[3][0], block.surface_texture_scale[3][1],
            block.surface_texture_scale[4][0], block.surface_texture_scale[4][1],
            block.surface_texture_scale[5][0], block.surface_texture_scale[5][1]);
         file.write_ln(
            "      SurfaceTextureOffset({}, {}, {}, {}, {}, {}, {}, {}, {}, "
            "{}, {}, {});",
            block.surface_texture_offset[0][0], block.surface_texture_offset[0][1],
            block.surface_texture_offset[1][0], block.surface_texture_offset[1][1],
            block.surface_texture_offset[2][0], block.surface_texture_offset[2][1],
            block.surface_texture_offset[3][0], block.surface_texture_offset[3][1],
            block.surface_texture_offset[4][0], block.surface_texture_offset[4][1],
            block.surface_texture_offset[5][0], block.surface_texture_offset[5][1]);
         file.write_ln("   }");
      }

      file.write_ln("}\n");
   }

   if (not group.blocks.hemispheres.empty()) {
      file.write_ln("BlocksHemispheres({})", group.blocks.hemispheres.size());
      file.write_ln("{");

      for (const block_description_hemisphere& hemisphere : group.blocks.hemispheres) {
         file.write_ln("   Hemisphere()");
         file.write_ln("   {");

         file.write_ln("      Rotation({}, {}, {}, {});", hemisphere.rotation.w,
                       hemisphere.rotation.x, hemisphere.rotation.y,
                       hemisphere.rotation.z);
         file.write_ln("      Position({}, {}, {});", hemisphere.position.x,
                       hemisphere.position.y, hemisphere.position.z);
         file.write_ln("      Size({}, {}, {});", hemisphere.size.x,
                       hemisphere.size.y, hemisphere.size.z);
         file.write_ln("      SurfaceMaterials({}, {});",
                       hemisphere.surface_materials[0],
                       hemisphere.surface_materials[1]);
         file.write_ln("      SurfaceTextureMode({}, {});",
                       std::to_underlying(hemisphere.surface_texture_mode[0]),
                       std::to_underlying(hemisphere.surface_texture_mode[1]));
         file.write_ln("      SurfaceTextureRotation({}, {});",
                       std::to_underlying(hemisphere.surface_texture_rotation[0]),
                       std::to_underlying(hemisphere.surface_texture_rotation[1]));
         file.write_ln("      SurfaceTextureScale({}, {}, {}, {});",
                       hemisphere.surface_texture_scale[0][0],
                       hemisphere.surface_texture_scale[0][1],
                       hemisphere.surface_texture_scale[1][0],
                       hemisphere.surface_texture_scale[1][1]);
         file.write_ln("      SurfaceTextureOffset({}, {}, {}, {});",
                       hemisphere.surface_texture_offset[0][0],
                       hemisphere.surface_texture_offset[0][1],
                       hemisphere.surface_texture_offset[1][0],
                       hemisphere.surface_texture_offset[1][1]);
         file.write_ln("   }");
      }

      file.write_ln("}\n");
   }

   if (not group.blocks.pyramids.empty()) {
      file.write_ln("BlocksPyramids({})", group.blocks.pyramids.size());
      file.write_ln("{");

      for (const block_description_pyramid& pyramid : group.blocks.pyramids) {
         file.write_ln("   Pyramid()");
         file.write_ln("   {");

         file.write_ln("      Rotation({}, {}, {}, {});", pyramid.rotation.w,
                       pyramid.rotation.x, pyramid.rotation.y, pyramid.rotation.z);
         file.write_ln("      Position({}, {}, {});", pyramid.position.x,
                       pyramid.position.y, pyramid.position.z);
         file.write_ln("      Size({}, {}, {});", pyramid.size.x,
                       pyramid.size.y, pyramid.size.z);
         file.write_ln("      SurfaceMaterials({}, {}, {}, {}, {});",
                       pyramid.surface_materials[0], pyramid.surface_materials[1],
                       pyramid.surface_materials[2], pyramid.surface_materials[3],
                       pyramid.surface_materials[4]);
         file.write_ln("      SurfaceTextureMode({}, {}, {}, {}, {});",
                       std::to_underlying(pyramid.surface_texture_mode[0]),
                       std::to_underlying(pyramid.surface_texture_mode[1]),
                       std::to_underlying(pyramid.surface_texture_mode[2]),
                       std::to_underlying(pyramid.surface_texture_mode[3]),
                       std::to_underlying(pyramid.surface_texture_mode[4]));
         file.write_ln("      SurfaceTextureRotation({}, {}, {}, {}, {});",
                       std::to_underlying(pyramid.surface_texture_rotation[0]),
                       std::to_underlying(pyramid.surface_texture_rotation[1]),
                       std::to_underlying(pyramid.surface_texture_rotation[2]),
                       std::to_underlying(pyramid.surface_texture_rotation[3]),
                       std::to_underlying(pyramid.surface_texture_rotation[4]));
         file.write_ln("      SurfaceTextureScale({}, {}, {}, {}, {}, {}, {}, "
                       "{}, {}, {});",
                       pyramid.surface_texture_scale[0][0],
                       pyramid.surface_texture_scale[0][1],
                       pyramid.surface_texture_scale[1][0],
                       pyramid.surface_texture_scale[1][1],
                       pyramid.surface_texture_scale[2][0],
                       pyramid.surface_texture_scale[2][1],
                       pyramid.surface_texture_scale[3][0],
                       pyramid.surface_texture_scale[3][1],
                       pyramid.surface_texture_scale[4][0],
                       pyramid.surface_texture_scale[4][1]);
         file.write_ln("      SurfaceTextureOffset({}, {}, {}, {}, {}, {}, {}, "
                       "{}, {}, {});",
                       pyramid.surface_texture_offset[0][0],
                       pyramid.surface_texture_offset[0][1],
                       pyramid.surface_texture_offset[1][0],
                       pyramid.surface_texture_offset[1][1],
                       pyramid.surface_texture_offset[2][0],
                       pyramid.surface_texture_offset[2][1],
                       pyramid.surface_texture_offset[3][0],
                       pyramid.surface_texture_offset[3][1],
                       pyramid.surface_texture_offset[4][0],
                       pyramid.surface_texture_offset[4][1]);
         file.write_ln("   }");
      }

      file.write_ln("}\n");
   }

   if (not group.blocks.terrain_cut_boxes.empty()) {
      file.write_ln("BlocksTerrainCutBoxes({})",
                    group.blocks.terrain_cut_boxes.size());
      file.write_ln("{");

      for (const block_description_terrain_cut_box& terrain_cut_box :
           group.blocks.terrain_cut_boxes) {
         file.write_ln("   TerrainCutBox()");
         file.write_ln("   {");

         file.write_ln("      Rotation({}, {}, {}, {});",
                       terrain_cut_box.rotation.w, terrain_cut_box.rotation.x,
                       terrain_cut_box.rotation.y, terrain_cut_box.rotation.z);
         file.write_ln("      Position({}, {}, {});", terrain_cut_box.position.x,
                       terrain_cut_box.position.y, terrain_cut_box.position.z);
         file.write_ln("      Size({}, {}, {});", terrain_cut_box.size.x,
                       terrain_cut_box.size.y, terrain_cut_box.size.z);
         file.write_ln("   }");
      }

      file.write_ln("}\n");
   }

   if (not group.blocks.materials.empty()) {
      file.write_ln("BlocksMaterials({})", group.blocks.materials.size());
      file.write_ln("{");

      for (const block_material& material : group.blocks.materials) {
         file.write_ln("   Material()");
         file.write_ln("   {");
         file.write_ln("      Name(\"{}\");", material.name);
         file.write_ln("      DiffuseMap(\"{}\");", material.diffuse_map);
         file.write_ln("      NormalMap(\"{}\");", material.normal_map);
         file.write_ln("      DetailMap(\"{}\");", material.detail_map);
         file.write_ln("      EnvMap(\"{}\");", material.env_map);
         file.write_ln("      DetailTiling({}, {});", material.detail_tiling[0],
                       material.detail_tiling[1]);
         file.write_ln("      TileNormalMap({:d});", material.tile_normal_map);
         file.write_ln("      SpecularLighting({:d});", material.specular_lighting);
         file.write_ln("      SpecularColor({}, {}, {});",
                       material.specular_color.x, material.specular_color.y,
                       material.specular_color.z);
         file.write_ln("      FoleyFXGroup({});",
                       std::to_underlying(material.foley_group));
         file.write_ln("   }");
      }

      file.write_ln("}\n");
   }
}

}

void save_entity_group(const io::path& file_path, const entity_group& group)
{
   io::output_file file{file_path};

   save_entity_group_impl(file, group);
}

auto save_entity_group_to_string(const entity_group& group) noexcept -> std::string
{
   string_buffer buffer;

   save_entity_group_impl(buffer, group);

   return buffer.finish();
}

}