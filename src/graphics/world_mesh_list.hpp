#pragma once

#include "allocators/aligned_allocator.hpp"
#include "container/enum_array.hpp"
#include "gpu/rhi.hpp"
#include "model.hpp"
#include "pipeline_library.hpp"
#include "types.hpp"

#include <functional>
#include <vector>

namespace we::graphics {

/// @brief The flags we want to use line up depth_prepass_pipeline_flags even though we won't use those directly
using mesh_opaque_flags = depth_prepass_pipeline_flags;

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

struct world_bbox_soa {
   using float_soa_vector = std::vector<float, aligned_allocator<float, 32>>;

   struct {
      float_soa_vector x;
      float_soa_vector y;
      float_soa_vector z;
   } min;

   struct {
      float_soa_vector x;
      float_soa_vector y;
      float_soa_vector z;
   } max;
};

struct world_opaque_mesh_list {
   world_bbox_soa bbox;
   std::vector<gpu_virtual_address> gpu_constants;
   std::vector<gpu_virtual_address> material_constant_buffer;
   std::vector<world_mesh> mesh;

   void push_back(math::bounding_box mesh_bbox, gpu_virtual_address mesh_gpu_constants,
                  gpu_virtual_address mesh_material_constant_buffer,
                  world_mesh world_mesh) noexcept
   {
      bbox.min.x.push_back(mesh_bbox.min.x);
      bbox.min.y.push_back(mesh_bbox.min.y);
      bbox.min.z.push_back(mesh_bbox.min.z);
      bbox.max.x.push_back(mesh_bbox.max.x);
      bbox.max.y.push_back(mesh_bbox.max.y);
      bbox.max.z.push_back(mesh_bbox.max.z);
      gpu_constants.push_back(mesh_gpu_constants);
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
      return bbox.min.x.size();
   }

private:
   void invoke_over_all(auto callback) noexcept
   {
      const auto invoke_all = [&](auto&... container) {
         (callback(container), ...);
      };

      invoke_all(bbox.min.x, bbox.min.y, bbox.min.z, bbox.max.x, bbox.max.y,
                 bbox.max.z, gpu_constants, material_constant_buffer, mesh);
   }
};

struct world_transparent_mesh_list {
   world_bbox_soa bbox;
   std::vector<gpu_virtual_address> gpu_constants;
   std::vector<float3> position;
   std::vector<material_pipeline_flags> pipeline_flags;
   std::vector<gpu_virtual_address> material_constant_buffer;
   std::vector<world_mesh> mesh;

   void push_back(math::bounding_box mesh_bbox,
                  gpu_virtual_address mesh_gpu_constants, float3 mesh_position,
                  material_pipeline_flags material_pipeline_flags,
                  gpu_virtual_address mesh_material_constant_buffer,
                  world_mesh world_mesh) noexcept
   {
      bbox.min.x.push_back(mesh_bbox.min.x);
      bbox.min.y.push_back(mesh_bbox.min.y);
      bbox.min.z.push_back(mesh_bbox.min.z);
      bbox.max.x.push_back(mesh_bbox.max.x);
      bbox.max.y.push_back(mesh_bbox.max.y);
      bbox.max.z.push_back(mesh_bbox.max.z);
      gpu_constants.push_back(mesh_gpu_constants);
      position.push_back(mesh_position);
      pipeline_flags.push_back(material_pipeline_flags);
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
      return bbox.min.x.size();
   }

private:
   void invoke_over_all(auto callback) noexcept
   {
      const auto invoke_all = [&](auto&... container) {
         (callback(container), ...);
      };

      invoke_all(bbox.min.x, bbox.min.y, bbox.min.z, bbox.max.x, bbox.max.y,
                 bbox.max.z, gpu_constants, position, pipeline_flags,
                 material_constant_buffer, mesh);
   }
};

struct world_mesh_list {
   container::enum_array<world_opaque_mesh_list, mesh_opaque_flags> opaque;
   world_transparent_mesh_list transparent;

   void clear() noexcept
   {
      for (world_opaque_mesh_list& list : opaque) list.clear();

      transparent.clear();
   }
};

struct world_mesh_render_list {
   container::enum_array<std::vector<uint16>, mesh_opaque_flags> opaque;
   std::vector<uint16> transparent;
};

}
