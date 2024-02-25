
#include "model_manager.hpp"

#include "assets/msh/default_missing_scene.hpp"
#include "lowercase_string.hpp"
#include "utility/event_listener.hpp"
#include "utility/string_ops.hpp"

#include <memory>
#include <shared_mutex>
#include <vector>

#include <fmt/core.h>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

namespace we::graphics {

struct model_manager::impl {
   impl(gpu::device& gpu_device, copy_command_list_pool& copy_command_list_pool,
        texture_manager& texture_manager,
        assets::library<assets::msh::flat_model>& model_assets,
        std::shared_ptr<async::thread_pool> thread_pool, output_stream& error_output)
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

   auto operator[](const lowercase_string& name) -> model&
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

         if (_pending_creations.contains(name) or
             _pending_loads.contains(name) or _failed_creations.contains(name)) {
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

   void for_each(function_ptr<void(model&) noexcept> callback) noexcept
   {
      std::shared_lock lock{_mutex};

      for (auto& [name, state] : _models) callback(*state.model);
   }

   void update_models() noexcept
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

   void trim_models() noexcept
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

   bool is_placeholder(const model& model) const noexcept
   {
      return &model == &_placeholder_model;
   }

   auto status(const lowercase_string& name) const noexcept -> model_status
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

private:
   struct model_state {
      std::unique_ptr<model> model;
      bool unused = true;
   };

   struct pending_create_model {
      async::task<model_state> task;
      asset_data<assets::msh::flat_model> flat_model;
      asset_ref<assets::msh::flat_model> model_asset_ref;
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

   void model_load_failed(const lowercase_string& name,
                          asset_ref<assets::msh::flat_model> asset) noexcept
   {
      std::scoped_lock lock{_mutex};

      _pending_loads.erase(name);
      _failed_creations.emplace(name);
   }

   /// @brief Creates a model asynchronously. _mutex **MUST** be held with exclusive ownership before calling this funciton.
   void enqueue_create_model(const lowercase_string& name,
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

   gpu::device& _gpu_device;
   copy_command_list_pool& _copy_command_list_pool;
   assets::library<assets::msh::flat_model>& _model_assets;
   texture_manager& _texture_manager;
   output_stream& _error_output;

   mutable std::shared_mutex _mutex;

   absl::flat_hash_map<lowercase_string, model_state> _models;
   absl::flat_hash_map<lowercase_string, asset_ref<assets::msh::flat_model>> _model_asset_refs;
   absl::flat_hash_map<lowercase_string, pending_create_model> _pending_creations;
   absl::flat_hash_map<lowercase_string, asset_ref<assets::msh::flat_model>> _pending_loads;
   absl::flat_hash_set<lowercase_string> _failed_creations;

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

model_manager::model_manager(gpu::device& gpu_device,
                             copy_command_list_pool& copy_command_list_pool,
                             texture_manager& texture_manager,
                             assets::library<assets::msh::flat_model>& model_assets,
                             std::shared_ptr<async::thread_pool> thread_pool,
                             output_stream& error_output)
   : _impl{gpu_device,   copy_command_list_pool, texture_manager,
           model_assets, std::move(thread_pool), error_output}
{
}

model_manager::~model_manager() = default;

auto model_manager::operator[](const lowercase_string& name) -> model&
{
   return _impl.get()[name];
}

void model_manager::for_each(function_ptr<void(model&) noexcept> callback) noexcept
{
   return _impl->for_each(callback);
}

void model_manager::update_models() noexcept
{
   return _impl->update_models();
}

void model_manager::trim_models() noexcept
{
   return _impl->trim_models();
}

bool model_manager::is_placeholder(const model& model) const noexcept
{
   return _impl->is_placeholder(model);
}

auto model_manager::status(const lowercase_string& name) const noexcept -> model_status
{
   return _impl->status(name);
}

}
