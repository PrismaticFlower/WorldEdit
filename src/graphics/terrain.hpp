#pragma once

#include "container/slim_bitset.hpp"
#include "dynamic_buffer_allocator.hpp"
#include "frustum.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "texture_manager.hpp"
#include "types.hpp"
#include "world/world.hpp"

#include <span>
#include <vector>

namespace we::graphics {

struct terrain_cut {
   math::bounding_box bbox;

   gpu_virtual_address constant_buffer;
   gpu::index_buffer_view index_buffer_view;
   gpu::vertex_buffer_view position_vertex_buffer_view;

   uint32 index_count = 0;
   uint32 start_index = 0;
   uint32 start_vertex = 0;
};

enum class terrain_draw { depth_prepass, main };

class terrain {
public:
   constexpr static std::size_t texture_count = assets::terrain::terrain::texture_count;

   terrain(gpu::device& device, copy_command_list_pool& copy_command_list_pool,
           dynamic_buffer_allocator& dynamic_buffer_allocator,
           texture_manager& texture_manager);

   void update(const world::terrain& terrain, gpu::copy_command_list& command_list,
               dynamic_buffer_allocator& dynamic_buffer_allocator,
               texture_manager& texture_manager);

   void draw(const terrain_draw draw, const frustum& view_frustum,
             std::span<const terrain_cut> terrain_cuts,
             gpu_virtual_address camera_constant_buffer_view,
             gpu_virtual_address lights_constant_buffer_view,
             gpu::graphics_command_list& command_list,
             root_signature_library& root_signatures, pipeline_library& pipelines,
             dynamic_buffer_allocator& dynamic_buffer_allocator);

   void process_updated_texture(const updated_textures& updated);

private:
   struct terrain_patch {
      int16 min_y = 0;
      int16 max_y = 0;
      container::slim_bitset<16> active_textures;
   };

   void create_gpu_textures();

   void create_unfilled_patch_info();

   void draw_cuts(const frustum& view_frustum, std::span<const terrain_cut> terrain_cuts,
                  gpu_virtual_address camera_constant_buffer_view,
                  gpu::graphics_command_list& command_list,
                  root_signature_library& root_signatures,
                  pipeline_library& pipelines);

   gpu::device& _device;

   bool _active = false;

   uint32 _terrain_length = 0;
   uint32 _patch_count = 0;

   float2 _terrain_half_world_size = {0.0f, 0.0f};
   float _terrain_grid_size = 0.0f;
   float _terrain_height_scale = 0.0f;

   std::vector<terrain_patch> _patches;

   gpu::unique_resource_handle _index_buffer;
   gpu::unique_resource_handle _terrain_constants_buffer;
   gpu::unique_resource_handle _height_map;
   gpu::unique_resource_handle _texture_weight_maps;
   gpu::unique_resource_handle _color_map;

   gpu_virtual_address _terrain_cbv;
   gpu::unique_resource_view _height_map_srv;
   gpu::unique_resource_view _texture_weight_maps_srv;
   gpu::unique_resource_view _color_map_srv;

   std::array<lowercase_string, texture_count> _diffuse_maps_names;
   std::array<std::shared_ptr<const world_texture>, texture_count> _diffuse_maps;

   uint32 _height_map_upload_row_pitch = 0;
   uint32 _weight_map_upload_row_pitch = 0;
   uint32 _color_map_upload_row_pitch = 0;

   gpu::unique_resource_handle _upload_buffer;
   std::array<std::byte*, gpu::frame_pipeline_length> _height_map_upload_ptr;
   std::array<uint32, gpu::frame_pipeline_length> _height_map_upload_offset;
   std::array<std::array<std::byte*, texture_count>, gpu::frame_pipeline_length> _weight_map_upload_ptr;
   std::array<std::array<uint32, texture_count>, gpu::frame_pipeline_length> _weight_map_upload_offset;
   std::array<std::byte*, gpu::frame_pipeline_length> _color_map_upload_ptr;
   std::array<uint32, gpu::frame_pipeline_length> _color_map_upload_offset;
};

}
