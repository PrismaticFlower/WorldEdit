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
#include <vector>

#include <absl/container/flat_hash_map.h>

namespace we::graphics {

class light_clusters {
public:
   light_clusters(gpu::device& gpu_device);

   void update_lights(const frustrum& view_frustrum, const world::world& world,
                      gpu::command_list& command_list,
                      gpu::dynamic_buffer_allocator& dynamic_buffer_allocator);

   void TEMP_render_shadow_maps(
      const camera& view_camera, const frustrum& view_frustrum,
      const world_mesh_list& meshes, const world::world& world,
      root_signature_library& root_signatures, pipeline_library& pipelines,
      gpu::command_list& command_list,
      gpu::dynamic_buffer_allocator& dynamic_buffer_allocator);

   auto light_descriptors() const noexcept -> gpu::descriptor_range;

private:
   gsl::not_null<gpu::device*> _gpu_device;

   gpu::buffer _lights_constant_buffer;
   gpu::buffer _regional_lights_buffer;

   gpu::texture _shadow_map;
   gpu::descriptor_allocation _shadow_map_dsv;
   std::array<float4x4, 4> _shadow_cascade_transforms;

   gpu::resource_view_set _resource_views;
};

}
