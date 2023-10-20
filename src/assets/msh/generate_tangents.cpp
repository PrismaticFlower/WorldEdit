#include "generate_tangents.hpp"
#include "mikktspace/mikktspace.h"

#include <bit>
#include <limits>
#include <stdexcept>

#include <absl/container/flat_hash_map.h>

namespace we::assets::msh {

namespace {

struct mesh_tri_list {
   explicit mesh_tri_list(const generate_tangents_input& input)
   {
      const auto size = input.triangles.size();

      positions.resize(size);
      normals.resize(size);
      tangents.resize(size);
      bitangents.resize(size);
      texcoords.resize(size);
      colors.resize(size);
   }

   std::vector<std::array<float3, 3>> positions;
   std::vector<std::array<float3, 3>> normals;
   std::vector<std::array<float3, 3>> tangents;
   std::vector<std::array<float3, 3>> bitangents;
   std::vector<std::array<uint32, 3>> colors;
   std::vector<std::array<float2, 3>> texcoords;
};

struct tspace_user_data {
   generate_tangents_input& input;
   mesh_tri_list& output;
};

SMikkTSpaceInterface mikktspace_interface{
   .m_getNumFaces = [](const SMikkTSpaceContext* context) -> int {
      generate_tangents_input& input =
         static_cast<tspace_user_data*>(context->m_pUserData)->input;

      return static_cast<int>(input.triangles.size());
   },

   .m_getNumVerticesOfFace = [](const SMikkTSpaceContext*, const int) -> int {
      return 3;
   },

   .m_getPosition =
      [](const SMikkTSpaceContext* context, float pos_out[], const int face,
         const int vert) {
         generate_tangents_input& input =
            static_cast<tspace_user_data*>(context->m_pUserData)->input;

         std::memcpy(pos_out, &input.positions[input.triangles[face][vert]],
                     sizeof(float3));
      },

   .m_getNormal =
      [](const SMikkTSpaceContext* context, float norm_out[], const int face,
         const int vert) {
         generate_tangents_input& input =
            static_cast<tspace_user_data*>(context->m_pUserData)->input;

         std::memcpy(norm_out, &input.normals[input.triangles[face][vert]],
                     sizeof(float3));
      },

   .m_getTexCoord =
      [](const SMikkTSpaceContext* context, float texcoord_out[],
         const int face, const int vert) {
         generate_tangents_input& input =
            static_cast<tspace_user_data*>(context->m_pUserData)->input;

         std::memcpy(texcoord_out, &input.texcoords[input.triangles[face][vert]],
                     sizeof(float2));
      },

   .m_setTSpaceBasic = nullptr,

   .m_setTSpace =
      [](const SMikkTSpaceContext* context, const float tangent[],
         const float bitangent[], [[maybe_unused]] const float mags,
         [[maybe_unused]] const float magt,
         [[maybe_unused]] const tbool is_orientation_preserving, const int face,
         const int vert) {
         tspace_user_data& user_data =
            *static_cast<tspace_user_data*>(context->m_pUserData);
         generate_tangents_input& input = user_data.input;
         mesh_tri_list& output = user_data.output;

         std::memcpy(&output.tangents[face][vert], tangent, sizeof(float3));
         std::memcpy(&output.bitangents[face][vert], bitangent, sizeof(float3));

         const auto input_index = input.triangles[face][vert];

         output.positions[face][vert] = input.positions[input_index];
         output.normals[face][vert] = input.normals[input_index];
         output.texcoords[face][vert] = input.texcoords[input_index];
         output.colors[face][vert] = input.colors[input_index];
      }};

struct cached_vertex {
   float3 position;
   float3 normal;
   float3 tangent;
   float3 bitangent;
   float2 texcoord;
   uint32 color;

   bool operator==(const cached_vertex& other) const noexcept
   {
      return std::memcmp(&other, this, sizeof(cached_vertex)) == 0;
   }

   template<typename H>
   friend H AbslHashValue(H h, const cached_vertex& cached)
   {
      return H::combine(std::move(h),
                        std::bit_cast<std::array<uint8, sizeof(cached_vertex)>>(cached));
   }
};

auto indexify_tri_list(const mesh_tri_list& tri_list) -> generate_tangents_output
{
   generate_tangents_output output;

   const auto reserve_size = tri_list.positions.size() * 3;

   output.positions.reserve(reserve_size);
   output.normals.reserve(reserve_size);
   output.tangents.reserve(reserve_size);
   output.bitangents.reserve(reserve_size);
   output.texcoords.reserve(reserve_size);
   output.colors.reserve(tri_list.colors.size() * 3);
   output.triangles.reserve(tri_list.positions.size());

   absl::flat_hash_map<cached_vertex, std::size_t> cache;
   cache.reserve(reserve_size);

   for (std::size_t tri_index = 0; tri_index < tri_list.positions.size(); ++tri_index) {
      auto& tri = output.triangles.emplace_back();

      for (std::size_t vertex_index = 0; vertex_index < tri.size(); ++vertex_index) {
         cached_vertex vertex{.position = tri_list.positions[tri_index][vertex_index],
                              .normal = tri_list.normals[tri_index][vertex_index],
                              .tangent = tri_list.tangents[tri_index][vertex_index],
                              .bitangent = tri_list.bitangents[tri_index][vertex_index],
                              .texcoord = tri_list.texcoords[tri_index][vertex_index],
                              .color = tri_list.colors[tri_index][vertex_index]};

         auto [index_it, inserted] = cache.emplace(vertex, output.positions.size());

         tri[vertex_index] = static_cast<uint16>(index_it->second);

         if (inserted) {
            output.positions.push_back(vertex.position);
            output.normals.push_back(vertex.normal);
            output.tangents.push_back(vertex.tangent);
            output.bitangents.push_back(vertex.bitangent);
            output.texcoords.push_back(vertex.texcoord);
            output.colors.push_back(vertex.color);
         }
      }
   }

   // TODO: Shrink to fit to save memory here?

   if (output.positions.size() > std::numeric_limits<uint16>::max()) {
      throw std::runtime_error{
         "Mesh has too many vertices after generating tangents! Ask nicely to "
         "have this fixed, include the mesh!"};
   }

   return output;
}

}

auto generate_tangents(generate_tangents_input input) -> generate_tangents_output
{
   mesh_tri_list tri_list{input};

   tspace_user_data user_data{.input = input, .output = tri_list};

   SMikkTSpaceContext context{&mikktspace_interface, &user_data};

   genTangSpaceDefault(&context);

   return indexify_tri_list(tri_list);
}

}
