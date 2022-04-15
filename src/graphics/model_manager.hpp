#pragma once

#include "assets/asset_libraries.hpp"
#include "assets/msh/default_missing_scene.hpp"
#include "async/thread_pool.hpp"
#include "lowercase_string.hpp"
#include "model.hpp"
#include "output_stream.hpp"
#include "texture_manager.hpp"
#include "utility/string_ops.hpp"

#include <memory>
#include <shared_mutex>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <fmt/format.h>
#include <gsl/gsl>

namespace we::graphics {

class model_manager {
public:
   model_manager(gpu::device& gpu_device, texture_manager& texture_manager,
                 assets::library<assets::msh::flat_model>& model_assets,
                 std::shared_ptr<async::thread_pool> thread_pool,
                 output_stream& error_output)
      : _gpu_device{&gpu_device},
        _model_assets{model_assets},
        _texture_manager{texture_manager},
        _thread_pool{thread_pool},
        _error_output{error_output}
   {
   }

   /// @brief Gets a model.
   /// @param name The name of the model.
   /// @return A reference to the model or the default model.
   auto operator[](const lowercase_string& name) -> model&
   {
      if (name.empty()) return _placeholder_model;

      // Check to see if the model is already loaded and ready.
      {
         std::shared_lock lock{_mutex};

         if (auto state = _models.find(name); state != _models.end()) {
            return *state->second.model;
         }

         if (_pending_creations.contains(name) or _failed_creations.contains(name)) {
            return _placeholder_model;
         }
      }

      auto asset = _model_assets[name];

      if (not asset.exists()) return _placeholder_model;

      std::scoped_lock lock{_mutex};

      // Make sure another thread hasn't created the model inbetween us checking
      // for it and taking the write lock.
      if (_pending_creations.contains(name)) return _placeholder_model;

      if (auto state = _models.find(name); state != _models.end()) {
         return *state->second.model;
      }

      auto flat_model = asset.get_if();

      if (flat_model == nullptr) return _placeholder_model;

      enqueue_create_model(name, asset, flat_model);

      return _placeholder_model;
   }

   /// @brief Permits safe iteration over active models.
   /// @param callback The function to call for each model.
   void for_each(std::invocable<model&> auto callback)
   {
      std::shared_lock lock{_mutex};

      for (auto& [name, state] : _models) callback(*state.model);
   }

   /// @brief Call at the start of a frame to update models that have been created asynchronously.
   void update_models() noexcept
   {
      std::scoped_lock lock{_mutex};

      for (auto it = _pending_creations.begin(); it != _pending_creations.end();) {
         auto elem_it = it++;
         auto& [name, pending_create] = *elem_it;

         if (pending_create.task.ready()) {
            try {
               // TODO: Currently we don't have to worry about model lifetime here but if in the future we're rendering
               // thumbnails in the background we may have to revist this.
               _models[name] = pending_create.task.get();
            }
            catch (std::exception& e) {
               _error_output.write(fmt::format(
                  "Failed to create model:\n   Name: {}\n   Message: \n{}\n",
                  std::string_view{name}, utility::string::indent(2, e.what())));

               _failed_creations.insert(name);
            }

            _pending_creations.erase(elem_it);
         }
      }
   }

   /// @brief Call at the end of a frame to destroy models that may have been updated and replaced midframe.
   ///        Also destroys models for which model_manager is the only remaining reference for the asset.
   void trim_models() noexcept
   {
      std::lock_guard lock{_mutex};

      erase_if(_models, [](const auto& key_value) {
         const auto& [name, state] = key_value;

         return state.asset.use_count() == 1;
      });

      _pending_destroys.clear();
   }

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
                     asset_data<assets::msh::flat_model> data) noexcept
   {
      std::scoped_lock lock{_mutex};

      if (_pending_creations.contains(name)) {
         _pending_creations[name].task.cancel();
      }

      _failed_creations.erase(name);

      enqueue_create_model(name, asset, data);
   }

   /// @brief Creates a model asynchronously. _mutex **MUST** be held with exclusive ownership before calling this funciton.
   void enqueue_create_model(const lowercase_string& name,
                             asset_ref<assets::msh::flat_model> asset,
                             asset_data<assets::msh::flat_model> flat_model) noexcept
   {
      _pending_creations[name] =
         {.task =
             _thread_pool->exec(async::task_priority::low,
                                [this, name = name, asset, flat_model]() -> model_state {
                                   auto new_model =
                                      std::make_unique<model>(*flat_model, *_gpu_device,
                                                              _texture_manager);

                                   std::shared_lock lock{_mutex};

                                   // Make sure an asset load event hasn't loaded a new asset before us.
                                   // This stops us replacing a new asset with an out of date one.
                                   if (not _pending_creations.contains(name) or
                                       _pending_creations[name].flat_model != flat_model) {
                                      return {};
                                   }

                                   return {.model = std::move(new_model),
                                           .asset = asset};
                                }),
          .flat_model = flat_model};
   }

   gsl::not_null<gpu::device*> _gpu_device;
   assets::library<assets::msh::flat_model>& _model_assets;
   texture_manager& _texture_manager;
   output_stream& _error_output;

   std::shared_mutex _mutex;

   absl::flat_hash_map<lowercase_string, model_state> _models;
   absl::flat_hash_map<lowercase_string, pending_create_model> _pending_creations;
   absl::flat_hash_set<lowercase_string> _failed_creations;
   std::vector<std::unique_ptr<model>> _pending_destroys;

   std::shared_ptr<async::thread_pool> _thread_pool;

   model _placeholder_model{*assets::msh::default_missing_scene(), *_gpu_device,
                            _texture_manager};

   event_listener<void(const lowercase_string&, asset_ref<assets::msh::flat_model>,
                       asset_data<assets::msh::flat_model>)>
      _asset_load_listener = _model_assets.listen_for_loads(
         [this](const auto&... args) { model_loaded(args...); });
};

}
