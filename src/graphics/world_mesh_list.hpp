#pragma once

#include "gpu/device.hpp"
#include "math/bounding_box.hpp"
#include "model.hpp"
#include "types.hpp"

#include <functional>
#include <vector>

namespace we::graphics {

struct world_mesh_constants {
   float4x4 object_to_world;
};

static_assert(sizeof(world_mesh_constants) == 64);

struct world_mesh {
   D3D12_INDEX_BUFFER_VIEW index_buffer_view;
   std::array<D3D12_VERTEX_BUFFER_VIEW, 3> vertex_buffer_views;
   uint32 index_count;
   uint32 start_index;
   uint32 start_vertex;
};

static_assert(std::is_trivially_copyable_v<world_mesh>, "Performance issue!");

struct world_mesh_list {
   std::vector<math::bounding_box> bbox;
   std::vector<D3D12_GPU_VIRTUAL_ADDRESS> gpu_constants;
   std::vector<float3> position;
   std::vector<ID3D12PipelineState*> pipeline;
   std::vector<gpu::material_pipeline_flags> pipeline_flags;
   std::vector<gpu::descriptor_range> material_descriptor_range;
   std::vector<world_mesh> mesh;

   void push_back(math::bounding_box mesh_bbox,
                  D3D12_GPU_VIRTUAL_ADDRESS mesh_gpu_constants,
                  float3 mesh_position, ID3D12PipelineState* mesh_pipeline,
                  gpu::material_pipeline_flags mesh_pipeline_flags,
                  gpu::descriptor_range mesh_material_descriptor_range,
                  world_mesh world_mesh) noexcept
   {
      bbox.push_back(mesh_bbox);
      gpu_constants.push_back(mesh_gpu_constants);
      position.push_back(mesh_position);
      pipeline.push_back(mesh_pipeline);
      pipeline_flags.push_back(mesh_pipeline_flags);
      material_descriptor_range.push_back(mesh_material_descriptor_range);
      mesh.push_back(world_mesh);
   }

   void reserve(std::size_t size) noexcept
   {
      invoke_over_all([size](auto& container) { container.reserve(size); });
   }

   void clear() noexcept
   {
      invoke_over_all([](auto& container) { container.clear(); });
   }

   auto size() const noexcept -> std::size_t
   {
      return bbox.size();
   }

private:
   void invoke_over_all(auto callback) noexcept
   {
      const auto invoke_all = [&](auto&... container) {
         (callback(container), ...);
      };

      invoke_all(bbox, gpu_constants, position, pipeline, pipeline_flags,
                 material_descriptor_range, mesh);
   }
};

}
