
#include "flat_model.hpp"
#include "bf_crc32.hpp"
#include "generate_tangents.hpp"

#include "math/matrix_funcs.hpp"
#include "math/plane_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include "utility/enum_bitflags.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>

#include <fmt/core.h>

using namespace std::literals;

using we::string::icontains;
using we::string::iequals;
using we::string::istarts_with;

namespace we::assets::msh {

namespace {

void scale_scene(scene& scene)
{
   const float scale = scene.options.scale;

   for (msh::node& node : scene.nodes) {
      node.transform.translation *= scale;

      for (msh::geometry_segment& segment : node.segments) {
         for (float3& position : segment.positions) {
            position *= scale;
         }
      }

      for (msh::shadow_volume& shadow_volume : node.shadow_volumes) {
         for (float3& position : shadow_volume.positions) {
            position *= scale;
         }
      }

      if (node.collision_primitive) {
         node.collision_primitive->radius *= scale;
         node.collision_primitive->height *= scale;
         node.collision_primitive->length *= scale;
      }

      if (node.cloth) {
         for (float3& position : node.cloth->positions) {
            position *= scale;
         }

         for (msh::cloth_collision_primitive& primitive : node.cloth->collision) {
            primitive.size *= scale;
         }
      }

      for (msh::shadow_volume& shadow_volume : node.shadow_volumes) {
         for (float3& position : shadow_volume.positions) {
            position *= scale;
         }
      }
   }
}

auto build_node_parents(const scene& scene) -> std::vector<uint32>
{
   std::vector<uint32> node_parents;
   node_parents.resize(scene.nodes.size());

   for (std::size_t node_index = 0; node_index < scene.nodes.size(); ++node_index) {
      const msh::node& node = scene.nodes[node_index];

      if (not node.parent) continue;

      for (uint32 parent_index = 0; parent_index < scene.nodes.size(); ++parent_index) {
         const msh::node& parent = scene.nodes[parent_index];

         if (*node.parent == parent.name) {
            node_parents[node_index] = parent_index;

            break;
         }
      }
   }

   return node_parents;
}

auto build_keep_nodes(const scene& scene) -> std::vector<bool>
{
   std::vector<bool> keep_node;
   keep_node.resize(scene.nodes.size());

   for (std::size_t i = 0; i < scene.nodes.size(); ++i) {
      const msh::node& node = scene.nodes[i];

      if (scene.options.keep_all or            //
          node.type == msh::node_type::bone or //
          not node.parent or                   //
          istarts_with(node.name, "hp_")) {
         keep_node[i] = true;
      }
      else {
         for (const std::string& keep : scene.options.keep_nodes) {
            if (iequals(node.name, keep)) {
               keep_node[i] = true;

               break;
            }
         }

         if (keep_node[i]) continue;

         const uint32 node_name_hash = bf_crc32(node.name);

         for (const uint32 bone_hash : scene.blend_bone_list) {
            if (node_name_hash == bone_hash) {
               keep_node[i] = true;

               break;
            }
         }

         if (keep_node[i]) continue;

         for (const msh::node& other_node : scene.nodes) {
            if (not other_node.cloth) continue;

            for (const msh::cloth_collision_primitive& primitive :
                 other_node.cloth->collision) {
               if (iequals(node.name, primitive.parent)) {

                  keep_node[i] = true;

                  break;
               }
            }

            if (keep_node[i]) break;
         }
      }
   }

   return keep_node;
}

auto build_node_from_vertex(const std::size_t node_index, const msh::scene& scene,
                            const std::vector<bool>& keep_node,
                            const std::vector<uint32>& node_parents) -> float4x4
{
   const msh::node& node = scene.nodes[node_index];

   if (not node.parent) return {};

   if (keep_node[node_index]) return {};

   const float4x4 node_from_vertex{node.transform};

   const std::size_t parent_index = node_parents[node_index];

   if (keep_node[parent_index]) return node_from_vertex;

   const float4x4 parent_from_vertex =
      build_node_from_vertex(parent_index, scene, keep_node, node_parents);

   return parent_from_vertex * node_from_vertex;
}

auto build_local_from_vertex(const std::size_t node_index, const msh::scene& scene,
                             const std::vector<uint32>& node_parents) -> float4x4
{
   const msh::node& node = scene.nodes[node_index];

   if (not node.parent) return {};

   const float4x4 node_from_parent{node.transform};
   const float4x4 parent_from_grandparent =
      build_local_from_vertex(node_parents[node_index], scene, node_parents);

   return parent_from_grandparent * node_from_parent;
}

bool is_mesh_node(const node& node) noexcept
{
   if (node.hidden) return false;

   if (node.type == msh::node_type::cloth) return false;

   if (node.name.starts_with("p_")) return false;
   if (node.name.starts_with("hp_")) return false;

   if (istarts_with(node.name, "collision")) return false;
   if (istarts_with(node.name, "terraincutter")) return false;

   if (icontains(node.name, "lod2")) return false;
   if (icontains(node.name, "lod3")) return false;
   if (icontains(node.name, "lowres")) return false;
   if (icontains(node.name, "lowrez")) return false;

   return true;
}

bool is_terrain_cut_node(const node& node) noexcept
{
   return istarts_with(node.name, "terraincutter"sv);
}

bool is_collision_node(const node& node) noexcept
{
   if (node.name.starts_with("p_"sv)) return true;
   if (istarts_with(node.name, "collision"sv)) return true;

   return false;
}

auto make_flat_scene_node(const node& base, const std::vector<node>& nodes) -> flat_model_node
{
   flat_model_node flat_node{.name = base.name,
                             .transform = {base.transform.translation,
                                           base.transform.rotation},
                             .type = base.type,
                             .hidden = base.hidden};

   flat_node.children.reserve(
      std::count_if(nodes.cbegin(), nodes.cend(),
                    [&](const node& node) { return node.parent == base.name; }));

   for (const auto& node : nodes) {
      if (node.parent != base.name) continue;

      flat_node.children.emplace_back(make_flat_scene_node(node, nodes));
   }

   return flat_node;
}

auto generate_normals(std::span<const float3> positions,
                      std::span<const std::array<uint16, 3>> triangles,
                      const float3x3 normal_node_to_object) -> std::vector<float3>
{
   std::vector<float3> normals{positions.size(), float3{}};

   for (const auto [i0, i1, i2] : triangles) {
      const float3 normal =
         cross(positions[i1] - positions[i0], positions[i2] - positions[i0]);

      normals[i0] += normal;
      normals[i1] += normal;
      normals[i2] += normal;
   }

   for (auto& normal : normals) {
      normal = normalize(normal_node_to_object * normal);
   }

   return normals;
}

void patch_materials_with_options(std::vector<mesh>& meshes, const scene_options& opts)
{
   for (auto& mesh : meshes) {
      for (auto& normal_map : opts.normal_maps) {
         if (iequals(mesh.material.textures[0], normal_map)) {
            mesh.material.textures[1] = mesh.material.textures[0] + "_bump";
            mesh.material.flags |= material_flags::perpixel;
         }
      }

      if (opts.additive_emissive and
          are_flags_set(mesh.material.flags, material_flags::unlit)) {
         mesh.material.flags |= material_flags::additive;
      }
   }
}

}

flat_model::flat_model(scene scene)
{
   scale_scene(scene);

   const std::vector<uint32> node_parents = build_node_parents(scene);
   const std::vector<bool> keep_nodes = build_keep_nodes(scene);

   std::vector<float4x4> node_from_vertex_transforms;
   node_from_vertex_transforms.resize(scene.nodes.size());
   std::vector<float4x4> local_from_vertex_transforms;
   local_from_vertex_transforms.resize(scene.nodes.size());

   for (std::size_t node_index = 0; node_index < scene.nodes.size(); ++node_index) {
      node_from_vertex_transforms[node_index] =
         build_node_from_vertex(node_index, scene, keep_nodes, node_parents);
      local_from_vertex_transforms[node_index] =
         build_local_from_vertex(node_index, scene, node_parents);
   }

   for (std::size_t i = 0; i < scene.nodes.size(); ++i) {
      const node& node = scene.nodes[i];
      const float4x4& node_from_vertex = node_from_vertex_transforms[i];
      const float4x4& local_from_vertex = local_from_vertex_transforms[i];

      if (is_mesh_node(node)) {
         flatten_segments_to_meshes(node.segments, node_from_vertex,
                                    scene.materials, scene.options);
      }
      else if (is_terrain_cut_node(node)) {
         flatten_segments_to_terrain_cut(node.segments, local_from_vertex, node.name);
      }
      else if (is_collision_node(node)) {
         flatten_node_to_collision(node, node_from_vertex, local_from_vertex);
      }
   }

   for (const auto& node : scene.nodes) {
      if (node.parent) continue;

      node_hierarchy.emplace_back(make_flat_scene_node(node, scene.nodes));
   }

   generate_tangents_for_meshes();
   patch_materials_with_options(meshes, scene.options);
   apply_ambient_lighting(scene.options);
   regenerate_bounding_boxes();
   build_ground_points();

   bvh = flat_model_bvh{meshes};
   terrain_cut_bvh = flat_model_terrain_cut_bvh{terrain_cuts};
}

void flat_model::regenerate_bounding_boxes() noexcept
{
   bounding_box = {.min = float3{std::numeric_limits<float>::max(),
                                 std::numeric_limits<float>::max(),
                                 std::numeric_limits<float>::max()},
                   .max = float3{std::numeric_limits<float>::lowest(),
                                 std::numeric_limits<float>::lowest(),
                                 std::numeric_limits<float>::lowest()}};
   terrain_cuts_bounding_box = {.min = float3{std::numeric_limits<float>::max(),
                                              std::numeric_limits<float>::max(),
                                              std::numeric_limits<float>::max()},
                                .max = float3{std::numeric_limits<float>::lowest(),
                                              std::numeric_limits<float>::lowest(),
                                              std::numeric_limits<float>::lowest()}};
   collision_bounding_box = {.min = float3{std::numeric_limits<float>::max(),
                                           std::numeric_limits<float>::max(),
                                           std::numeric_limits<float>::max()},
                             .max = float3{std::numeric_limits<float>::lowest(),
                                           std::numeric_limits<float>::lowest(),
                                           std::numeric_limits<float>::lowest()}};

   for (auto& mesh : meshes) {
      mesh.regenerate_bounding_box();

      bounding_box = math::combine(bounding_box, mesh.bounding_box);
   }

   for (auto& cut : terrain_cuts) {
      cut.regenerate_bounding_box();

      terrain_cuts_bounding_box =
         math::combine(terrain_cuts_bounding_box, cut.bounding_box);
   }

   for (auto& coll : collision) {
      coll.regenerate_bounding_box();

      collision_bounding_box =
         math::combine(collision_bounding_box, coll.bounding_box);
   }
}

void flat_model::flatten_segments_to_meshes(const std::vector<geometry_segment>& segments,
                                            const float4x4& node_from_vertex,
                                            const std::vector<material>& scene_materials,
                                            const scene_options& options)
{
   for (const auto& segment : segments) {
      if (segment.triangles.empty()) continue;

      auto& mesh =
         select_mesh_for_segment(segment, scene_materials.at(segment.material_index),
                                 options);

      const auto vertex_offset = mesh.positions.size();

      for (auto pos : segment.positions) {
         mesh.positions.emplace_back(node_from_vertex * pos);
      }

      const float3x3 normal_node_from_vertex{node_from_vertex};

      if (not segment.normals) {
         mesh.normals.append_range(generate_normals(segment.positions, segment.triangles,
                                                    normal_node_from_vertex));
      }
      else {
         for (auto normal : *segment.normals) {
            mesh.normals.emplace_back(normal_node_from_vertex * normal);
         }
      }

      if (not segment.colors) {
         mesh.colors.insert(mesh.colors.end(), segment.positions.size(), 0xffffffffu);
      }
      else {
         mesh.colors.append_range(*segment.colors);
      }

      if (not segment.texcoords) {
         mesh.texcoords.insert(mesh.texcoords.end(), segment.positions.size(),
                               float2{0.0f, 0.0f});
      }
      else {
         mesh.texcoords.append_range(*segment.texcoords);
      }

      for (auto [i0, i1, i2] : segment.triangles) {
         mesh.triangles.push_back({static_cast<uint16>(i0 + vertex_offset),
                                   static_cast<uint16>(i1 + vertex_offset),
                                   static_cast<uint16>(i2 + vertex_offset)});
      }
   }
}

void flat_model::flatten_segments_to_terrain_cut(const std::vector<geometry_segment>& segments,
                                                 const float4x4& local_from_vertex,
                                                 const std::string_view node_name)
{
   std::size_t vertices_count = 0;
   std::size_t triangles_count = 0;

   for (const auto& segment : segments) {
      vertices_count += segment.positions.size();
      triangles_count += segment.triangles.size();
   }

   if (vertices_count > std::numeric_limits<uint16>::max()) {
      throw std::runtime_error{
         fmt::format(".msh file load failure! Node '{}' has too many "
                     "vertices to be a terrain cut.",
                     node_name)};
   }

   auto& cut = terrain_cuts.emplace_back();

   cut.positions.reserve(vertices_count);
   cut.triangles.reserve(triangles_count);

   for (const auto& segment : segments) {
      if (segment.triangles.empty()) continue;

      const std::size_t vertex_offset = cut.positions.size();

      for (auto pos : segment.positions) {
         cut.positions.emplace_back(local_from_vertex * pos);
      }

      for (auto [i0, i1, i2] : segment.triangles) {
         cut.triangles.push_back({static_cast<uint16>(i0 + vertex_offset),
                                  static_cast<uint16>(i1 + vertex_offset),
                                  static_cast<uint16>(i2 + vertex_offset)});
      }
   }

   cut.planes.reserve(flat_model_terrain_cut::max_planes);

   for (const auto& tri : cut.triangles) {
      const float4 tri_plane =
         make_plane(cut.positions[tri[0]], cut.positions[tri[1]],
                    cut.positions[tri[2]]);

      for (const float4& plane : cut.planes) {
         const float eps = 0.001f;

         if (std::abs(plane.x - tri_plane.x) < eps and
             std::abs(plane.y - tri_plane.y) < eps and
             std::abs(plane.z - tri_plane.z) < eps and
             std::abs(plane.w - tri_plane.w) < eps) {
            goto next_tri;
         }
      }

      cut.planes.push_back(tri_plane);

   next_tri:
      continue;
   }

   if (cut.planes.size() > flat_model_terrain_cut::max_planes) {
      throw std::runtime_error{
         fmt::format(".msh file load failure! Terrain cut '{}' has too many "
                     "planes. Has '{}, max '{}'. Split it into multiple meshes "
                     "to resolve this.",
                     node_name, cut.planes.size(), flat_model_terrain_cut::max_planes)};
   }
}

auto flat_model::select_mesh_for_segment(const geometry_segment& segment,
                                         const material& material,
                                         const scene_options& options) -> mesh&
{
   const auto search_for_suitable_mesh = [&](std::vector<mesh>::iterator begin) {
      return std::find_if(begin, meshes.end(), [&](const mesh& mesh) {
         if (options.vertex_lighting) {
            // If vertex colors represent vertex lighting then we don't want to
            // merge meshes that would have different colors_are_lighting values.
            //
            // Which would be true for any mesh made from a geometry_segment **with** colours and false
            // for a mesh that recieved default colours.

            return mesh.material == material and
                   mesh.colors_are_lighting == segment.colors.has_value();
         }

         return mesh.material == material;
      });
   };

   for (auto existing = search_for_suitable_mesh(meshes.begin());
        existing != meshes.end(); existing = search_for_suitable_mesh(++existing)) {
      if ((existing->positions.size() + segment.positions.size()) >
          std::numeric_limits<uint16>::max()) {
         continue;
      }

      return *existing;
   }

   auto& mesh = meshes.emplace_back();

   mesh.material = material;
   mesh.colors_are_lighting = options.vertex_lighting and segment.colors.has_value();

   return mesh;
}

void flat_model::flatten_node_to_collision(const node& node,
                                           const float4x4& node_from_vertex,
                                           const float4x4& local_from_vertex)
{
   auto& flat_collision = collision.emplace_back();

   if (node.collision_primitive and node.name.starts_with("p_"sv)) {
      auto& primitive =
         flat_collision.geometry.emplace<flat_model_collision::primitive>();

      primitive.local_from_primitive = local_from_vertex;
      primitive.shape = node.collision_primitive->shape;
      primitive.radius = node.collision_primitive->radius;
      primitive.height = node.collision_primitive->height;
      primitive.length = node.collision_primitive->length;
   }
   else {
      auto& mesh = flat_collision.geometry.emplace<flat_model_collision::mesh>();

      for (auto& segment : node.segments) {
         const auto base_vertex = mesh.positions.size();

         for (auto pos : segment.positions) {
            mesh.positions.emplace_back(node_from_vertex * pos);
         }

         for (auto [i0, i1, i2] : segment.triangles) {
            mesh.triangles.push_back({static_cast<uint16>(base_vertex + i0),
                                      static_cast<uint16>(base_vertex + i1),
                                      static_cast<uint16>(base_vertex + i2)});
         }
      }
   }
}

void flat_model::generate_tangents_for_meshes()
{
   for (auto& mesh : meshes) {
      auto mesh_with_tangents = generate_tangents({.positions = mesh.positions,
                                                   .normals = mesh.normals,
                                                   .colors = mesh.colors,
                                                   .texcoords = mesh.texcoords,

                                                   .triangles = mesh.triangles});

      mesh.positions = std::move(mesh_with_tangents.positions);
      mesh.normals = std::move(mesh_with_tangents.normals);
      mesh.tangents = std::move(mesh_with_tangents.tangents);
      mesh.bitangents = std::move(mesh_with_tangents.bitangents);
      mesh.colors = std::move(mesh_with_tangents.colors);
      mesh.texcoords = std::move(mesh_with_tangents.texcoords);
      mesh.triangles = std::move(mesh_with_tangents.triangles);
   }
}

void flat_model::apply_ambient_lighting(const scene_options& options) noexcept
{
   if (not options.ambient_lighting) return;

   const float3 ambient_lighting = *options.ambient_lighting;

   for (auto& mesh : meshes) {
      if (not mesh.colors_are_lighting) continue;

      for (uint32& bgra : mesh.colors) {
         float3 color = {((bgra >> 16) & 0xff) / 255.0f,
                         ((bgra >> 8) & 0xff) / 255.0f, (bgra & 0xff) / 255.0f};

         color = clamp(*options.ambient_lighting + color, 0.0f, 1.0f);

         bgra &= 0xff'00'00'00u;
         bgra |= static_cast<uint32>(color.z * 255.0f + 0.5f);
         bgra |= static_cast<uint32>(color.y * 255.0f + 0.5f) << 8u;
         bgra |= static_cast<uint32>(color.x * 255.0f + 0.5f) << 16u;
      }
   }
}

void flat_model::build_ground_points() noexcept
{
   const double ground_point_snapping = 1000.0;
   const double ground = std::round(bounding_box.min.y * ground_point_snapping) /
                         ground_point_snapping;

   for (const mesh& mesh : meshes) {
      for (const float3& position : mesh.positions) {
         const double position_y =
            std::round(position.y * ground_point_snapping) / ground_point_snapping;

         if (position_y == ground) ground_points.push_back(position);
      }
   }
}

void mesh::regenerate_bounding_box() noexcept
{
   bounding_box = {.min = float3{std::numeric_limits<float>::max(),
                                 std::numeric_limits<float>::max(),
                                 std::numeric_limits<float>::max()},
                   .max = float3{std::numeric_limits<float>::lowest(),
                                 std::numeric_limits<float>::lowest(),
                                 std::numeric_limits<float>::lowest()}};

   for (const auto& pos : positions) {
      bounding_box = math::integrate(bounding_box, pos);
   }
}

void flat_model_terrain_cut::regenerate_bounding_box() noexcept
{
   bounding_box = {.min = float3{std::numeric_limits<float>::max(),
                                 std::numeric_limits<float>::max(),
                                 std::numeric_limits<float>::max()},
                   .max = float3{std::numeric_limits<float>::lowest(),
                                 std::numeric_limits<float>::lowest(),
                                 std::numeric_limits<float>::lowest()}};

   for (const auto& pos : positions) {
      bounding_box = math::integrate(bounding_box, pos);
   }
}

void flat_model_collision::regenerate_bounding_box() noexcept
{
   if (std::holds_alternative<mesh>(geometry)) {
      auto& coll_mesh = std::get<mesh>(geometry);

      bounding_box = {.min = float3{std::numeric_limits<float>::max(),
                                    std::numeric_limits<float>::max(),
                                    std::numeric_limits<float>::max()},
                      .max = float3{std::numeric_limits<float>::lowest(),
                                    std::numeric_limits<float>::lowest(),
                                    std::numeric_limits<float>::lowest()}};

      for (const auto& pos : coll_mesh.positions) {
         bounding_box = math::integrate(bounding_box, pos);
      }
   }
   else {
      auto& coll_primitive = std::get<primitive>(geometry);

      switch (coll_primitive.shape) {
      case collision_primitive_shape::sphere:
      default: {
         const float3 centreLS = {coll_primitive.local_from_primitive[3].x,
                                  coll_primitive.local_from_primitive[3].y,
                                  coll_primitive.local_from_primitive[3].z};

         bounding_box = {.min = centreLS - coll_primitive.radius,
                         .max = centreLS + coll_primitive.radius};
      } break;
      case collision_primitive_shape::cylinder: {
         // Inigo Quilez - Cylinder Bounding Box https://iquilezles.org/articles/bboxes3d/

         const float3 topLS = coll_primitive.local_from_primitive *
                              float3{0.0f, coll_primitive.height, 0.0f};
         const float3 bottomLS = coll_primitive.local_from_primitive *
                                 float3{0.0f, -coll_primitive.height, 0.0f};

         const float3 a = bottomLS - topLS;
         const float3 e = coll_primitive.radius * sqrt(1.0f - a * a / dot(a, a));

         bounding_box = {.min = min(topLS, bottomLS) - e,
                         .max = max(topLS, bottomLS) + e};
      } break;
      case collision_primitive_shape::box: {
         const float3 size = {coll_primitive.radius, coll_primitive.height,
                              coll_primitive.length};

         const math::bounding_box bboxVS = {.min = -size, .max = size};

         bounding_box = coll_primitive.local_from_primitive * bboxVS;
      } break;
      }
   }
}

}
