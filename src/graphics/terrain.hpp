#pragma once

#include "frustrum.hpp"
#include "gpu/command_list.hpp"
#include "gpu/device.hpp"
#include "types.hpp"
#include "world/world.hpp"

#include <vector>

#include <gsl/gsl>

namespace sk::graphics {

class terrain {
public:
   explicit terrain(gpu::device& device);

   void init(const world::terrain& terrain, gpu::command_list& command_list,
             gpu::dynamic_buffer_allocator& dynamic_buffer_allocator);

   void draw(const frustrum& view_frustrum,
             gpu::descriptor_range camera_constant_buffer_view,
             gpu::command_list& command_list,
             gpu::dynamic_buffer_allocator& dynamic_buffer_allocator);

private:
   struct terrain_patch {
      math::bounding_box bbox;
      uint32 x;
      uint32 y;
   };

   void init_gpu_resources(const world::terrain& terrain, gpu::command_list& command_list,
                           gpu::dynamic_buffer_allocator& dynamic_buffer_allocator);

   void init_patches_info(const world::terrain& terrain);

   gsl::not_null<gpu::device*> _gpu_device;

   uint32 _terrain_length;
   uint32 _patch_count;

   float2 _terrain_half_world_size;
   float _terrain_grid_size;
   float _terrain_height_scale;

   std::vector<terrain_patch> _patches;

   gpu::buffer _index_buffer;
   gpu::texture _height_map;

   gpu::resource_view_set _resource_views;
};

}
