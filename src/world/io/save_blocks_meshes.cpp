#include "save_blocks_meshes.hpp"

#include "../blocks/export/mesh.hpp"
#include "../blocks/export/mesh_clusters.hpp"
#include "../blocks/export/mesh_cull.hpp"
#include "../blocks/export/mesh_scenes.hpp"
#include "../blocks/mesh_geometry.hpp"

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
                        const block_vertex& vertex,
                        const float3x3& world_from_object_adjugate,
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
   case block_texture_mode::tangent_space_xyz: {
      const float3 tangentWS = normalize(world_from_object_adjugate * vertex.tangent);
      const float3 bitangentWS =
         normalize(vertex.bitangent_sign * cross(normalWS, tangentWS));
      const float3x3 texture_from_world = {
         {tangentWS.x, bitangentWS.x, normalWS.x},
         {tangentWS.y, bitangentWS.y, normalWS.y},
         {tangentWS.z, bitangentWS.z, normalWS.z},
      };
      const float3 positionTS = texture_from_world * positionWS;

      texcoords = {positionTS.x, positionTS.y};
   } break;
   case block_texture_mode::unwrapped:
      texcoords = vertex.texcoords;
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
void process_blocks(const T& blocks, std::span<const block_vertex> block_vertices,
                    std::span<const std::array<uint16, 3>> block_triangles,
                    std::span<const std::array<uint16, 4>> block_occluders,
                    std::vector<block_world_triangle>& out_triangle_list,
                    std::vector<block_world_occluder>& out_occluder_list) noexcept
{
   using description_type = decltype(T::description)::value_type;
   using id_type = id<description_type>;

   for (uint32 block_index = 0; block_index < blocks.size(); ++block_index) {
      const description_type& block = blocks.description[block_index];
      const id_type block_id = blocks.ids[block_index];

      const float4x4 scale = {
         {block.size.x, 0.0f, 0.0f, 0.0f},
         {0.0f, block.size.y, 0.0f, 0.0f},
         {0.0f, 0.0f, block.size.z, 0.0f},
         {0.0f, 0.0f, 0.0f, 1.0f},
      };
      const float4x4 rotation = to_matrix(block.rotation);

      float4x4 world_from_object = rotation * scale;
      float3x3 world_from_object_adjugate = adjugate(world_from_object);
      world_from_object[3] = {block.position, 1.0f};

      for (const std::array<uint16, 3>& tri : block_triangles) {
         block_world_triangle world_triangle = {.block_id = block_id};

         for (std::size_t i = 0; i < tri.size(); ++i) {
            const block_vertex& vertex = block_vertices[tri[i]];

            const float3 positionWS = world_from_object * vertex.position;
            const float3 normalWS =
               normalize(world_from_object_adjugate * vertex.normal);
            const float2 texcoords =
               evaluate_texcorrds(positionWS, normalWS, vertex,
                                  world_from_object_adjugate,
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

         out_triangle_list.push_back(world_triangle);
      }

      for (const std::array<uint16, 4>& quad : block_occluders) {
         block_world_occluder occluder = {.block_id = block_id};

         for (std::size_t i = 0; i < quad.size(); ++i) {
            const block_vertex& vertex = block_vertices[quad[i]];

            occluder.verticesWS[i] = world_from_object * vertex.position;
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
}

auto foley_name(const block_foley_group group) noexcept -> const char*
{
   // clang-format off
   switch (group) {
   case block_foley_group::stone:   return "stone";
   case block_foley_group::dirt:    return "dirt";
   case block_foley_group::grass:   return "grass";
   case block_foley_group::metal:   return "metal";
   case block_foley_group::snow:    return "snow";
   case block_foley_group::terrain: return "terrain";
   case block_foley_group::water:   return "water";
   case block_foley_group::wood:    return "wood";
   }
   // clang-format on

   return "unknown";
}

}

auto save_blocks_meshes(const io::path& output_directory,
                        const std::string_view world_name, const blocks& blocks)
   -> std::size_t
{
   if (not io::exists(output_directory) and not io::create_directory(output_directory)) {
      throw std::runtime_error{"Unable to create directory to save blocks."};
   }

   std::size_t triangle_count = 0;

   triangle_count += blocks.boxes.size() * block_cube_triangles.size();
   triangle_count += blocks.ramps.size() * block_ramp_triangles.size();

   std::size_t occluder_count = 0;

   triangle_count += blocks.boxes.size() * block_cube_occluders.size();
   triangle_count += blocks.ramps.size() * block_ramp_occluders.size();

   std::vector<block_world_triangle> triangle_list;
   std::vector<block_world_occluder> occluder_list;

   triangle_list.reserve(triangle_count);
   occluder_list.reserve(occluder_count);

   process_blocks(blocks.boxes, block_cube_vertices, block_cube_triangles,
                  block_cube_occluders, triangle_list, occluder_list);
   process_blocks(blocks.ramps, block_ramp_vertices, block_ramp_triangles,
                  block_ramp_occluders, triangle_list, occluder_list);

   triangle_list = cull_hidden_triangles(triangle_list, occluder_list);

   const std::vector<std::vector<uint32>> triangle_clusters =
      build_mesh_clusters(triangle_list, min_cluster_triangles, max_cluster_subdivision);
   const std::vector<block_export_scene> scenes =
      build_mesh_scenes(triangle_list, triangle_clusters, blocks.materials);

   for (std::size_t scene_index = 0; scene_index < scenes.size(); ++scene_index) {
      const block_export_scene& scene = scenes[scene_index];
      const std::string mesh_name =
         fmt::format("WE_{}_blocks{}", world_name, scene_index);

      assets::msh::save_scene(io::compose_path(output_directory, mesh_name, ".msh"),
                              scene.msh_scene);

      io::output_file odf{io::compose_path(output_directory,
                                           fmt::format("WE_{}_blocks{}.odf",
                                                       world_name, scene_index))};

      odf.write_ln("[GameObjectClass]");
      odf.write_ln("");
      odf.write_ln("ClassLabel   = \"prop\"");
      odf.write_ln("GeometryName = \"{}.msh\"", mesh_name);
      odf.write_ln("");
      odf.write_ln("[Properties]");
      odf.write_ln("");
      odf.write_ln("GeometryName = \"{}\"", mesh_name);
      odf.write_ln("FoleyFXGroup = \"{}_foley\"", foley_name(scene.foley_group));
   }

   return scenes.size();
}

}