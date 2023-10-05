
#include "model_manager.hpp"

#include "assets/msh/default_missing_scene.hpp"
#include "lowercase_string.hpp"
#include "utility/string_ops.hpp"

#include <fmt/core.h>

namespace we::graphics {

model_manager::model_manager(gpu::device& gpu_device,
                             copy_command_list_pool& copy_command_list_pool,
                             texture_manager& texture_manager,
                             assets::library<assets::msh::flat_model>& model_assets,
                             std::shared_ptr<async::thread_pool> thread_pool,
                             output_stream& error_output)
   : _gpu_device{gpu_device},
     _copy_command_list_pool{copy_command_list_pool},
     _model_assets{model_assets},
     _texture_manager{texture_manager},
     _thread_pool{thread_pool},
     _placeholder_model{*assets::msh::default_missing_scene(), _gpu_device,
                        copy_command_list_pool, _texture_manager},
     _error_output{error_output}
{
}

auto model_manager::operator[](const lowercase_string& name) -> model&
{
   if (name.empty()) return _placeholder_model;

   // Check to see if the model is already loaded and ready.
   {
      std::shared_lock lock{_mutex};

      if (auto it = _models.find(name); it != _models.end()) {
         auto& [_, model_state] = *it;

         std::atomic_ref<bool>{model_state.unused}.store(false, std::memory_order_relaxed);

         return *model_state.model;
      }

      if (_pending_creations.contains(name) or _pending_loads.contains(name) or
          _failed_creations.contains(name)) {
         return _placeholder_model;
      }
   }

   auto asset = _model_assets[name];

   if (not asset.exists()) return _placeholder_model;

   std::scoped_lock lock{_mutex};

   // Make sure another thread hasn't created the model inbetween us checking
   // for it and taking the write lock.
   if (_pending_creations.contains(name) or _pending_loads.contains(name)) {
      return _placeholder_model;
   }

   if (auto it = _models.find(name); it != _models.end()) {
      auto& [_, model_state] = *it;

      model_state.unused = false;

      return *model_state.model;
   }

   auto flat_model = asset.get_if();

   if (flat_model == nullptr) {
      _pending_loads.emplace(name, asset);

      return _placeholder_model;
   }

   enqueue_create_model(name, asset, flat_model);

   return _placeholder_model;
}

void model_manager::update_models() noexcept
{
   std::scoped_lock lock{_mutex};

   for (auto it = _pending_creations.begin(); it != _pending_creations.end();) {
      auto elem_it = it++;
      auto& [name, pending_create] = *elem_it;

      if (pending_create.task.ready()) {
         try {
            _models[name] = pending_create.task.get();
            _model_asset_refs[name] = pending_create.model_asset_ref;
         }
         catch (std::exception& e) {
            _error_output.write(
               "Failed to create model:\n   Name: {}\n   Message: \n{}\n",
               std::string_view{name}, string::indent(2, e.what()));

            _failed_creations.insert(name);
         }

         _pending_creations.erase(elem_it);
      }
   }
}

void model_manager::trim_models() noexcept
{
   std::lock_guard lock{_mutex};

   for (auto it = _models.begin(); it != _models.end();) {
      auto elem_it = it++;
      auto& [name, model_state] = *elem_it;

      if (std::exchange(model_state.unused, false)) {
         _model_asset_refs.erase(name);
         _models.erase(elem_it);
      }
   }
}

bool model_manager::is_placeholder(const model& model) const noexcept
{
   return &model == &_placeholder_model;
}

auto model_manager::status(const lowercase_string& name) const noexcept -> model_status
{
   std::shared_lock lock{_mutex};

   if (auto it = _models.find(name); it != _models.end()) {
      const model& model = *it->second.model;

      bool missing_textures = false;

      for (const mesh_part& part : model.parts) {
         switch (part.material.status(_texture_manager)) {
         case material_status::ready:
            break; // Nothing to do but handle the case anyway.
         case material_status::ready_textures_missing:
            missing_textures = true;
            break;
         case material_status::ready_textures_loading:
            return model_status::ready_textures_loading;
         }
      }

      if (missing_textures) return model_status::ready_textures_missing;

      return model_status::ready;
   }

   if (_pending_creations.contains(name)) return model_status::loading;

   if (_pending_loads.contains(name)) return model_status::loading;

   if (_failed_creations.contains(name)) return model_status::errored;

   return model_status::missing;
}

void model_manager::model_loaded(const lowercase_string& name,
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

void model_manager::model_load_failed(const lowercase_string& name,
                                      asset_ref<assets::msh::flat_model> asset) noexcept
{
   std::scoped_lock lock{_mutex};

   _pending_loads.erase(name);
   _failed_creations.emplace(name);
}

void model_manager::enqueue_create_model(const lowercase_string& name,
                                         asset_ref<assets::msh::flat_model> asset,
                                         asset_data<assets::msh::flat_model> flat_model) noexcept
{
   _pending_loads.erase(name);

   _pending_creations[name] =
      {.task =
          _thread_pool->exec(async::task_priority::low,
                             [this, name = name, asset, flat_model]() -> model_state {
                                auto new_model =
                                   std::make_unique<model>(*flat_model, _gpu_device,
                                                           _copy_command_list_pool,
                                                           _texture_manager);

                                _gpu_device.background_copy_queue.wait_for_idle();

                                std::shared_lock lock{_mutex};

                                // Make sure an asset load event hasn't loaded a new asset before us.
                                // This stops us replacing a new asset with an out of date one.
                                if (not _pending_creations.contains(name) or
                                    _pending_creations[name].flat_model != flat_model) {
                                   return {};
                                }

                                return {.model = std::move(new_model)};
                             }),
       .flat_model = flat_model,
       .model_asset_ref = asset};
}

}
