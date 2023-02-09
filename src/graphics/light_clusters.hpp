#pragma once

#include "camera.hpp"
#include "copy_command_list_pool.hpp"
#include "dynamic_buffer_allocator.hpp"
#include "frustum.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"
#include "model_manager.hpp"
#include "pipeline_library.hpp"
#include "profiler.hpp"
#include "root_signature_library.hpp"
#include "shadow_camera.hpp"
#include "terrain.hpp"
#include "world/object_class.hpp"
#include "world/world.hpp"
#include "world_mesh_list.hpp"

#include <array>
#include <memory>
#include <vector>

namespace we::graphics {

class light_clusters {
public:
   light_clusters(gpu::device& device, copy_command_list_pool& copy_command_list_pool,
                  uint32 render_width, uint32 render_height);

   void update_render_resolution(uint32 width, uint32 height);

   void prepare_lights(const camera& view_camera, const frustum& view_frustum,
                       const world::world& world,
                       const world::light* optional_placement_light,
                       const std::array<float, 2> scene_depth_min_max,
                       gpu::copy_command_list& command_list,
                       dynamic_buffer_allocator& dynamic_buffer_allocator);

   void tile_lights(root_signature_library& root_signatures, pipeline_library& pipelines,
                    gpu::graphics_command_list& command_list,
                    dynamic_buffer_allocator& dynamic_buffer_allocator,
                    profiler& profiler);

   void draw_shadow_maps(const world_mesh_list& meshes,
                         root_signature_library& root_signatures,
                         pipeline_library& pipelines,
                         gpu::graphics_command_list& command_list,
                         dynamic_buffer_allocator& dynamic_buffer_allocator,
                         profiler& profiler);

   auto lights_constant_buffer_view() const noexcept -> gpu_virtual_address;

   constexpr static uint32 sun_cascade_count = 4;

private:
   void update_render_resolution(uint32 width, uint32 height, bool recreate_descriptors);

   void update_descriptors();

   void init_proxy_geometry(gpu::device& device,
                            copy_command_list_pool& copy_command_list_pool);

   gpu::device& _device;

   gpu::unique_resource_handle _tiling_inputs;

   gpu::unique_resource_handle _lights_constants;
   gpu::unique_resource_handle _lights_tiles;
   gpu::unique_resource_handle _lights_region_list;

   gpu::unique_resource_handle _sphere_proxy_indices;
   gpu::unique_resource_handle _sphere_proxy_vertices;

   gpu::unique_resource_handle _shadow_map;
   std::array<gpu::unique_dsv_handle, 4> _shadow_map_dsv;

   gpu_virtual_address _tiling_inputs_cbv;

   gpu_virtual_address _lights_constant_buffer_view;
   gpu::unique_resource_view _lights_tiles_srv;
   gpu::unique_resource_view _lights_region_list_srv;
   gpu::unique_resource_view _shadow_map_srv;

   uint32 _tiles_count = 0;
   uint32 _tiles_width = 0;
   uint32 _tiles_height = 0;
   float _render_width = 0.0f;
   float _render_height = 0.0f;

   std::array<uint32, 8> _tiles_start_value;
   uint32 _light_count = 0;
   gpu_virtual_address _sphere_light_proxies_srv = 0;

   std::array<shadow_ortho_camera, sun_cascade_count> _sun_shadow_cascades;
   std::vector<uint16> _shadow_render_list;
};

}
