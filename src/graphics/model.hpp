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

struct mesh_attributes {
   float3 normals;
   float3 tangents;
   float3 bitangents;
   float2 texcoords;
};

struct mesh_vertices {
   std::span<float3> positions;
   std::span<mesh_attributes> attributes;
};

struct mesh_part {
   uint32 index_count;
   uint32 start_index;
   uint32 start_vertex;
   uint32 vertex_count;
   material material;

   gpu::buffer raytracing_blas;
};

struct mesh_data_offsets {
   std::size_t indices;
   std::size_t positions;
   std::size_t attributes;
};

struct mesh_gpu_buffer {
   gpu::buffer buffer;

   D3D12_INDEX_BUFFER_VIEW index_buffer_view;
   D3D12_VERTEX_BUFFER_VIEW position_vertex_buffer_view;
   D3D12_VERTEX_BUFFER_VIEW attributes_vertex_buffer_view;
};

struct model {
   explicit model(const assets::msh::flat_model& model, gpu::device& device,
                  texture_manager& texture_manager);

   math::bounding_box bbox;

   mesh_data_offsets data_offsets;
   mesh_gpu_buffer gpu_buffer;

   boost::container::small_vector<mesh_part, 8> parts;

private:
   void init_gpu_buffer_async(gpu::device& device, std::span<const std::byte> buffer,
                              std::span<std::array<uint16, 3>> indices,
                              mesh_vertices vertices);
};

}
