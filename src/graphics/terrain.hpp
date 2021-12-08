#pragma once

#include "frustrum.hpp"
#include "gpu/command_list.hpp"
#include "gpu/device.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "texture_manager.hpp"
#include "types.hpp"
#include "world/world.hpp"

#include <bitset>
#include <vector>

#include <gsl/gsl>

namespace we::graphics {

enum class terrain_draw { depth_prepass, main };

class terrain {
public:
   constexpr static std::size_t texture_count = assets::terrain::terrain::texture_count;

   explicit terrain(gpu::device& device, texture_manager& texture_manager);

   void init(const world::terrain& terrain, gpu::graphics_command_list& command_list,
             gpu::dynamic_buffer_allocator& dynamic_buffer_allocator);

   void draw(const terrain_draw draw, const frustrum& view_frustrum,
             gpu::descriptor_range camera_constant_buffer_view,
             gpu::descriptor_range light_descriptors,
             gpu::graphics_command_list& command_list,
             root_signature_library& root_signatures, pipeline_library& pipelines,
             gpu::dynamic_buffer_allocator& dynamic_buffer_allocator);

   void process_updated_texture(const updated_textures& updated);

private:
   struct terrain_patch {
      math::bounding_box bbox;
      uint32 x;
      uint32 y;
      std::bitset<16> active_textures;
   };

   void init_gpu_resources(const world::terrain& terrain,
                           gpu::graphics_command_list& command_list,
                           gpu::dynamic_buffer_allocator& dynamic_buffer_allocator);

   void init_gpu_height_map(const world::terrain& terrain,
                            gpu::graphics_command_list& command_list);

   void init_gpu_texture_weight_map(const world::terrain& terrain,
                                    gpu::graphics_command_list& command_list);

   void init_gpu_terrain_constants_buffer(
      const world::terrain& terrain, gpu::graphics_command_list& command_list,
      gpu::dynamic_buffer_allocator& dynamic_buffer_allocator);

   void init_textures(const world::terrain& terrain);

   void init_textures_resource_views();

   void init_patches_info(const world::terrain& terrain);

   gsl::not_null<gpu::device*> _gpu_device;
   texture_manager& _texture_manager;

   bool _active = false;

   uint32 _terrain_length;
   uint32 _patch_count;

   float2 _terrain_half_world_size;
   float _terrain_grid_size;
   float _terrain_height_scale;

   std::vector<terrain_patch> _patches;

   gpu::buffer _index_buffer;
   gpu::buffer _terrain_constants_buffer;
   gpu::texture _height_map;
   gpu::texture _texture_weight_maps;

   gpu::resource_view_set _resource_views;
   gpu::resource_view_set _texture_resource_views;

   std::array<lowercase_string, texture_count> _diffuse_maps_names;
   std::array<std::shared_ptr<const world_texture>, texture_count> _diffuse_maps;
};

}
