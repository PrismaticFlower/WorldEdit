#pragma once

#include "container/slim_bitset.hpp"
#include "dynamic_buffer_allocator.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"
#include "math/frustum.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "texture_manager.hpp"
#include "types.hpp"
#include "world/world.hpp"

#include <span>
#include <vector>

namespace we::settings {

struct graphics;

}

namespace we::graphics {

struct terrain_vertex {
   std::array<int16, 4> positionCS = {};
};

struct terrain_vertex_attributes {
   std::array<int8, 4> normalWS = {};
   std::array<uint8, 16> weights = {};
   uint32 static_light = 0;
};

struct terrain_cut {
   math::bounding_box bbox;

   gpu_virtual_address constant_buffer;
   gpu::index_buffer_view index_buffer_view;
   gpu::vertex_buffer_view position_vertex_buffer_view;

   uint32 index_count = 0;
   uint32 start_index = 0;
   uint32 start_vertex = 0;
};

enum class terrain_draw { depth_prepass, main, grid, foliage_map };

class terrain {
public:
   constexpr static std::size_t texture_count = assets::terrain::terrain::texture_count;
   constexpr static uint32 patch_length_grids = 16;
   constexpr static uint32 patch_length_points = 17;
   constexpr static uint32 patch_vertex_count =
      patch_length_points * patch_length_points;

   terrain(gpu::device& device, copy_command_list_pool& copy_command_list_pool,
           dynamic_buffer_allocator& dynamic_buffer_allocator,
           texture_manager& texture_manager);

   void update(const world::terrain& terrain, gpu::copy_command_list& command_list,
               dynamic_buffer_allocator& dynamic_buffer_allocator,
               texture_manager& texture_manager, const settings::graphics& settings);

   void predraw_cull(const frustum& view_frustum,
                     dynamic_buffer_allocator& dynamic_buffer_allocator);

   void draw(const terrain_draw draw, const frustum& view_frustum,
             std::span<const terrain_cut> terrain_cuts,
             gpu_virtual_address camera_constant_buffer_view,
             gpu_virtual_address lights_constant_buffer_view,
             gpu::graphics_command_list& command_list,
             root_signature_library& root_signatures, pipeline_library& pipelines);

   void process_updated_texture(const updated_textures& updated);

private:
   struct terrain_patch {
      int16 min_y = 0;
      int16 max_y = 0;
   };

   void create_patches();

   void create_gpu_resources();

   void draw_cuts(const frustum& view_frustum, std::span<const terrain_cut> terrain_cuts,
                  gpu_virtual_address camera_constant_buffer_view,
                  gpu::graphics_command_list& command_list,
                  root_signature_library& root_signatures,
                  pipeline_library& pipelines);

   gpu::device& _device;

   bool _active = false;

   uint32 _terrain_length = 0;
   uint32 _patches_length = 0;

   float2 _terrain_half_world_size = {0.0f, 0.0f};
   float _terrain_grid_size = 0.0f;
   float _terrain_height_scale = 0.0f;

   std::vector<terrain_patch> _patches;

   std::vector<std::array<terrain_vertex, patch_vertex_count>> _patch_vertices;
   std::vector<std::array<terrain_vertex_attributes, patch_vertex_count>> _patch_vertex_attributes;

   std::vector<bool> _dirty_vertex_patches;
   std::vector<uint32> _vertex_patches_to_upload;

   std::vector<bool> _dirty_vertex_attributes_patches;
   std::vector<uint32> _vertex_attributes_patches_to_upload;

   gpu::unique_resource_handle _index_buffer;
   gpu::unique_resource_handle _vertex_buffer;
   gpu::unique_resource_handle _terrain_constants_buffer;
   gpu::unique_resource_handle _foliage_map;

   uint32 _vertex_attributes_offset = 0;
   std::array<gpu::vertex_buffer_view, 2> _vertex_buffer_views = {};

   gpu_virtual_address _terrain_cbv;
   gpu::unique_resource_view _foliage_map_srv;

   std::array<lowercase_string, texture_count> _diffuse_maps_names;
   std::array<std::shared_ptr<const world_texture>, texture_count> _diffuse_maps;
   std::array<std::shared_ptr<const world_texture_load_token>, texture_count> _diffuse_map_load_tokens;

   uint32 _foliage_map_upload_row_pitch = 0;

   gpu::unique_resource_handle _upload_buffer;
   std::array<std::byte*, gpu::frame_pipeline_length> _vertices_upload_ptr;
   std::array<uint32, gpu::frame_pipeline_length> _vertices_upload_offset;
   std::array<std::byte*, gpu::frame_pipeline_length> _vertex_attributes_upload_ptr;
   std::array<uint32, gpu::frame_pipeline_length> _vertex_attributes_upload_offset;
   std::array<std::byte*, gpu::frame_pipeline_length> _foliage_map_upload_ptr;
   std::array<uint32, gpu::frame_pipeline_length> _foliage_map_upload_offset;
};

}
