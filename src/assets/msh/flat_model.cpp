
#include "flat_model.hpp"
#include "generate_tangents.hpp"
#include "math/matrix_funcs.hpp"
#include "math/plane_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"
#include "utility/enum_bitflags.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include <algorithm>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>

#include <fmt/core.h>

using namespace std::literals;

namespace we::assets::msh {

namespace {

auto build_node_to_object_transforms(const scene& scene) -> std::vector<float4x4>
{
   std::vector<float4x4> transforms;

   transforms.reserve(scene.nodes.size());

   const float scale = scene.options.scale;

   for (const auto& node : scene.nodes) {
      auto& transform = transforms.emplace_back(
         msh::transform{node.transform.translation * scale, node.transform.rotation});

      const auto apply_parent_transform =
         [](auto apply_parent_transform, float4x4& transform,
            const std::string_view parent, const std::vector<msh::node>& nodes,
            const float scale) -> void {
         if (auto parent_it = std::find_if(nodes.cbegin(), nodes.cend(),
                                           [&](const msh::node& node) {
                                              return node.name == parent;
                                           });
             parent_it != nodes.cend()) {
            const float4x4 parent_matrix =
               parent_it->parent
                  ? float4x4{msh::transform{parent_it->transform.translation * scale,
                                            parent_it->transform.rotation}}
                  : float4x4{};

            transform = parent_matrix * transform;

            if (parent_it->parent) {
               apply_parent_transform(apply_parent_transform, transform,
                                      *parent_it->parent, nodes, scale);
            }
         }
         else {
            throw std::runtime_error{
               fmt::format("Unable to find parent node '{}' in .msh scene!", parent)};
         }
      };

      if (node.parent) {
         apply_parent_transform(apply_parent_transform, transform, *node.parent,
                                scene.nodes, scale);
      }
   }

   return transforms;
}

bool is_mesh_node(const node& node) noexcept
{
   switch (node.type) {
   case node_type::cloth:
   case node_type::bone:
   case node_type::shadow_volume:
      return false;
   }

   if (node.hidden) return false;
   if (node.name.starts_with("sv_"sv)) return false;
   if (node.name.starts_with("shadowvolume"sv)) return false;
   if (node.name.starts_with("p_"sv)) return false;
   if (node.name.starts_with("hp_"sv)) return false;
   if (string::istarts_with(node.name, "collision"sv)) return false;
   if (string::istarts_with(node.name, "terraincutter"sv)) return false;

   // TODO: LOD Support
   if (string::iends_with(node.name, "lod2"sv)) return false;
   if (string::iends_with(node.name, "lod3"sv)) return false;
   if (string::iends_with(node.name, "lowres"sv)) return false;
   if (string::iends_with(node.name, "lowrez"sv)) return false;

   return true;
}

bool is_terrain_cut_node(const node& node) noexcept
{
   return string::istarts_with(node.name, "terraincutter"sv);
}

bool is_collision_node(const node& node) noexcept
{
   if (node.name.starts_with("p_"sv)) return true;
   if (string::istarts_with(node.name, "collision"sv)) return true;

   return false;
}

auto make_flat_scene_node(const node& base, const std::vector<node>& nodes,
                          const float scale) -> flat_model_node
{
   flat_model_node flat_node{.name = base.name,
                             .transform = {base.transform.translation * scale,
                                           base.transform.rotation},
                             .type = base.type,
                             .hidden = base.hidden};

   flat_node.children.reserve(
      std::count_if(nodes.cbegin(), nodes.cend(),
                    [&](const node& node) { return node.parent == base.name; }));

   for (const auto& node : nodes) {
      if (node.parent != base.name) continue;

      flat_node.children.emplace_back(make_flat_scene_node(node, nodes, scale));
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
         cross(positions[i0] - positions[i1], positions[i2] - positions[i1]);

      normals[i0] += normal;
      normals[i1] += normal;
      normals[i2] += normal;
   }

   for (auto& normal : normals) {
      normal = normalize(normal_node_to_object * normal);
   }

   return normals;
}

void patch_materials_with_options(std::vector<mesh>& meshes, const options& opts)
{
   for (auto& mesh : meshes) {
      for (auto& normal_map : opts.normal_maps) {
         if (string::iequals(mesh.material.textures[0], normal_map)) {
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

flat_model::flat_model(const scene& scene)
{
   const std::vector<float4x4> node_to_object_transforms =
      build_node_to_object_transforms(scene);

   for (std::size_t i = 0; i < scene.nodes.size(); ++i) {
      const auto& node = scene.nodes[i];
      const auto& node_to_object = node_to_object_transforms[i];

      if (is_mesh_node(node)) {
         flatten_segments_to_meshes(node.segments, node_to_object,
                                    scene.materials, scene.options);
      }
      else if (is_terrain_cut_node(node)) {
         flatten_segments_to_terrain_cut(node.segments, node_to_object,
                                         node.name, scene.options);
      }
      else if (is_collision_node(node)) {
         flatten_node_to_collision(node, node_to_object, scene.options);
      }
   }

   for (const auto& node : scene.nodes) {
      if (node.parent) continue;

      node_hierarchy.emplace_back(
         make_flat_scene_node(node, scene.nodes, scene.options.scale));
   }

   generate_tangents_for_meshes();
   patch_materials_with_options(meshes, scene.options);
   apply_ambient_lighting(scene.options);
   regenerate_bounding_boxes();

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
                                            const float4x4& node_to_object,
                                            const std::vector<material>& scene_materials,
                                            const options& options)
{
   for (const auto& segment : segments) {
      if (segment.triangles.empty()) continue;

      auto& mesh =
         select_mesh_for_segment(segment, scene_materials.at(segment.material_index),
                                 options);

      const auto vertex_offset = mesh.positions.size();

      for (auto pos : segment.positions) {
         mesh.positions.emplace_back(node_to_object * pos * options.scale);
      }

      const float3x3 normal_node_to_object{node_to_object};

      if (not segment.normals) {
         mesh.normals.append_range(generate_normals(segment.positions, segment.triangles,
                                                    normal_node_to_object));
      }
      else {
         for (auto normal : *segment.normals) {
            mesh.normals.emplace_back(normal_node_to_object * normal);
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

void flat_model::flatten_segments_to_terrain_cut(
   const std::vector<geometry_segment>& segments, const float4x4& node_to_object,
   const std::string_view node_name, const options& options)
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
         cut.positions.emplace_back(node_to_object * pos * options.scale);
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
                                         const options& options) -> mesh&
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
                                           const float4x4& node_to_object,
                                           const options& options)
{
   auto& flat_collision = collision.emplace_back();

   if (node.collision_primitive and node.name.starts_with("p_"sv)) {
      auto& primitive =
         flat_collision.geometry.emplace<flat_model_collision::primitive>();

      primitive.transform = {.translation =
                                float3{node_to_object[3].x, node_to_object[3].y,
                                       node_to_object[3].z},
                             .rotation = make_quat_from_matrix(node_to_object)};
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
            mesh.positions.emplace_back(node_to_object * pos * options.scale);
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

void flat_model::apply_ambient_lighting(const options& options) noexcept
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
      default:
         bounding_box = {.min = float3{coll_primitive.transform.translation -
                                       coll_primitive.radius},
                         .max = float3{coll_primitive.transform.translation +
                                       coll_primitive.radius}};
         break;
      case collision_primitive_shape::cylinder:
         bounding_box = {.min = float3{-coll_primitive.radius, -coll_primitive.height,
                                       -coll_primitive.radius},
                         .max = float3{coll_primitive.radius, coll_primitive.height,
                                       coll_primitive.radius}};

         bounding_box = coll_primitive.transform.rotation * bounding_box +
                        coll_primitive.transform.translation;

         break;
      case collision_primitive_shape::box:
         bounding_box = {.min = float3{-coll_primitive.radius, -coll_primitive.height,
                                       -coll_primitive.length},
                         .max = float3{coll_primitive.radius, coll_primitive.height,
                                       coll_primitive.length}};

         bounding_box = coll_primitive.transform.rotation * bounding_box +
                        coll_primitive.transform.translation;
         break;
      }
   }
}

}
