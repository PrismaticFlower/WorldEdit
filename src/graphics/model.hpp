#pragma once

#include "assets/msh/flat_model.hpp"
#include "gpu/buffer.hpp"
#include "math/bounding_box.hpp"
#include "types.hpp"

#include <array>
#include <vector>

#include <boost/container/small_vector.hpp>

namespace sk::graphics {

struct mesh_vertices {
   std::vector<float3> positions;
   std::vector<float3> normals;
   std::vector<float2> texcoords;
};

struct mesh_part {
   uint32 index_count;
   uint32 start_index;
   uint32 start_vertex;
};

struct mesh_gpu_buffer {
   gpu::buffer buffer;

   D3D12_INDEX_BUFFER_VIEW index_buffer_view;
   D3D12_VERTEX_BUFFER_VIEW position_vertex_buffer_view;
   D3D12_VERTEX_BUFFER_VIEW normal_vertex_buffer_view;
   D3D12_VERTEX_BUFFER_VIEW texcoord_vertex_buffer_view;
};

struct model {
   explicit model(const assets::msh::flat_model& model);

   auto init_gpu_buffer_async(gpu::device& device) -> UINT64;

   math::bounding_box bbox;

   mesh_vertices vertices;
   std::vector<std::array<uint16, 3>> indices;

   mesh_gpu_buffer gpu_buffer;

   boost::container::small_vector<mesh_part, 8> parts;
};

}
