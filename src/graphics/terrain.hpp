#pragma once

#include "dynamic_buffer_allocator.hpp"
#include "frustum.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "texture_manager.hpp"
#include "types.hpp"
#include "world/world.hpp"

#include <bitset>
#include <vector>

namespace we::graphics {

enum class terrain_draw { depth_prepass, main };

class terrain {
public:
   constexpr static std::size_t texture_count = assets::terrain::terrain::texture_count;

   explicit terrain(gpu::device& device, texture_manager& texture_manager);

   void init(const world::terrain& terrain, gpu::copy_command_list& command_list,
             dynamic_buffer_allocator& dynamic_buffer_allocator);

   void draw(const terrain_draw draw, const frustum& view_frustum,
             gpu_virtual_address camera_constant_buffer_view,
             gpu_virtual_address lights_constant_buffer_view,
             gpu::graphics_command_list& command_list,
             root_signature_library& root_signatures, pipeline_library& pipelines,
             dynamic_buffer_allocator& dynamic_buffer_allocator);

   void process_updated_texture(gpu::copy_command_list& command_list,
                                const updated_textures& updated);

private:
   struct terrain_patch {
      math::bounding_box bbox;
      uint32 x;
      uint32 y;
      std::bitset<16> active_textures;
   };

   void init_gpu_resources(const world::terrain& terrain,
                           gpu::copy_command_list& command_list,
                           dynamic_buffer_allocator& dynamic_buffer_allocator);

   void init_gpu_height_map(const world::terrain& terrain,
                            gpu::copy_command_list& command_list);

   void init_gpu_texture_weight_map(const world::terrain& terrain,
                                    gpu::copy_command_list& command_list);

   void init_gpu_terrain_constants_buffer(const world::terrain& terrain,
                                          gpu::copy_command_list& command_list,
                                          dynamic_buffer_allocator& dynamic_buffer_allocator);

   void init_textures(const world::terrain& terrain);

   void init_patches_info(const world::terrain& terrain);

   gpu::device& _device;
   texture_manager& _texture_manager;

   bool _active = false;

   uint32 _terrain_length;
   uint32 _patch_count;

   float2 _terrain_half_world_size;
   float _terrain_grid_size;
   float _terrain_height_scale;

   std::vector<terrain_patch> _patches;

   gpu::unique_resource_handle _index_buffer;
   gpu::unique_resource_handle _terrain_constants_buffer;
   gpu::unique_resource_handle _height_map;
   gpu::unique_resource_handle _texture_weight_maps;

   gpu_virtual_address _terrain_cbv;
   gpu::unique_resource_view _height_map_srv;
   gpu::unique_resource_view _texture_weight_maps_srv;

   std::array<lowercase_string, texture_count> _diffuse_maps_names;
   std::array<std::shared_ptr<const world_texture>, texture_count> _diffuse_maps;

   gpu::unique_resource_handle _height_map_upload_buffer;
   gpu::unique_resource_handle _texture_weight_upload_buffer;
};

}
