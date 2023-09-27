#pragma once

#include "assets/asset_libraries.hpp"
#include "async/thread_pool.hpp"
#include "copy_command_list_pool.hpp"
#include "gpu/rhi.hpp"
#include "model.hpp"
#include "output_stream.hpp"
#include "texture_manager.hpp"
#include "utility/event_listener.hpp"

#include <memory>
#include <shared_mutex>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

namespace we::graphics {

enum class model_status {
   ready,
   ready_textures_missing,
   ready_textures_loading,
   loading,
   errored,
   missing
};

class model_manager {
public:
   model_manager(gpu::device& gpu_device, copy_command_list_pool& copy_command_list_pool,
                 texture_manager& texture_manager,
                 assets::library<assets::msh::flat_model>& model_assets,
                 std::shared_ptr<async::thread_pool> thread_pool,
                 output_stream& error_output);

   /// @brief Gets a model.
   /// @param name The name of the model.
   /// @return A reference to the model or the default model.
   auto operator[](const lowercase_string& name) -> model&;

   /// @brief Permits safe iteration over active models.
   /// @param callback The function to call for each model.
   void for_each(std::invocable<model&> auto callback)
   {
      std::shared_lock lock{_mutex};

      for (auto& [name, state] : _models) callback(*state.model);
   }

   /// @brief Call at the start of a frame to update models that have been created asynchronously.
   void update_models() noexcept;

   /// @brief Call at the end of a frame to destroy models that may have been updated and replaced midframe.
   ///        Also destroys models for which model_manager is the only remaining reference for the asset.
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
   struct model_state {
      std::unique_ptr<model> model;
      asset_ref<assets::msh::flat_model> asset;
   };

   struct pending_create_model {
      async::task<model_state> task;
      asset_data<assets::msh::flat_model> flat_model;
   };

   void model_loaded(const lowercase_string& name,
                     asset_ref<assets::msh::flat_model> asset,
                     asset_data<assets::msh::flat_model> data) noexcept;

   void model_load_failed(const lowercase_string& name,
                          asset_ref<assets::msh::flat_model> asset) noexcept;

   /// @brief Creates a model asynchronously. _mutex **MUST** be held with exclusive ownership before calling this funciton.
   void enqueue_create_model(const lowercase_string& name,
                             asset_ref<assets::msh::flat_model> asset,
                             asset_data<assets::msh::flat_model> flat_model) noexcept;

   gpu::device& _gpu_device;
   copy_command_list_pool& _copy_command_list_pool;
   assets::library<assets::msh::flat_model>& _model_assets;
   texture_manager& _texture_manager;
   output_stream& _error_output;

   mutable std::shared_mutex _mutex;

   absl::flat_hash_map<lowercase_string, model_state> _models;
   absl::flat_hash_map<lowercase_string, pending_create_model> _pending_creations;
   absl::flat_hash_map<lowercase_string, asset_ref<assets::msh::flat_model>> _pending_loads;
   absl::flat_hash_set<lowercase_string> _failed_creations;
   std::vector<std::unique_ptr<model>> _pending_destroys;

   std::shared_ptr<async::thread_pool> _thread_pool;

   model _placeholder_model;

   event_listener<void(const lowercase_string&, asset_ref<assets::msh::flat_model>,
                       asset_data<assets::msh::flat_model>)>
      _asset_load_listener = _model_assets.listen_for_loads(
         [this](const auto&... args) { model_loaded(args...); });

   event_listener<void(const lowercase_string&, asset_ref<assets::msh::flat_model>)> _asset_load_failure_listener =
      _model_assets.listen_for_load_failures(
         [this](const auto&... args) { model_load_failed(args...); });
};

}
