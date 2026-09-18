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
#include "texture_manager.hpp"
#include "world_mesh_list.hpp"

#include "math/frustum.hpp"

#include "world/active_elements.hpp"
#include "world/object_class.hpp"
#include "world/world.hpp"

#include <memory>

namespace we::world {

struct light_class;

}

namespace we::graphics {

class light_clusters {
public:
   light_clusters(gpu::device& device, texture_manager& texture_manager,
                  copy_command_list_pool& copy_command_list_pool,
                  uint32 render_width, uint32 render_height);

   ~light_clusters();

   void update(const bool animate_lights);

   void update_render_resolution(uint32 width, uint32 height);

   void add_object_light(const float4x4& world_from_object,
                         const world::light_class& light_class);

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

   void process_updated_textures(const updated_textures& updated) noexcept;

private:
   struct impl;

   const std::unique_ptr<impl> _impl;
};

}
