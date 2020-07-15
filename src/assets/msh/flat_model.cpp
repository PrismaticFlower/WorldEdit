
#include "flat_model.hpp"
#include "utility/string_ops.hpp"

#include <algorithm>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

#include <range/v3/algorithm.hpp>

using namespace std::literals;

namespace sk::assets::msh {

namespace {

auto build_node_to_object_transforms(const scene& scene) -> std::vector<float4x4>
{
   std::vector<float4x4> transforms;

   transforms.reserve(scene.nodes.size());

   for (const auto& node : scene.nodes) {
      auto& transform = transforms.emplace_back(node.transform);

      const auto apply_parent_transform =
         [](auto apply_parent_transform, float4x4& transform,
            const std::string_view parent, const std::vector<msh::node>& nodes) -> void {
         if (auto parent_it = std::find_if(nodes.cbegin(), nodes.cend(),
                                           [&](const msh::node& node) {
                                              return node.name == parent;
                                           });
             parent_it != nodes.cend()) {
            transform = float4x4{parent_it->transform} * transform;

            if (parent_it->parent) {
               apply_parent_transform(apply_parent_transform, transform,
                                      *parent_it->parent, nodes);
            }
         }
         else {
            throw std::runtime_error{
               fmt::format("Unable to find parent node '{}' in .msh scene!"sv, parent)};
         }
      };

      if (node.parent) {
         apply_parent_transform(apply_parent_transform, transform, *node.parent,
                                scene.nodes);
      }
   }

   return transforms;
}

bool is_mesh_node(const node& node) noexcept
{
   switch (node.type) {
   case node_type::null:
   case node_type::cloth:
   case node_type::bone:
   case node_type::shadow_volume:
      return false;
   }

   if (node.hidden) return false;
   if (node.name.starts_with("sv_"sv)) return false;
   if (node.name.starts_with("shadowvolume"sv)) return false;
   if (node.name.starts_with("p_"sv)) return false;
   if (boost::istarts_with(node.name, "collision"sv)) return false;
   if (boost::istarts_with(node.name, "terraincutter"sv)) return false;

   // TODO: LOD Support
   if (boost::iends_with(node.name, "lod2"sv)) return false;
   if (boost::iends_with(node.name, "lod3"sv)) return false;
   if (boost::iends_with(node.name, "lowres"sv)) return false;
   if (boost::iends_with(node.name, "lowrez"sv)) return false;

   return true;
}

bool is_collision_node(const node& node) noexcept
{
   if (node.name.starts_with("p_"sv)) return true;
   if (boost::istarts_with(node.name, "collision"sv)) return true;

   return false;
}

auto make_flat_scene_node(const node& base, const std::vector<node>& nodes) -> flat_model_node
{
   flat_model_node flat_node{.name = base.name,
                             .transform = base.transform,
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
                      std::span<const std::array<uint16, 3>> triangles)
   -> std::vector<float3>
{
   std::vector<float3> normals{positions.size(), float3{0.0f}};

   for (const auto [i0, i1, i2] : triangles) {
      const float3 normal =
         glm::cross(positions[i0] - positions[i1], positions[i2] - positions[i1]);

      normals[i0] += normal;
      normals[i1] += normal;
      normals[i2] += normal;
   }

   for (auto& normal : normals) normal = glm::normalize(normal);

   return normals;
}

}

flat_model::flat_model(const scene& scene) noexcept
{
   const std::vector<float4x4> node_to_object_transforms =
      build_node_to_object_transforms(scene);

   for (std::size_t i = 0; i < scene.nodes.size(); ++i) {
      const auto& node = scene.nodes[i];
      const auto& node_to_object = node_to_object_transforms[i];

      if (is_mesh_node(node)) {
         flatten_segments_to_meshes(node.segments, node_to_object, scene.materials);
      }
      else if (is_collision_node(node)) {
         flatten_node_to_collision(node, node_to_object);
      }
   }

   for (const auto& node : scene.nodes) {
      if (node.parent) continue;

      node_hierarchy.emplace_back(make_flat_scene_node(node, scene.nodes));
   }

   regenerate_bounding_boxes();
}

void flat_model::regenerate_bounding_boxes() noexcept
{
   bounding_box = {.min = float3{std::numeric_limits<float>::max()},
                   .max = float3{std::numeric_limits<float>::lowest()}};

   for (auto& mesh : meshes) {
      mesh.regenerate_bounding_box();

      bounding_box = math::combine(bounding_box, mesh.bounding_box);
   }

   for (auto& coll : collision) {
      coll.regenerate_bounding_box();

      bounding_box = math::combine(bounding_box, coll.bounding_box);
   }
}

void flat_model::flatten_segments_to_meshes(const std::vector<geometry_segment>& segments,
                                            const float4x4& node_to_object,
                                            const std::vector<material>& scene_materials)
{
   for (const auto& segment : segments) {
      auto& mesh =
         select_mesh_for_segment(segment, scene_materials.at(segment.material_index));

      const auto vertex_offset = mesh.positions.size();

      for (auto pos : segment.positions) {
         mesh.positions.emplace_back(node_to_object * float4{pos, 1.0f});
      }

      const float3x3 normal_node_to_object = node_to_object;

      if (not segment.normals) {
         mesh.normals = generate_normals(mesh.positions, mesh.triangles);
      }
      else {
         for (auto normal : *segment.normals) {
            mesh.normals.emplace_back(normal_node_to_object * float4{normal, 1.0f});
         }
      }

      if (not segment.colors) {
         mesh.colors.insert(mesh.colors.end(), segment.positions.size(),
                            float4{1.0f, 1.0f, 1.0f, 1.0f});
      }
      else {
         mesh.colors.insert(mesh.colors.end(), segment.colors->begin(),
                            segment.colors->end());
      }

      if (not segment.texcoords) {
         mesh.texcoords.insert(mesh.texcoords.end(), segment.positions.size(),
                               float2{0.0f, 0.0f});
      }
      else {
         mesh.texcoords.insert(mesh.texcoords.end(), segment.texcoords->begin(),
                               segment.texcoords->end());
      }

      for (auto [i0, i1, i2] : segment.triangles) {
         mesh.triangles.push_back({static_cast<uint16>(i0 + vertex_offset),
                                   static_cast<uint16>(i1 + vertex_offset),
                                   static_cast<uint16>(i2 + vertex_offset)});
      }
   }
}

auto flat_model::select_mesh_for_segment(const geometry_segment& segment,
                                         const material& material) -> mesh&
{
   const auto search_for_suitable_mesh = [&](std::vector<mesh>::iterator begin) {
      return std::find_if(begin, meshes.end(), [&](const mesh& mesh) {
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

   return mesh;
}

void flat_model::flatten_node_to_collision(const node& node, const float4x4& node_to_object)
{
   auto& flat_collision = collision.emplace_back();

   if (node.collision_primitive and node.name.starts_with("p_"sv)) {
      auto& primitive =
         flat_collision.geometry.emplace<flat_model_collision::primitive>();

      primitive.transform = {.translation = node_to_object[3],
                             .rotation = node_to_object};
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
            mesh.positions.emplace_back(node_to_object * float4{pos, 1.0f});
         }

         for (auto [i0, i1, i2] : segment.triangles) {
            mesh.triangles.push_back({static_cast<uint16>(base_vertex + i0),
                                      static_cast<uint16>(base_vertex + i1),
                                      static_cast<uint16>(base_vertex + i2)});
         }
      }
   }
}

void mesh::regenerate_bounding_box() noexcept
{
   bounding_box = {.min = float3{std::numeric_limits<float>::max()},
                   .max = float3{std::numeric_limits<float>::lowest()}};

   for (const auto& pos : positions) {
      bounding_box = math::integrate(bounding_box, pos);
   }
}

void flat_model_collision::regenerate_bounding_box() noexcept
{
   if (boost::variant2::holds_alternative<mesh>(geometry)) {
      auto& coll_mesh = boost::variant2::get<mesh>(geometry);

      bounding_box = {.min = float3{std::numeric_limits<float>::max()},
                      .max = float3{std::numeric_limits<float>::lowest()}};

      for (const auto& pos : coll_mesh.positions) {
         bounding_box = math::integrate(bounding_box, pos);
      }
   }
   else {
      auto& coll_primitive = boost::variant2::get<primitive>(geometry);

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
