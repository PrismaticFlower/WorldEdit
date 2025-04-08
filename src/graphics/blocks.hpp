#pragma once

#include "copy_command_list_pool.hpp"
#include "dynamic_buffer_allocator.hpp"
#include "geometric_shapes.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"
#include "math/frustum.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "texture_manager.hpp"
#include "types.hpp"
#include "world/world.hpp"

#include <vector>

namespace we::graphics {

enum class blocks_draw { depth_prepass, main, shadow };

struct blocks {
   struct view {
      uint32 box_instances_count = 0;
      gpu_virtual_address box_instances = 0;
   };

   blocks(gpu::device& device, copy_command_list_pool& copy_command_list_pool,
          dynamic_buffer_allocator& dynamic_buffer_allocator,
          texture_manager& texture_manager);

   void update(const world::blocks& blocks, gpu::copy_command_list& command_list,
               dynamic_buffer_allocator& dynamic_buffer_allocator,
               texture_manager& texture_manager);

   auto prepare_view(blocks_draw draw, const world::blocks& blocks,
                     const frustum& view_frustum,
                     dynamic_buffer_allocator& dynamic_buffer_allocator) -> view;

   void draw(blocks_draw draw, const view& view,
             gpu_virtual_address frame_constant_buffer_view,
             gpu_virtual_address lights_constant_buffer_view,
             gpu::graphics_command_list& command_list,
             root_signature_library& root_signatures, pipeline_library& pipelines) const;

   /// @brief Process updated material textures.
   /// @param command_list The copy command list to use to update the constant buffer.
   /// @param updated The updated textures.
   void process_updated_textures(gpu::copy_command_list& command_list,
                                 const updated_textures& updated);

   /// @brief Process updated material textures using copy_buffer_region instead of write_buffer_immediate.
   /// @param allocator The dynamic buffer allocator.
   /// @param mesh_buffer The handle to the mesh buffer containing the material.
   /// @param command_list The copy command list to use to update the constant buffer.
   /// @param updated The updated textures.
   void process_updated_textures_copy(dynamic_buffer_allocator& allocator,
                                      gpu::copy_command_list& command_list,
                                      const updated_textures& updated);

   struct material {
      struct texture {
         lowercase_string name;
         std::shared_ptr<const world_texture> texture;
         std::shared_ptr<const world_texture_load_token> load_token;

         void update(const std::string_view new_name, texture_manager& texture_manager,
                     const std::shared_ptr<const world_texture>& default_texture);
      };

      texture diffuse_map;
      texture normal_map;
      texture detail_map;
      texture env_map;

      std::array<uint8, 2> detail_tiling = {0, 0};
      bool tile_normal_map = false;
      bool specular_lighting = false;

      float3 specular_color;
   };

private:
   gpu::device& _device;

   gpu::unique_resource_handle _blocks_ia_buffer;
   gpu::unique_resource_handle _blocks_materials_buffer;

   gpu::unique_resource_handle _boxes_instance_data;
   uint64 _boxes_instance_data_capacity = 0;

   std::vector<uint32> _TEMP_culling_storage;

   std::vector<material> _materials;
};

}
