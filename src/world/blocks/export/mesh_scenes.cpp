#include "mesh_scenes.hpp"

#include "container/enum_array.hpp"

#include <memory>

#include <absl/container/flat_hash_map.h>

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we::world {

namespace {

// 40 is the highest I've gone without the game crashing, 32 is a nicer number.
constexpr uint32 max_node_segments = 32;

enum class foley_index {
   stone,
   dirt,
   grass,
   metal,
   snow,
   terrain,
   water,
   wood,

   COUNT,
};

auto to_index(block_foley_group group) -> foley_index
{
   switch (group) {
   default:
   case block_foley_group::stone:
      return foley_index::stone;
   case block_foley_group::dirt:
      return foley_index::dirt;
   case block_foley_group::grass:
      return foley_index::grass;
   case block_foley_group::metal:
      return foley_index::metal;
   case block_foley_group::snow:
      return foley_index::snow;
   case block_foley_group::terrain:
      return foley_index::terrain;
   case block_foley_group::water:
      return foley_index::water;
   case block_foley_group::wood:
      return foley_index::wood;
   }
}

constexpr foley_index foley_index_list[] = {
   foley_index::stone, foley_index::dirt, foley_index::grass,
   foley_index::metal, foley_index::snow, foley_index::terrain,
   foley_index::water, foley_index::wood,
};

constexpr block_foley_group foley_group_list[] = {
   block_foley_group::stone, block_foley_group::dirt,
   block_foley_group::grass, block_foley_group::metal,
   block_foley_group::snow,  block_foley_group::terrain,
   block_foley_group::water, block_foley_group::wood,
};

struct cache_vertex {
   cache_vertex() = default;

   cache_vertex(const block_world_vertex& vertex)
      : positionWS{vertex.positionWS},
        normalWS{vertex.normalWS},
        texcoords{vertex.texcoords}
   {
      // clang-format off
      if (positionWS.x == 0.0f or positionWS.x != positionWS.x) positionWS.x = 0.0f;
      if (positionWS.y == 0.0f or positionWS.y != positionWS.y) positionWS.y = 0.0f;
      if (positionWS.z == 0.0f or positionWS.z != positionWS.z) positionWS.z = 0.0f;
      if (normalWS.x == 0.0f or normalWS.x != normalWS.x) normalWS.x = 0.0f;
      if (normalWS.y == 0.0f or normalWS.y != normalWS.y) normalWS.y = 0.0f;
      if (normalWS.z == 0.0f or normalWS.z != normalWS.z) normalWS.z = 0.0f;
      if (texcoords.x == 0.0f or texcoords.x != texcoords.x) texcoords.x = 0.0f;
      if (texcoords.y == 0.0f or texcoords.y != texcoords.y) texcoords.y = 0.0f;
      // clang-format on
   }

   float3 positionWS;
   float3 normalWS;
   float2 texcoords;

   bool operator==(const cache_vertex& other) const noexcept = default;

   template<typename H>
   friend H AbslHashValue(H h, const cache_vertex& vertex)
   {
      return H::combine(std::move(h), vertex.positionWS.x, vertex.positionWS.y,
                        vertex.positionWS.z, vertex.normalWS.x, vertex.normalWS.y,
                        vertex.normalWS.z, vertex.texcoords.x, vertex.texcoords.y);
   }
};

struct cache_collision_vertex {
   cache_collision_vertex() = default;

   cache_collision_vertex(const float3& vertexWS) : positionWS{vertexWS}
   {
      // clang-format off
      if (positionWS.x == 0.0f or positionWS.x != positionWS.x) positionWS.x = 0.0f;
      if (positionWS.y == 0.0f or positionWS.y != positionWS.y) positionWS.y = 0.0f;
      if (positionWS.z == 0.0f or positionWS.z != positionWS.z) positionWS.z = 0.0f;
      // clang-format on
   }

   float3 positionWS;

   bool operator==(const cache_collision_vertex& other) const noexcept = default;

   template<typename H>
   friend H AbslHashValue(H h, const cache_collision_vertex& vertex)
   {
      return H::combine(std::move(h), vertex.positionWS.x, vertex.positionWS.y,
                        vertex.positionWS.z);
   }
};

auto make_material(const block_material& material) noexcept -> assets::msh::material
{
   using assets::msh::rendertype;

   assets::msh::material msh_material;

   msh_material.textures[0] = material.diffuse_map;
   msh_material.textures[1] = material.normal_map;
   msh_material.textures[2] = material.detail_map;
   msh_material.textures[3] = material.env_map;
   msh_material.data0 = material.detail_tiling[0];
   msh_material.data1 = material.detail_tiling[1];
   msh_material.specular_color = material.specular_color;

   if (not material.env_map.empty()) {
      if (not material.normal_map.empty()) {
         if (material.tile_normal_map) {
            msh_material.rendertype = rendertype::normalmap_tiled_envmapped;
         }
         else {
            msh_material.rendertype = rendertype::normalmap_envmapped;
         }
      }
      else {
         msh_material.rendertype = rendertype::envmapped;
      }
   }
   else if (not material.normal_map.empty()) {
      if (material.tile_normal_map) {
         msh_material.rendertype = rendertype::normalmap_tiled;
      }
      else {
         msh_material.rendertype = rendertype::normalmap;
      }
   }

   if (material.specular_lighting) {
      msh_material.flags = assets::msh::material_flags::specular;
   }

   return msh_material;
}

void process_triangles(
   const block_foley_group foley_group,
   const std::array<std::vector<std::array<block_world_vertex, 3>>, max_block_materials>& material_binned_triangles,
   const std::vector<std::array<float3, 3>>& foley_binned_collision_triangles,
   std::span<const block_material> materials,
   std::vector<block_export_scene>& scenes_out) noexcept
{
   const std::size_t first_scene = scenes_out.size();

   assets::msh::scene scene = {.nodes = {
                                  assets::msh::node{
                                     .name = "root",
                                  },
                                  assets::msh::node{
                                     .name = "geometry",
                                     .parent = "root",
                                     .type = assets::msh::node_type::static_mesh,
                                  },
                               }};

   absl::flat_hash_map<cache_vertex, uint16> vertex_cache;

   for (uint32 material_index = 0; material_index < max_block_materials;
        ++material_index) {
      const std::vector<std::array<block_world_vertex, 3>>& binned_triangles =
         material_binned_triangles[material_index];

      if (binned_triangles.empty()) continue;

      if (scene.nodes[1].segments.size() == max_node_segments) {
         scene.materials.push_back(make_material(materials[material_index]));

         scenes_out.push_back({foley_group, std::move(scene)});

         scene = {.nodes = {
                     assets::msh::node{
                        .name = "root",
                     },
                     assets::msh::node{
                        .name = "geometry",
                        .parent = "root",
                        .type = assets::msh::node_type::static_mesh,
                     },
                  }};
      }

      // Initialize segment we'll append to.
      {
         assets::msh::geometry_segment& segment =
            scene.nodes[1].segments.emplace_back();

         segment.material_index = static_cast<uint32>(scene.materials.size());

         segment.positions.reserve(binned_triangles.size());
         segment.normals = std::vector<float3>{};
         segment.normals->reserve(binned_triangles.size());
         segment.texcoords = std::vector<float2>{};
         segment.texcoords->reserve(binned_triangles.size());
         segment.triangles.reserve(binned_triangles.size());
      }

      vertex_cache.reserve(binned_triangles.size() * 2);

      for (const std::array<block_world_vertex, 3>& triangle : binned_triangles) {
         if (scene.nodes[1].segments.back().positions.size() >
             assets::msh::geometry_segment::max_vertex_count - 3) {
            if (scene.nodes[1].segments.size() == max_node_segments) {
               scene.materials.push_back(make_material(materials[material_index]));

               scenes_out.push_back({foley_group, std::move(scene)});

               scene = {.nodes = {
                           assets::msh::node{
                              .name = "root",
                           },
                           assets::msh::node{
                              .name = "geometry",
                              .parent = "root",
                              .type = assets::msh::node_type::static_mesh,
                           },
                        }};
            }

            assets::msh::geometry_segment& segment =
               scene.nodes[1].segments.emplace_back();

            segment.material_index = static_cast<uint32>(scene.materials.size());

            segment.positions.reserve(binned_triangles.size());
            segment.normals = std::vector<float3>{};
            segment.normals->reserve(binned_triangles.size());
            segment.texcoords = std::vector<float2>{};
            segment.texcoords->reserve(binned_triangles.size());
            segment.triangles.reserve(binned_triangles.size());

            vertex_cache.clear();
         }

         assets::msh::geometry_segment& segment = scene.nodes[1].segments.back();

         std::array<uint16, 3>& indexed_triangle = segment.triangles.emplace_back();

         for (uint32 i = 0; i < indexed_triangle.size(); ++i) {
            auto index_it = vertex_cache.find(triangle[i]);

            if (index_it == vertex_cache.end()) {
               indexed_triangle[i] = static_cast<uint16>(segment.positions.size());

               vertex_cache.emplace(triangle[i], indexed_triangle[i]);

               segment.positions.push_back(triangle[i].positionWS);
               segment.normals->push_back(triangle[i].normalWS);
               segment.texcoords->push_back(triangle[i].texcoords);
            }
            else {
               indexed_triangle[i] = index_it->second;
            }
         }
      }

      scene.materials.push_back(make_material(materials[material_index]));

      vertex_cache.clear();
   }

   if (not scene.nodes[1].segments.empty()) {
      scenes_out.push_back({foley_group, std::move(scene)});
   }

   vertex_cache = {};

   if (foley_binned_collision_triangles.empty()) return;

   const std::size_t last_scene = scenes_out.size();
   std::size_t collision_triangle_index = 0;

   absl::flat_hash_map<cache_collision_vertex, uint16> collision_vertex_cache;
   collision_vertex_cache.reserve(foley_binned_collision_triangles.size() * 2);

   for (std::size_t scene_index = first_scene; scene_index < last_scene; ++scene_index) {
      assets::msh::node node{
         .name = "collision",
         .parent = "root",
         .type = assets::msh::node_type::static_mesh,
         .hidden = true,
      };

      for (; collision_triangle_index < foley_binned_collision_triangles.size();
           ++collision_triangle_index) {
         if (node.segments.empty() or node.segments.back().positions.size() == 0x8000) {
            if (node.segments.size() == 2) break;

            assets::msh::geometry_segment& segment = node.segments.emplace_back();

            segment.positions.reserve(
               std::min((foley_binned_collision_triangles.size() - collision_triangle_index) * 3,
                        std::size_t{0x8000}));
            segment.triangles.reserve(foley_binned_collision_triangles.size() -
                                      collision_triangle_index);

            vertex_cache.clear();
         }

         assets::msh::geometry_segment& segment = node.segments.back();

         const std::array<float3, 3>& triangle =
            foley_binned_collision_triangles[collision_triangle_index];
         std::array<uint16, 3>& indexed_triangle = segment.triangles.emplace_back();

         for (uint32 i = 0; i < indexed_triangle.size(); ++i) {
            auto index_it = collision_vertex_cache.find(triangle[i]);

            if (index_it == collision_vertex_cache.end()) {
               indexed_triangle[i] = static_cast<uint16>(segment.positions.size());

               collision_vertex_cache.emplace(triangle[i], indexed_triangle[i]);

               segment.positions.push_back(triangle[i]);
            }
            else {
               indexed_triangle[i] = index_it->second;
            }
         }
      }

      collision_vertex_cache.clear();

      if (not node.segments.empty()) {
         scenes_out[scene_index].msh_scene.nodes.push_back(std::move(node));
         scenes_out[scene_index].has_collision = true;
      }
   }

   assert(collision_triangle_index == foley_binned_collision_triangles.size());
}

}

auto build_mesh_scenes(std::span<const block_world_mesh> meshes,
                       std::span<const std::vector<uint32>> mesh_clusters,
                       std::span<const block_material> materials) noexcept
   -> std::vector<block_export_scene>
{
   std::vector<block_export_scene> scenes;

   std::unique_ptr foley_binned_triangle_counts =
      std::make_unique<container::enum_array<std::array<uint32, max_block_materials>, foley_index>>();
   std::unique_ptr foley_binned_triangles = std::make_unique<container::enum_array<
      std::array<std::vector<std::array<block_world_vertex, 3>>, max_block_materials>, foley_index>>();

   std::unique_ptr foley_binned_collision_triangle_counts =
      std::make_unique<container::enum_array<uint32, foley_index>>();
   std::unique_ptr foley_binned_collision_triangles =
      std::make_unique<container::enum_array<std::vector<std::array<float3, 3>>, foley_index>>();

   for (const std::vector<uint32>& cluster : mesh_clusters) {
      for (const uint32 mesh_index : cluster) {
         for (const block_world_triangle& triangle : meshes[mesh_index].triangles) {
            foley_binned_triangle_counts->at(to_index(
               materials[triangle.material_index].foley_group))[triangle.material_index] +=
               1;
         }

         for (const block_world_collision_triangle& triangle :
              meshes[mesh_index].collision_triangles) {
            foley_binned_collision_triangle_counts->at(
               to_index(materials[triangle.material_index].foley_group)) += 1;
         }
      }

      for (const foley_index foley_index : foley_index_list) {
         for (uint32 material_index = 0; material_index < max_block_materials;
              ++material_index) {
            foley_binned_triangles->at(foley_index)[material_index].reserve(
               foley_binned_triangle_counts->at(foley_index)[material_index]);
         }

         foley_binned_collision_triangles->at(foley_index)
            .reserve(foley_binned_collision_triangle_counts->at(foley_index));
      }

      for (const uint32 mesh_index : cluster) {
         for (const block_world_triangle& triangle : meshes[mesh_index].triangles) {
            foley_binned_triangles
               ->at(to_index(
                  materials[triangle.material_index].foley_group))[triangle.material_index]
               .push_back(triangle.vertices);
         }

         for (const block_world_collision_triangle& triangle :
              meshes[mesh_index].collision_triangles) {
            foley_binned_collision_triangles
               ->at(to_index(materials[triangle.material_index].foley_group))
               .push_back(triangle.verticesWS);
         }
      }

      for (const block_foley_group foley_group : foley_group_list) {
         const std::array<std::vector<std::array<block_world_vertex, 3>>, max_block_materials>& material_binned_triangles =
            foley_binned_triangles->at(to_index(foley_group));
         const std::vector<std::array<float3, 3>>& binned_collision_triangles =
            foley_binned_collision_triangles->at(to_index(foley_group));

         process_triangles(foley_group, material_binned_triangles,
                           binned_collision_triangles, materials, scenes);
      }

      for (std::array<uint32, max_block_materials>& counts :
           *foley_binned_triangle_counts) {
         for (uint32& count : counts) count = 0;
      }

      for (std::array<std::vector<std::array<block_world_vertex, 3>>, max_block_materials>& material_binned_triangles :
           *foley_binned_triangles) {
         for (std::vector<std::array<block_world_vertex, 3>>& tris :
              material_binned_triangles) {
            tris.clear();
         }
      }

      for (uint32& count : *foley_binned_collision_triangle_counts) count = 0;

      for (std::vector<std::array<float3, 3>>& tris : *foley_binned_collision_triangles) {
         tris.clear();
      }
   }

   return scenes;
}

}
