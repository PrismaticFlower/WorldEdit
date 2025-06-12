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

#include "allocators/aligned_allocator.hpp"
#include "allocators/offset_allocator.hpp"

#include "world/active_elements.hpp"
#include "world/blocks.hpp"
#include "world/entity_group.hpp"

#include <memory>
#include <vector>

namespace we::graphics {

enum class blocks_draw { depth_prepass, main, shadow };

struct blocks {
   struct view {
      struct instance_list {
         uint32 count = 0;
         gpu_virtual_address instances = 0;
      };

      struct draw_list {
         uint32 count = 0;
         gpu::resource_handle draw_buffer = gpu::null_resource_handle;
         uint64 draw_buffer_offset = 0;
      };

      instance_list boxes;
      instance_list ramps;
      instance_list quads;
      instance_list cylinders;
      instance_list cones;
      instance_list hemispheres;
      instance_list pyramids;
      draw_list custom;

      instance_list dynamic_boxes;
      instance_list dynamic_ramps;
      instance_list dynamic_quads;
      instance_list dynamic_cylinders;
      instance_list dynamic_cones;
      instance_list dynamic_hemispheres;
      instance_list dynamic_pyramids;
      draw_list dynamic_custom;
   };

   blocks(gpu::device& device, copy_command_list_pool& copy_command_list_pool,
          dynamic_buffer_allocator& dynamic_buffer_allocator,
          texture_manager& texture_manager, root_signature_library& root_signatures);

   void update(const world::blocks& blocks, const world::entity_group* entity_group,
               gpu::copy_command_list& command_list,
               dynamic_buffer_allocator& dynamic_buffer_allocator,
               texture_manager& texture_manager);

   auto prepare_view(blocks_draw draw, const world::blocks& blocks,
                     const world::entity_group* entity_group, const frustum& view_frustum,
                     const world::active_layers active_layers,
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

   struct custom_mesh {
      offset_allocator_aligned::allocation ia_allocation;

      uint32 index_count = 0;
      uint32 vertex_count = 0;

      uint32 start_index_location = 0;
      int32 base_vertex_location = 0;
   };

private:
   gpu::device& _device;

   gpu::unique_resource_handle _blocks_ia_buffer;
   gpu::unique_resource_handle _blocks_materials_buffer;

   gpu::unique_resource_handle _boxes_instance_data;
   uint64 _boxes_instance_data_capacity = 0;

   gpu::unique_resource_handle _ramps_instance_data;
   uint64 _ramps_instance_data_capacity = 0;

   gpu::unique_resource_handle _quads_instance_data;
   uint64 _quads_instance_data_capacity = 0;

   gpu::unique_resource_handle _cylinders_instance_data;
   uint64 _cylinders_instance_data_capacity = 0;

   gpu::unique_resource_handle _custom_instance_data;
   uint64 _custom_instance_data_capacity = 0;

   gpu::unique_resource_handle _cones_instance_data;
   uint64 _cones_instance_data_capacity = 0;

   gpu::unique_resource_handle _hemispheres_instance_data;
   uint64 _hemispheres_instance_data_capacity = 0;

   gpu::unique_resource_handle _pyramids_instance_data;
   uint64 _pyramids_instance_data_capacity = 0;

   gpu::unique_command_signature_handle _custom_mesh_command_signature;

   offset_allocator_aligned _custom_mesh_allocator;
   gpu::unique_resource_handle _custom_mesh_buffer;
   uint32 _custom_mesh_buffer_capacity = 0;

   bool _initialized_custom_meshes = false;
   std::vector<custom_mesh> _custom_meshes;

   std::vector<uint32> _TEMP_culling_storage;

   std::vector<material> _materials;

   struct dynamic_blocks {
      struct bbox_soa {
         using float_soa_vector = std::vector<float, aligned_allocator<float, 32>>;

         float_soa_vector min_x;
         float_soa_vector min_y;
         float_soa_vector min_z;
         float_soa_vector max_x;
         float_soa_vector max_y;
         float_soa_vector max_z;

         auto size() const noexcept -> std::size_t;

         void push_back(const math::bounding_box& bbox) noexcept;

         void reserve(std::size_t size) noexcept;

         void clear() noexcept;
      };

      gpu::unique_resource_handle materials_buffer;

      gpu::unique_resource_handle boxes_instance_data;
      uint64 boxes_instance_data_capacity = 0;

      gpu::unique_resource_handle ramps_instance_data;
      uint64 ramps_instance_data_capacity = 0;

      gpu::unique_resource_handle quads_instance_data;
      uint64 quads_instance_data_capacity = 0;

      gpu::unique_resource_handle cylinders_instance_data;
      uint64 cylinders_instance_data_capacity = 0;

      gpu::unique_resource_handle custom_instance_data;
      uint64 custom_instance_data_capacity = 0;

      gpu::unique_resource_handle cones_instance_data;
      uint64 cones_instance_data_capacity = 0;

      gpu::unique_resource_handle hemispheres_instance_data;
      uint64 hemispheres_instance_data_capacity = 0;

      gpu::unique_resource_handle pyramids_instance_data;
      uint64 pyramids_instance_data_capacity = 0;

      bbox_soa boxes_bbox;
      bbox_soa ramps_bbox;
      bbox_soa quads_bbox;
      bbox_soa cylinders_bbox;
      bbox_soa custom_bbox;
      bbox_soa cones_bbox;
      bbox_soa hemispheres_bbox;
      bbox_soa pyramids_bbox;

      std::array<material, world::max_block_materials> materials;

      void update(const world::entity_group& entity_group, gpu::device& device,
                  gpu::copy_command_list& command_list,
                  dynamic_buffer_allocator& dynamic_buffer_allocator,
                  texture_manager& texture_manager);

      void process_updated_textures(const updated_textures& updated);
   };

   std::unique_ptr<dynamic_blocks> _dynamic_blocks;
};

}
