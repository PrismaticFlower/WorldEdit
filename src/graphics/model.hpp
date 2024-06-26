#pragma once

#include "assets/msh/flat_model.hpp"
#include "copy_command_list_pool.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"
#include "material.hpp"
#include "math/bounding_box.hpp"
#include "texture_manager.hpp"
#include "types.hpp"

#include <array>
#include <span>
#include <vector>

#include <absl/container/inlined_vector.h>

namespace we::graphics {

struct mesh_attributes {
   std::array<int16, 4> normals;
   std::array<int16, 4> tangents;
   std::array<int16, 4> bitangents;
   float2 texcoords;
   uint32 color;
};

struct mesh_vertices {
   std::span<float3> positions;
   std::span<mesh_attributes> attributes;
};

struct mesh_part {
   uint32 index_count;
   uint32 start_index;
   uint32 start_vertex;
   material material;
};

struct mesh_terrain_cut {
   math::bounding_box bbox;
   uint32 index_count;
   uint32 start_index;
   uint32 start_vertex;
};

struct mesh_data_offsets {
   std::size_t indices;
   std::size_t positions;
   std::size_t attributes;
};

struct mesh_gpu_buffer {
   gpu::index_buffer_view index_buffer_view;
   gpu::vertex_buffer_view position_vertex_buffer_view;
   gpu::vertex_buffer_view attributes_vertex_buffer_view;

   gpu::unique_resource_handle buffer;
};

struct model {
   explicit model(const assets::msh::flat_model& model, gpu::device& device,
                  copy_command_list_pool& copy_command_list_pool,
                  texture_manager& texture_manager);

   math::bounding_box bbox;

   mesh_data_offsets data_offsets;
   mesh_gpu_buffer gpu_buffer;

   absl::InlinedVector<mesh_part, 8> parts;

   math::bounding_box terrain_cuts_bbox;

   std::vector<mesh_terrain_cut> terrain_cuts;

private:
   void init_gpu_buffer(gpu::device& device,
                        copy_command_list_pool& copy_command_list_pool,
                        std::span<const std::byte> buffer);
};

}
