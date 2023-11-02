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

namespace we::graphics {

struct water {
   water(gpu::device& device, texture_manager& texture_manager);

   void update(const world::terrain& terrain, gpu::copy_command_list& command_list,
               dynamic_buffer_allocator& dynamic_buffer_allocator,
               texture_manager& texture_manager);

   void draw(const frustum& view_frustum, gpu_virtual_address camera_constant_buffer_view,
             gpu::graphics_command_list& command_list,
             dynamic_buffer_allocator& dynamic_buffer_allocator,
             root_signature_library& root_signatures, pipeline_library& pipelines);

   void process_updated_texture(const updated_textures& updated);

private:
   gpu::device& _device;

   bool _active = false;

   struct patch {
      float2 min;
      float2 max;
   };

   uint32 _water_map_length = 0;
   uint32 _water_map_row_words = 0;

   std::vector<uint64> _water_map;
   std::vector<patch> _patches;

   float _water_grid_scale = 0.0f;
   float _half_water_map_length = 0.0f;
   float _water_height = 0.0f;

   gpu::unique_resource_handle _water_constants_buffer;
   gpu_virtual_address _water_constants_cbv;

   lowercase_string _color_map_name;
   std::shared_ptr<const world_texture> _color_map;
};

}
