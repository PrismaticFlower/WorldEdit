#pragma once

#include "assets/asset_libraries.hpp"
#include "async/thread_pool.hpp"
#include "copy_command_list_pool.hpp"
#include "gpu/rhi.hpp"
#include "model.hpp"
#include "output_stream.hpp"
#include "texture_manager.hpp"
#include "utility/function_ptr.hpp"
#include "utility/implementation_storage.hpp"

namespace we::graphics {

enum class model_status {
   ready,
   ready_textures_missing,
   ready_textures_loading,
   loading,
   errored,
   missing
};

struct model_manager {
   model_manager(gpu::device& gpu_device, copy_command_list_pool& copy_command_list_pool,
                 texture_manager& texture_manager,
                 assets::library<assets::msh::flat_model>& model_assets,
                 std::shared_ptr<async::thread_pool> thread_pool,
                 output_stream& error_output);

   model_manager(const model_manager&) = delete;
   model_manager(model_manager&&) = delete;

   ~model_manager();

   /// @brief Gets a model.
   /// @param name The name of the model.
   /// @return A reference to the model or the default model.
   auto operator[](const lowercase_string& name) -> model&;

   /// @brief Permits safe iteration over active models.
   /// @param callback The function to call for each model.
   void for_each(function_ptr<void(model&) noexcept> callback) noexcept;

   /// @brief Call at the start of a frame to update models that have been created asynchronously.
   void update_models() noexcept;

   /// @brief Call at the end of a frame to destroy models that went unused across the frame.
   void trim_models() noexcept;

   /// @brief Checks if model is the placeholder model.
   /// @param model
   /// @return If the model is the placeholder or not.
   bool is_placeholder(const model& model) const noexcept;

   /// @brief Query the status of a model.
   /// @param name The name of the model to query the status of.
   /// @return The status of the model.
   auto status(const lowercase_string& name) const noexcept -> model_status;

private:
   struct impl;

   implementation_storage<impl, 3176> _impl;
};

}
