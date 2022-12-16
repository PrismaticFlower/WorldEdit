#pragma once

#include "gpu/rhi.hpp"
#include "math/bounding_box.hpp"
#include "model.hpp"
#include "pipeline_library.hpp"
#include "types.hpp"

#include <functional>
#include <vector>

namespace we::graphics {

struct alignas(16) world_mesh_constants {
   float4x4 object_to_world;
   std::array<std::byte, 192> padding;
};

static_assert(sizeof(world_mesh_constants) == 256);

struct world_mesh {
   gpu::index_buffer_view index_buffer_view;
   std::array<gpu::vertex_buffer_view, 2> vertex_buffer_views;
   uint32 index_count;
   uint32 start_index;
   uint32 start_vertex;
};

static_assert(std::is_trivially_copyable_v<world_mesh>, "Performance issue!");

struct world_mesh_list {
   std::vector<math::bounding_box> bbox;
   std::vector<gpu_virtual_address> gpu_constants;
   std::vector<float3> position;
   std::vector<gpu::pipeline_handle> pipeline;
   std::vector<material_pipeline_flags> pipeline_flags;
   std::vector<gpu_virtual_address> material_constant_buffer;
   std::vector<world_mesh> mesh;

   void push_back(math::bounding_box mesh_bbox, gpu_virtual_address mesh_gpu_constants,
                  float3 mesh_position, gpu::pipeline_handle mesh_pipeline,
                  material_pipeline_flags mesh_pipeline_flags,
                  gpu_virtual_address mesh_material_constant_buffer,
                  world_mesh world_mesh) noexcept
   {
      bbox.push_back(mesh_bbox);
      gpu_constants.push_back(mesh_gpu_constants);
      position.push_back(mesh_position);
      pipeline.push_back(mesh_pipeline);
      pipeline_flags.push_back(mesh_pipeline_flags);
      material_constant_buffer.push_back(mesh_material_constant_buffer);
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
                 material_constant_buffer, mesh);
   }
};

}
