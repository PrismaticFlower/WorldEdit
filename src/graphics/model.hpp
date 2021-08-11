#pragma once

#include "assets/msh/flat_model.hpp"
#include "gpu/device.hpp"
#include "material.hpp"
#include "math/bounding_box.hpp"
#include "texture_manager.hpp"
#include "types.hpp"

#include <array>
#include <span>
#include <vector>

#include <boost/container/small_vector.hpp>

namespace we::graphics {

struct mesh_vertices {
   std::span<float3> positions;
   std::span<float3> normals;
   std::span<float3> tangents;
   std::span<float3> bitangents;
   std::span<float2> texcoords;
};

struct mesh_part {
   uint32 index_count;
   uint32 start_index;
   uint32 start_vertex;
   material material;
};

struct mesh_data_offsets {
   std::size_t indices;
   std::size_t positions;
   std::size_t normals;
   std::size_t tangents;
   std::size_t bitangents;
   std::size_t texcoords;
};

struct mesh_gpu_buffer {
   gpu::buffer buffer;

   D3D12_INDEX_BUFFER_VIEW index_buffer_view;
   D3D12_VERTEX_BUFFER_VIEW position_vertex_buffer_view;
   D3D12_VERTEX_BUFFER_VIEW normal_vertex_buffer_view;
   D3D12_VERTEX_BUFFER_VIEW tangent_vertex_buffer_view;
   D3D12_VERTEX_BUFFER_VIEW bitangent_vertex_buffer_view;
   D3D12_VERTEX_BUFFER_VIEW texcoord_vertex_buffer_view;
};

struct model {
   explicit model(const assets::msh::flat_model& model, gpu::device& gpu_device,
                  texture_manager& texture_manager);

   auto init_gpu_buffer_async(gpu::device& device) -> UINT64;

   math::bounding_box bbox;

   std::vector<std::byte> buffer;

   mesh_vertices vertices;
   std::span<std::array<uint16, 3>> indices;

   mesh_data_offsets data_offsets;
   mesh_gpu_buffer gpu_buffer;

   boost::container::small_vector<mesh_part, 8> parts;
};

}
