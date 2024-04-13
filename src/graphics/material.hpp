#pragma once

#include "assets/msh/material.hpp"
#include "copy_command_list_pool.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"
#include "pipeline_library.hpp"
#include "texture_manager.hpp"

namespace we::graphics {

enum class material_status {
   ready,
   ready_textures_missing,
   ready_textures_loading
};

struct material {
   material(const assets::msh::material& material, const bool static_lighting,
            std::span<std::byte> constant_buffer_upload_memory,
            gpu_virtual_address constant_buffer_view,
            texture_manager& texture_manager);

   /// @brief Process updated material textures.
   /// @param command_list The copy command list to use to update the constant buffer.
   /// @param updated The updated textures.
   /// @param device The GPU device.
   void process_updated_textures(gpu::copy_command_list& command_list,
                                 const updated_textures& updated);

   /// @brief Query the status of the material's textures.
   /// @param texture_manager The texture manager.
   /// @return The status of the material's textures.
   auto status(const texture_manager& texture_manager) const noexcept -> material_status;

   /// @brief Get the pipeline flags for using this material with the thumbnail mesh renderer.
   /// @return The flags.
   auto thumbnail_mesh_flags() const noexcept -> thumbnail_mesh_pipeline_flags;

   /// @brief The size of the material constant buffer.
   const static std::size_t constant_buffer_size;

   depth_prepass_pipeline_flags depth_prepass_flags =
      depth_prepass_pipeline_flags::none;
   material_pipeline_flags flags = material_pipeline_flags::none;
   gpu_virtual_address constant_buffer_view;

   struct textures {
      std::shared_ptr<const world_texture> diffuse_map;
      std::shared_ptr<const world_texture> normal_map;
      std::shared_ptr<const world_texture> detail_map;
      std::shared_ptr<const world_texture> env_map;
   };

   struct texture_names {
      lowercase_string diffuse_map;
      lowercase_string normal_map;
      lowercase_string detail_map;
      lowercase_string env_map;
   };

   struct texture_load_tokens {
      std::shared_ptr<const world_texture_load_token> diffuse_map;
      std::shared_ptr<const world_texture_load_token> normal_map;
      std::shared_ptr<const world_texture_load_token> detail_map;
      std::shared_ptr<const world_texture_load_token> env_map;
   };

   gpu::resource_handle constant_buffer;

   textures textures;
   texture_names texture_names;
   texture_load_tokens texture_load_tokens;

private:
   void init_textures(const assets::msh::material& material,
                      texture_manager& texture_manager);

   void init_flags(const assets::msh::material& material);

   void init_constant_buffer(const assets::msh::material& material,
                             const bool static_lighting,
                             std::span<std::byte> constant_buffer_upload_memory);
};

}
