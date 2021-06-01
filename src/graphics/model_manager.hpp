#pragma once

#include "assets/asset_libraries.hpp"
#include "assets/msh/default_missing_scene.hpp"
#include "lowercase_string.hpp"
#include "model.hpp"
#include "texture_manager.hpp"

#include <memory>
#include <shared_mutex>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <gsl/gsl>
#include <tbb/task_group.h>

namespace sk::graphics {

class model_manager {
public:
   model_manager(gpu::device& gpu_device, texture_manager& texture_manager,
                 assets::library<assets::msh::flat_model>& model_assets)
      : _gpu_device{&gpu_device}, _model_assets{model_assets}, _texture_manager{texture_manager}
   {
      _placeholder_model.init_gpu_buffer_async(gpu_device);
   };

   ~model_manager()
   {
      _creation_tasks.cancel();
      _creation_tasks.wait();
   }

   /// @brief Gets a model.
   /// @param name The name of the model.
   /// @return A const reference to the model or the default model.
   auto operator[](const lowercase_string& name) -> const model&
   {
      if (name.empty()) return _placeholder_model;

      // Check to see if the model is already loaded and ready.
      {
         std::shared_lock lock{_mutex};

         if (auto state = _models.find(name); state != _models.end()) {
            return *state->second.model;
         }

         if (_pending_creations.contains(name)) return _placeholder_model;
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

      _pending_creations.insert(name);

      _creation_tasks.run([this, name = name, asset, flat_model] {
         auto new_model =
            std::make_unique<model>(*flat_model, *_gpu_device, _texture_manager);

         new_model->init_gpu_buffer_async(*_gpu_device);

         std::scoped_lock lock{_mutex};

         // Nake sure an asset load event hasn't loaded a new asset before us.
         // This stops us replacing a new asset with an out of date one.
         if (_pending_creations.erase(name) == 0) return;

         _models.emplace(name, model_state{.model = std::move(new_model),
                                           .asset = asset});
      });

      return _placeholder_model;
   }

   void process_updated_texture(gpu::device& gpu_device, updated_texture updated)
   {
      std::shared_lock lock{_mutex};

      for (auto& [name, state] : _models) {
         for (auto& part : state.model->parts) {
            part.material.process_updated_texture(gpu_device, updated);
         }
      }
   }

   /// @brief Call at the end of a frame to destroy models that may have been updated and replaced midframe.
   ///        Also destroys models for which model_manager is the only remaining reference for the asset.
   /// @return None.
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

   void model_loaded(const lowercase_string& name,
                     asset_ref<assets::msh::flat_model> asset,
                     asset_data<assets::msh::flat_model> data) noexcept
   {
      auto new_model = std::make_unique<model>(*data, *_gpu_device, _texture_manager);

      new_model->init_gpu_buffer_async(*_gpu_device);

      std::scoped_lock lock{_mutex};

      auto& state = _models[name];

      state.asset = asset;

      std::swap(state.model, _pending_destroys.emplace_back());
      std::swap(state.model, new_model);

      _pending_creations.erase(name);
   }

   gsl::not_null<gpu::device*> _gpu_device; // Do NOT change while _creation_tasks has active tasks queued or while _asset_load_listener is active.
   assets::library<assets::msh::flat_model>& _model_assets;
   texture_manager& _texture_manager;

   std::shared_mutex _mutex;

   absl::flat_hash_map<lowercase_string, model_state> _models;
   absl::flat_hash_set<lowercase_string> _pending_creations;
   std::vector<std::unique_ptr<model>> _pending_destroys;

   tbb::task_group _creation_tasks;

   model _placeholder_model{*assets::msh::default_missing_scene(), *_gpu_device,
                            _texture_manager};

   event_listener<void(const lowercase_string&, asset_ref<assets::msh::flat_model>,
                       asset_data<assets::msh::flat_model>)>
      _asset_load_listener = _model_assets.listen_for_loads(
         [this](const auto&... args) { model_loaded(args...); });
};

}
