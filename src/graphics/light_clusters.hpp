#pragma once

#include "frustrum.hpp"
#include "gpu/buffer.hpp"
#include "gpu/command_list.hpp"
#include "gpu/device.hpp"
#include "gpu/dynamic_buffer_allocator.hpp"
#include "model_manager.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "terrain.hpp"
#include "world/object_class.hpp"
#include "world/world.hpp"
#include "world_mesh_list.hpp"

#include <array>
#include <memory>
#include <vector>

#include <absl/container/flat_hash_map.h>

namespace we::graphics {

class light_clusters {
public:
   light_clusters(gpu::device& gpu_device, uint32 render_width, uint32 render_height);

   void update_render_resolution(uint32 width, uint32 height);

   void update_lights(const camera& view_camera, const frustrum& view_frustrum,
                      const world::world& world, root_signature_library& root_signatures,
                      pipeline_library& pipelines,
                      gpu::graphics_command_list& command_list,
                      gpu::dynamic_buffer_allocator& dynamic_buffer_allocator);

   void TEMP_render_shadow_maps(
      const camera& view_camera, const frustrum& view_frustrum,
      const world_mesh_list& meshes, const world::world& world,
      root_signature_library& root_signatures, pipeline_library& pipelines,
      gpu::graphics_command_list& command_list,
      gpu::dynamic_buffer_allocator& dynamic_buffer_allocator);

   auto light_descriptors() const noexcept -> gpu::descriptor_range;

private:
   void update_render_resolution(uint32 width, uint32 height, bool recreate_descriptors);

   void update_descriptors();

   void init_proxy_geometry(gpu::device& device);

   gsl::not_null<gpu::device*> _gpu_device;

   gpu::buffer _tiling_inputs;

   gpu::buffer _lights_constants;
   gpu::buffer _lights_tiles;
   gpu::buffer _lights_list;
   gpu::buffer _lights_region_list;

   gpu::buffer _sphere_proxy_indices;
   gpu::buffer _sphere_proxy_vertices;

   gpu::texture _shadow_map;
   gpu::descriptor_allocation _shadow_map_dsv;
   std::array<float4x4, 4> _shadow_cascade_transforms;

   gpu::resource_view_set _tiling_resource_views;
   gpu::resource_view_set _resource_views;

   uint32 _tiles_count = 0;
   uint32 _tiles_width = 0;
   uint32 _tiles_height = 0;
   float _render_width = 0.0f;
   float _render_height = 0.0f;
};

}
