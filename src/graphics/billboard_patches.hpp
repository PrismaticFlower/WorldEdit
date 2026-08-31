#pragma once

#include "dynamic_buffer_allocator.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "texture_manager.hpp"

#include "gpu/rhi.hpp"

#include "types.hpp"

#include "math/frustum.hpp"

#include "world/world.hpp"

#include "utility/implementation_storage.hpp"

#include <span>

namespace we::world {

struct billboard_patch_class;

}

namespace we::graphics {

enum class billboard_patches_draw {
   depth_prepass,
   main_opaque,
   main_transparent,
   shadow,
};

enum class billboard_patches_prepare {
   main,
   shadow,
};

struct billboard_patches {
   struct view {
      struct instances {
         uint32 count = 0;
         gpu_virtual_address world_from_object = 0;
      };

      std::span<instances> opaque;
      std::span<instances> transparent;
   };

   billboard_patches(gpu::device& device, texture_manager& texture_manager);

   billboard_patches(const billboard_patches&) = delete;
   billboard_patches(billboard_patches&&) = delete;

   ~billboard_patches();

   void update(const float4x4& world_matrix, const world::world& world,
               const bool animated);

   void update_index_buffer(gpu::copy_command_list& command_list,
                            dynamic_buffer_allocator& allocator);

   /// @brief Dangerously exciting function. Uses cached world::billboard_patch_class pointers to update the world_matrix for patches in the same frame. Should **ONLY** be called from the draw_env_map function
   void update_view_same_frame(const float4x4& world_matrix,
                               dynamic_buffer_allocator& allocator);

   void add_billboard_patch(const world::billboard_patch_class& billboard_patch_class,
                            const float4x4& world_from_object,
                            dynamic_buffer_allocator& allocator);

   auto prepare_view(billboard_patches_prepare prepare, const frustum& view_frustum,
                     dynamic_buffer_allocator& allocator) -> view;

   void draw(billboard_patches_draw draw, const view& view,
             gpu_virtual_address frame_constant_buffer_view,
             gpu_virtual_address lights_constant_buffer_view,
             gpu::graphics_command_list& command_list,
             root_signature_library& root_signatures, pipeline_library& pipelines) const;

   void process_updated_textures(const updated_textures& updated);

private:
   struct impl;

   implementation_storage<impl, 1024> _impl;
};

}