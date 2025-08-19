#include "mesh_gather.hpp"
#include "mesh.hpp"
#include "mesh_clusters.hpp"
#include "mesh_cull.hpp"

#include "../custom_mesh.hpp"
#include "../mesh_geometry.hpp"

#include "../utility/find.hpp"

#include "assets/msh/scene_io.hpp"

#include "math/bounding_box.hpp"
#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include "io/output_file.hpp"

#include <vector>

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we::world {

namespace {

constexpr int32 min_cluster_triangles = 128;
constexpr int32 max_cluster_subdivision = 6;

auto evaluate_texcorrds(const float3& positionWS, const float3& normalWS,
                        const float2& vertex_texcoords,
                        const quaternion& local_from_world,
                        block_texture_mode mode, block_texture_rotation rotation,
                        const std::array<int8, 2>& scale,
                        const std::array<uint16, 2>& offset) noexcept -> float2
{
   float2 texcoords;

   switch (mode) {
   case block_texture_mode::world_space_auto: {
      const float3 normal_absWS = abs(normalWS);

      if (normal_absWS.x < normal_absWS.y and normal_absWS.z < normal_absWS.y) {
         texcoords = {positionWS.x, positionWS.z};
      }
      else if (normal_absWS.x < normal_absWS.z) {
         texcoords = {positionWS.x, positionWS.y};
      }
      else {
         texcoords = {positionWS.z, positionWS.y};
      }
   } break;
   case block_texture_mode::world_space_zy:
      texcoords = {positionWS.z, positionWS.y};
      break;
   case block_texture_mode::world_space_xz:
      texcoords = {positionWS.x, positionWS.z};
      break;
   case block_texture_mode::world_space_xy:
      texcoords = {positionWS.x, positionWS.y};
      break;
   case block_texture_mode::local_space_auto: {
      const float3 positionLS = local_from_world * positionWS;
      const float3 normal_absLS = abs(local_from_world * normalWS);

      if (normal_absLS.x < normal_absLS.y && normal_absLS.z < normal_absLS.y) {
         texcoords = {positionLS.x, positionLS.z};
      }
      else if (normal_absLS.x < normal_absLS.z) {
         texcoords = {positionLS.x, positionLS.y};
      }
      else {
         texcoords = {positionLS.z, positionLS.y};
      }
   } break;
   case block_texture_mode::local_space_zy: {
      const float3 positionLS = local_from_world * positionWS;

      texcoords = {positionLS.z, positionLS.y};
      break;
   }
   case block_texture_mode::local_space_xz: {
      const float3 positionLS = local_from_world * positionWS;

      texcoords = {positionLS.x, positionLS.z};
   } break;
   case block_texture_mode::local_space_xy: {
      const float3 positionLS = local_from_world * positionWS;

      texcoords = {positionLS.x, positionLS.y};
   } break;
   case block_texture_mode::unwrapped:
      texcoords = vertex_texcoords;
      break;
   };

   texcoords *= float2{exp2f(scale[0]), exp2f(scale[1])};

   switch (rotation) {
   case block_texture_rotation::d0: {
   } break;
   case block_texture_rotation::d90: {
      texcoords = {-texcoords.y, texcoords.x};
   } break;
   case block_texture_rotation::d180: {
      texcoords = -texcoords;
   } break;
   case block_texture_rotation::d270: {
      texcoords = {texcoords.y, -texcoords.x};
   } break;
   }

   texcoords += float2{static_cast<float>(offset[0]), static_cast<float>(offset[1])} *
                (1.0f / 8192.0f);

   return texcoords;
}

template<typename T>
void process_block(const T& block, const id<T> block_id,
                   std::span<const block_vertex> block_vertices,
                   std::span<const std::array<uint16, 3>> block_triangles,
                   std::span<const std::array<uint16, 4>> block_occluders,
                   std::vector<block_world_mesh>& out_mesh_list,
                   std::vector<block_world_occluder>& out_occluder_list) noexcept
{
   block_world_mesh& out_mesh = out_mesh_list.emplace_back();
   out_mesh.triangles.reserve(block_triangles.size());
   out_mesh.collision_triangles.reserve(block_triangles.size());

   const float4x4 scale = {
      {block.size.x, 0.0f, 0.0f, 0.0f},
      {0.0f, block.size.y, 0.0f, 0.0f},
      {0.0f, 0.0f, block.size.z, 0.0f},
      {0.0f, 0.0f, 0.0f, 1.0f},
   };
   const float4x4 rotation = to_matrix(block.rotation);

   float4x4 world_from_local = rotation * scale;
   float3x3 world_from_local_adjugate = adjugate(world_from_local);
   world_from_local[3] = {block.position, 1.0f};

   const float3 local_from_world_xyz = {-block.rotation.x, -block.rotation.y,
                                        -block.rotation.z};
   const float local_from_world_w_sign = -block.rotation.w < 0.0f ? -1.0f : 1.0f;

   const quaternion local_from_world =
      {local_from_world_w_sign *
          sqrt(1.0f - std::max(std::min(dot(local_from_world_xyz, local_from_world_xyz), 1.0f),
                               0.0f)),
       local_from_world_xyz.x, local_from_world_xyz.y, local_from_world_xyz.z};

   for (const std::array<uint16, 3>& tri : block_triangles) {
      block_world_triangle world_triangle = {.block_id = block_id};

      for (std::size_t i = 0; i < tri.size(); ++i) {
         const block_vertex& vertex = block_vertices[tri[i]];

         const float3 positionWS = world_from_local * vertex.position;
         const float3 normalWS = normalize(world_from_local_adjugate * vertex.normal);
         const float2 texcoords =
            evaluate_texcorrds(positionWS, normalWS, vertex.texcoords, local_from_world,
                               block.surface_texture_mode[vertex.surface_index],
                               block.surface_texture_rotation[vertex.surface_index],
                               block.surface_texture_scale[vertex.surface_index],
                               block.surface_texture_offset[vertex.surface_index]);
         const uint8 material_index = block.surface_materials[vertex.surface_index];

         world_triangle.vertices[i] = {.positionWS = positionWS,
                                       .normalWS = normalWS,
                                       .texcoords = texcoords};
         world_triangle.material_index = material_index;
      }

      out_mesh.triangles.push_back(world_triangle);
      out_mesh.collision_triangles.push_back(
         {.block_id = world_triangle.block_id,
          .verticesWS{world_triangle.vertices[0].positionWS,
                      world_triangle.vertices[1].positionWS,
                      world_triangle.vertices[2].positionWS},
          .material_index = world_triangle.material_index});
   }

   for (const std::array<uint16, 4>& quad : block_occluders) {
      block_world_occluder occluder = {.block_id = block_id};

      for (std::size_t i = 0; i < quad.size(); ++i) {
         const block_vertex& vertex = block_vertices[quad[i]];

         occluder.verticesWS[i] = world_from_local * vertex.position;
      }

      const float3 edge0WS = occluder.verticesWS[1] - occluder.verticesWS[0];
      const float3 edge1WS = occluder.verticesWS[2] - occluder.verticesWS[0];
      const float3 e0_cross_e1 = cross(edge0WS, edge1WS);

      const float e0_cross_e1_length = length(e0_cross_e1);
      const float3 normalWS = e0_cross_e1 / e0_cross_e1_length;

      occluder.area = e0_cross_e1_length;
      occluder.planeWS = {normalWS, -dot(normalWS, occluder.verticesWS[0])};

      out_occluder_list.push_back(occluder);
   }
}

void process_block(const block_description_quad& block, const block_quad_id block_id,
                   std::vector<block_world_mesh>& out_mesh_list) noexcept
{
   block_world_mesh& out_mesh = out_mesh_list.emplace_back();
   out_mesh.triangles.reserve(2);
   out_mesh.collision_triangles.reserve(2);

   for (const std::array<uint16, 3>& tri : block.quad_split == block_quad_split::regular
                                              ? block_quad_triangles
                                              : block_quad_alternate_triangles) {

      const float3 normalWS =
         normalize(cross(block.vertices[tri[1]] - block.vertices[tri[0]],
                         block.vertices[tri[2]] - block.vertices[tri[0]]));

      block_world_triangle world_triangle = {.block_id = block_id};

      for (std::size_t i = 0; i < tri.size(); ++i) {
         const float3 positionWS = block.vertices[tri[i]];
         const float2 texcoords =
            evaluate_texcorrds(positionWS, normalWS,
                               block_quad_vertex_texcoords[tri[i]],
                               quaternion{}, block.surface_texture_mode[0],
                               block.surface_texture_rotation[0],
                               block.surface_texture_scale[0],
                               block.surface_texture_offset[0]);
         const uint8 material_index = block.surface_materials[0];

         world_triangle.vertices[i] = {.positionWS = positionWS,
                                       .normalWS = normalWS,
                                       .texcoords = texcoords};
         world_triangle.material_index = material_index;
      }

      out_mesh.triangles.push_back(world_triangle);
      out_mesh.collision_triangles.push_back(
         {.block_id = world_triangle.block_id,
          .verticesWS{world_triangle.vertices[0].positionWS,
                      world_triangle.vertices[1].positionWS,
                      world_triangle.vertices[2].positionWS},
          .material_index = world_triangle.material_index});
   }
}

void process_block(const block_description_custom& block,
                   const block_custom_id block_id, const block_custom_mesh& mesh,
                   std::vector<block_world_mesh>& out_mesh_list,
                   std::vector<block_world_occluder>& out_occluder_list) noexcept
{
   block_world_mesh& out_mesh = out_mesh_list.emplace_back();
   out_mesh.triangles.reserve(mesh.triangles.size());
   out_mesh.collision_triangles.reserve(mesh.collision_occluders.size());

   float4x4 world_from_local = to_matrix(block.rotation);
   float3x3 world_from_local_adjugate = adjugate(world_from_local);
   world_from_local[3] = {block.position, 1.0f};

   const float3 local_from_world_xyz = {-block.rotation.x, -block.rotation.y,
                                        -block.rotation.z};
   const float local_from_world_w_sign = -block.rotation.w < 0.0f ? -1.0f : 1.0f;

   const quaternion local_from_world =
      {local_from_world_w_sign *
          sqrt(1.0f - std::max(std::min(dot(local_from_world_xyz, local_from_world_xyz), 1.0f),
                               0.0f)),
       local_from_world_xyz.x, local_from_world_xyz.y, local_from_world_xyz.z};

   for (const std::array<uint16, 3>& tri : mesh.triangles) {
      block_world_triangle world_triangle = {.block_id = block_id};

      for (std::size_t i = 0; i < tri.size(); ++i) {
         const block_vertex& vertex = mesh.vertices[tri[i]];

         const float3 positionWS = world_from_local * vertex.position;
         const float3 normalWS = normalize(world_from_local_adjugate * vertex.normal);
         const float2 texcoords =
            evaluate_texcorrds(positionWS, normalWS, vertex.texcoords, local_from_world,
                               block.surface_texture_mode[vertex.surface_index],
                               block.surface_texture_rotation[vertex.surface_index],
                               block.surface_texture_scale[vertex.surface_index],
                               block.surface_texture_offset[vertex.surface_index]);
         const uint8 material_index = block.surface_materials[vertex.surface_index];

         world_triangle.vertices[i] = {.positionWS = positionWS,
                                       .normalWS = normalWS,
                                       .texcoords = texcoords};
         world_triangle.material_index = material_index;
      }

      out_mesh.triangles.push_back(world_triangle);
   }

   for (const std::array<uint16, 4>& quad : mesh.occluders) {
      block_world_occluder occluder = {.block_id = block_id};

      for (std::size_t i = 0; i < quad.size(); ++i) {
         const block_vertex& vertex = mesh.vertices[quad[i]];

         occluder.verticesWS[i] = world_from_local * vertex.position;
      }

      const float3 edge0WS = occluder.verticesWS[1] - occluder.verticesWS[0];
      const float3 edge1WS = occluder.verticesWS[2] - occluder.verticesWS[0];
      const float3 e0_cross_e1 = cross(edge0WS, edge1WS);

      const float e0_cross_e1_length = length(e0_cross_e1);
      const float3 normalWS = e0_cross_e1 / e0_cross_e1_length;

      occluder.area = e0_cross_e1_length;
      occluder.planeWS = {normalWS, -dot(normalWS, occluder.verticesWS[0])};

      out_occluder_list.push_back(occluder);
   }

   for (const std::array<uint16, 3>& tri : mesh.collision_triangles) {
      block_world_collision_triangle world_triangle;

      for (std::size_t i = 0; i < tri.size(); ++i) {
         const block_collision_vertex& vertex = mesh.collision_vertices[tri[i]];

         world_triangle.verticesWS[i] = world_from_local * vertex.position;
         world_triangle.material_index = block.surface_materials[vertex.surface_index];
      }

      out_mesh.collision_triangles.push_back(world_triangle);
   }
}

template<typename T>
void process_blocks(const T& blocks, std::span<const block_vertex> block_vertices,
                    std::span<const std::array<uint16, 3>> block_triangles,
                    std::span<const std::array<uint16, 4>> block_occluders,
                    std::vector<block_world_mesh>& out_mesh_list,
                    std::vector<block_world_occluder>& out_occluder_list) noexcept
{
   for (uint32 block_index = 0; block_index < blocks.size(); ++block_index) {
      process_block(blocks.description[block_index], blocks.ids[block_index],
                    block_vertices, block_triangles, block_occluders,
                    out_mesh_list, out_occluder_list);
   }
}

void process_blocks(const blocks_quads& blocks,
                    std::vector<block_world_mesh>& out_mesh_list) noexcept
{
   for (uint32 block_index = 0; block_index < blocks.size(); ++block_index) {
      process_block(blocks.description[block_index], blocks.ids[block_index],
                    out_mesh_list);
   }
}

void process_blocks(const blocks_custom& blocks,
                    const blocks_custom_mesh_library& meshes,
                    std::vector<block_world_mesh>& out_mesh_list,
                    std::vector<block_world_occluder>& out_occluder_list) noexcept
{
   for (uint32 block_index = 0; block_index < blocks.size(); ++block_index) {
      process_block(blocks.description[block_index], blocks.ids[block_index],
                    meshes[blocks.mesh[block_index]], out_mesh_list,
                    out_occluder_list);
   }
}

void fill_bounding_boxes(std::span<block_world_mesh> mesh_list) noexcept
{
   for (block_world_mesh& mesh : mesh_list) {
      mesh.bboxWS = {
         .min = {FLT_MAX, FLT_MAX, FLT_MAX},
         .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX},
      };

      for (const block_world_triangle& tri : mesh.triangles) {
         mesh.bboxWS.min =
            min(min(tri.vertices[0].positionWS, tri.vertices[1].positionWS),
                tri.vertices[2].positionWS);
         mesh.bboxWS.max =
            max(max(tri.vertices[0].positionWS, tri.vertices[1].positionWS),
                tri.vertices[2].positionWS);
      }
   }
}

}

auto gather_export_meshes(const blocks& blocks) -> std::vector<block_export_scene>
{
   std::size_t mesh_count = 0;

   mesh_count += blocks.boxes.size();
   mesh_count += blocks.ramps.size();
   mesh_count += blocks.quads.size();
   mesh_count += blocks.custom.size();
   mesh_count += blocks.hemispheres.size();
   mesh_count += blocks.pyramids.size();

   std::size_t occluder_count = 0;

   occluder_count += blocks.boxes.size() * block_cube_occluders.size();
   occluder_count += blocks.ramps.size() * block_ramp_occluders.size();
   occluder_count += blocks.hemispheres.size() * block_hemisphere_occluders.size();
   occluder_count += blocks.pyramids.size() * block_pyramid_occluders.size();

   for (uint32 block_index = 0; block_index < blocks.custom.size(); ++block_index) {
      const block_custom_mesh& mesh =
         blocks.custom_meshes[blocks.custom.mesh[block_index]];

      occluder_count += mesh.occluders.size();
   }

   std::vector<block_world_mesh> mesh_list;
   std::vector<block_world_occluder> occluder_list;

   mesh_list.reserve(mesh_count);
   occluder_list.reserve(occluder_count);

   process_blocks(blocks.boxes, block_cube_vertices, block_cube_triangles,
                  block_cube_occluders, mesh_list, occluder_list);
   process_blocks(blocks.ramps, block_ramp_vertices, block_ramp_triangles,
                  block_ramp_occluders, mesh_list, occluder_list);
   process_blocks(blocks.quads, mesh_list);
   process_blocks(blocks.custom, blocks.custom_meshes, mesh_list, occluder_list);
   process_blocks(blocks.hemispheres, block_hemisphere_vertices,
                  block_hemisphere_triangles, block_hemisphere_occluders,
                  mesh_list, occluder_list);
   process_blocks(blocks.pyramids, block_pyramid_vertices, block_pyramid_triangles,
                  block_pyramid_occluders, mesh_list, occluder_list);

   fill_bounding_boxes(mesh_list);

   mesh_list = cull_hidden_triangles(mesh_list, occluder_list);

   const std::vector<std::vector<uint32>> mesh_clusters =
      build_mesh_clusters(mesh_list, min_cluster_triangles, max_cluster_subdivision);

   return build_mesh_scenes(mesh_list, mesh_clusters, blocks.materials);
}

auto gather_export_meshes(const blocks& blocks, const selection& selection)
   -> std::vector<block_export_scene>
{
   std::size_t mesh_count = 0;
   std::size_t occluder_count = 0;

   for (const selected_entity& selected : selection) {
      if (not selected.is<block_id>()) continue;

      const block_id id = selected.get<block_id>();
      const std::optional<uint32> block_index = find_block(blocks, id);

      if (not block_index) continue;

      mesh_count += 1;

      switch (id.type()) {
      case block_type::box: {
         occluder_count += block_cube_occluders.size();
      } break;
      case block_type::ramp: {
         occluder_count += block_ramp_occluders.size();
      } break;
      case block_type::quad: {
         const block_custom_mesh& mesh =
            blocks.custom_meshes[blocks.custom.mesh[*block_index]];

         occluder_count += mesh.occluders.size();
      } break;
      case block_type::custom: {
         occluder_count += block_ramp_occluders.size();
      } break;
      case block_type::hemisphere: {
         occluder_count += block_hemisphere_occluders.size();
      } break;
      case block_type::pyramid: {
         occluder_count += block_pyramid_occluders.size();
      } break;
      case block_type::terrain_cut_box: {
      } break;
      }
   }

   std::vector<block_world_mesh> mesh_list;
   std::vector<block_world_occluder> occluder_list;

   mesh_list.reserve(mesh_count);
   occluder_list.reserve(occluder_count);

   for (const selected_entity& selected : selection) {
      if (not selected.is<block_id>()) continue;

      const block_id id = selected.get<block_id>();
      const std::optional<uint32> block_index = find_block(blocks, id);

      if (not block_index) continue;

      switch (id.type()) {
      case block_type::box: {
         process_block(blocks.boxes.description[*block_index],
                       blocks.boxes.ids[*block_index], block_cube_vertices,
                       block_cube_triangles, block_cube_occluders, mesh_list,
                       occluder_list);
      } break;
      case block_type::ramp: {
         process_block(blocks.ramps.description[*block_index],
                       blocks.ramps.ids[*block_index], block_ramp_vertices,
                       block_ramp_triangles, block_ramp_occluders, mesh_list,
                       occluder_list);
      } break;
      case block_type::quad: {
         process_block(blocks.quads.description[*block_index],
                       blocks.quads.ids[*block_index], mesh_list);
      } break;
      case block_type::custom: {
         process_block(blocks.custom.description[*block_index],
                       blocks.custom.ids[*block_index],
                       blocks.custom_meshes[blocks.custom.mesh[*block_index]],
                       mesh_list, occluder_list);
      } break;
      case block_type::hemisphere: {
         process_block(blocks.hemispheres.description[*block_index],
                       blocks.hemispheres.ids[*block_index],
                       block_hemisphere_vertices, block_hemisphere_triangles,
                       block_hemisphere_occluders, mesh_list, occluder_list);
      } break;
      case block_type::pyramid: {
         process_block(blocks.pyramids.description[*block_index],
                       blocks.pyramids.ids[*block_index],
                       block_hemisphere_vertices, block_hemisphere_triangles,
                       block_hemisphere_occluders, mesh_list, occluder_list);
      } break;
      case block_type::terrain_cut_box: {
      } break;
      }
   }

   fill_bounding_boxes(mesh_list);

   mesh_list = cull_hidden_triangles(mesh_list, occluder_list);

   const std::vector<std::vector<uint32>> mesh_clusters =
      build_mesh_clusters(mesh_list, min_cluster_triangles, max_cluster_subdivision);

   return build_mesh_scenes(mesh_list, mesh_clusters, blocks.materials);
}

}