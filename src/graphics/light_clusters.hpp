#pragma once

#include "billboard_patches.hpp"
#include "blocks.hpp"
#include "camera.hpp"
#include "copy_command_list_pool.hpp"
#include "dynamic_buffer_allocator.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"
#include "model_manager.hpp"
#include "pipeline_library.hpp"
#include "profiler.hpp"
#include "root_signature_library.hpp"
#include "shadow_camera.hpp"
#include "terrain.hpp"
#include "world_mesh_list.hpp"

#include "math/frustum.hpp"

#include "world/active_elements.hpp"
#include "world/object_class.hpp"
#include "world/world.hpp"

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
                       const world::entity_group* optional_entity_group,
                       const std::array<float, 2> scene_depth_min_max,
                       blocks& blocks, billboard_patches& billboard_patches,
                       const world::active_layers active_layers,
                       const world::active_entity_types active_entity_types,
                       gpu::copy_command_list& command_list,
                       dynamic_buffer_allocator& dynamic_buffer_allocator);

   void tile_lights(root_signature_library& root_signatures, pipeline_library& pipelines,
                    gpu::graphics_command_list& command_list, profiler& profiler);

   void draw_shadow_maps(const world_mesh_list& meshes, const blocks& blocks,
                         const billboard_patches& billboard_patches,
                         root_signature_library& root_signatures,
                         pipeline_library& pipelines,
                         gpu::graphics_command_list& command_list,
                         dynamic_buffer_allocator& dynamic_buffer_allocator,
                         profiler& profiler);

   auto lights_constant_buffer_view() const noexcept -> gpu_virtual_address;

   constexpr static uint32 sun_cascade_count = 4;
   constexpr static uint32 max_onscreen_lights = 256;

   enum class light_type : uint32 {
      directional_box,
      directional_sphere,
      directional_cylinder,
      point,
      spot
   };

   struct global_light {
      float3 directionWS;
      uint32 is_dynamic;
      float3 color;
      uint32 has_shadows;
   };

   struct light {
      light_type type = light_type::point;

      uint32 is_dynamic;

      struct point_desc {
         float3 positionWS;
         float range;
         float3 color;
      };

      struct spot_desc {
         float3 positionWS;
         float range;
         float3 color;
         float3 directionWS;
         float spot_outer_param;
         float spot_inner_param;
      };

      struct directional_box_desc {
         float3 color;
         float3 directionWS;
         float4x4 world_from_region;
         float4x4 region_from_world;
         float3 size;
      };

      struct directional_sphere_desc {
         float3 color;
         float3 directionWS;
         float4x4 world_from_region;
         float4x4 region_from_world;
         float radius;
      };

      struct directional_cylinder_desc {
         float3 color;
         float3 directionWS;
         float4x4 world_from_region;
         float4x4 region_from_world;
         float radius;
         float height;
      };

      union {
         point_desc point = {};
         spot_desc spot;
         directional_box_desc directional_box;
         directional_sphere_desc directional_sphere;
         directional_cylinder_desc directional_cylinder;
      };
   };

private:
   void update_render_resolution(uint32 width, uint32 height, bool recreate_descriptors);

   void update_descriptors();

   void init_proxy_geometry(gpu::device& device,
                            copy_command_list_pool& copy_command_list_pool);

   void draw_meshes_shadow_map(const world_opaque_mesh_list& meshes,
                               const std::span<const uint16> render_list,
                               gpu::pipeline_handle pipeline,
                               gpu::graphics_command_list& command_list) const;

   void draw_meshes_alpha_cutout_shadow_map(const world_opaque_mesh_list& meshes,
                                            const std::span<const uint16> render_list,
                                            gpu::pipeline_handle pipeline,
                                            gpu::graphics_command_list& command_list) const;

   void add_world_lights(const camera& view_camera, const frustum& view_frustum,
                         const world::world& world,
                         const world::light* optional_placement_light,
                         const world::entity_group* optional_entity_group);

   auto try_add_light(float distance) noexcept -> light*;

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

   uint32 _light_proxy_count = 0;
   gpu_virtual_address _sphere_light_proxies_srv = 0;

   std::array<float, 2> _scene_depth_min_max = {0.0f, 1.0f};

   float3 _ambient_sky_color;
   float3 _ambient_ground_color;

   std::array<global_light, 2> _global_lights;

   struct light_entry {
      float distance = FLT_MAX;
      uint32 light_index = 0;
   };

   std::array<light_entry, max_onscreen_lights> _lights_order;
   std::array<light, max_onscreen_lights> _lights;

   uint32 _lights_allocated = 0;

   bool _has_sun_shadows = false;

   std::array<shadow_ortho_camera, sun_cascade_count> _sun_shadow_cascades;
   std::array<blocks::view, sun_cascade_count> _sun_shadow_blocks_view;
   std::array<billboard_patches::view, sun_cascade_count> _sun_shadow_billboard_patches_view;
   std::vector<uint16> _shadow_render_list;
};

}
