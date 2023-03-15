#include "object_class_library.hpp"
#include "assets/asset_libraries.hpp"
#include "object.hpp"
#include "object_class.hpp"

#include <shared_mutex>

#include <absl/container/flat_hash_map.h>

#include "imgui/imgui.h"

using namespace std::string_literals;

namespace we::world {

struct object_class_library::impl {
   explicit impl(assets::libraries_manager& asset_libraries) noexcept
      : _default_object_class{asset_libraries,
                              asset_libraries.odfs[lowercase_string{""s}]},
        _asset_libraries{asset_libraries}
   {
   }

   ~impl() = default;

   impl(const impl&) noexcept = delete;
   auto operator=(const impl&) noexcept -> impl& = delete;

   void update(std::span<std::span<const object>> object_spans) noexcept
   {
      for (auto& [name, object_class] : _object_classes) {
         object_class.world_frame_references = 0;
      }

      for (const auto& span : object_spans) {
         for (const auto& object : span) {
            if (auto class_it = _object_classes.find(object.class_name);
                class_it != _object_classes.end()) {
               auto& [name, object_class] = *class_it;

               object_class.world_frame_references += 1;
            }
            else {
               auto definition = _asset_libraries.odfs[object.class_name];

               _object_classes.emplace(object.class_name,
                                       world::object_class{_asset_libraries, definition});
            }
         }
      }

      {
         std::scoped_lock lock{_definition_load_queue_mutex};

         for (const auto& loaded : _definition_load_queue) {
            object_definition_loaded(loaded);
         }

         _definition_load_queue.clear();
      }

      {
         std::scoped_lock lock{_model_load_queue_mutex};

         for (const auto& loaded : _model_load_queue) {
            model_loaded(loaded);
         }

         _model_load_queue.clear();
      }

      absl::erase_if(_object_classes, [](const auto& name_object_class) {
         auto& [name, object_class] = name_object_class;

         return object_class.world_frame_references == 0;
      });
   }

   void clear() noexcept
   {
      _object_classes.clear();
   }

   auto operator[](const lowercase_string& name) const noexcept -> const object_class&
   {
      auto it = _object_classes.find(name);

      return it != _object_classes.end() ? it->second : _default_object_class;
   }

private:
   struct loaded_definition {
      lowercase_string name;
      asset_ref<assets::odf::definition> asset;
      asset_data<assets::odf::definition> data;
   };

   struct loaded_model {
      lowercase_string name;
      asset_ref<assets::msh::flat_model> asset;
      asset_data<assets::msh::flat_model> data;
   };

   void object_definition_loaded(const loaded_definition& loaded)
   {
      _object_classes[loaded.name].update_definition(_asset_libraries, loaded.asset);
   }

   void model_loaded(const loaded_model& loaded)
   {
      for (auto& [object_class_name, object_class] : _object_classes) {
         if (object_class.model_name != loaded.name) continue;

         object_class.model_asset = loaded.asset;
         object_class.model = loaded.data;
      }
   }

   absl::flat_hash_map<lowercase_string, object_class> _object_classes;

   const object_class _default_object_class;

   std::shared_mutex _definition_load_queue_mutex;
   std::vector<loaded_definition> _definition_load_queue;

   std::shared_mutex _model_load_queue_mutex;
   std::vector<loaded_model> _model_load_queue;

   assets::libraries_manager& _asset_libraries;

   event_listener<void(const lowercase_string&, asset_ref<assets::odf::definition>,
                       asset_data<assets::odf::definition>)>
      _object_definition_load_listener = _asset_libraries.odfs.listen_for_loads(
         [this](lowercase_string name, asset_ref<assets::odf::definition> asset,
                asset_data<assets::odf::definition> data) {
            std::scoped_lock lock{_definition_load_queue_mutex};

            _definition_load_queue.emplace_back(std::move(name), std::move(asset),
                                                std::move(data));
         });

   event_listener<void(const lowercase_string&, asset_ref<assets::msh::flat_model>,
                       asset_data<assets::msh::flat_model>)>
      _model_load_listener = _asset_libraries.models.listen_for_loads(
         [this](lowercase_string name, asset_ref<assets::msh::flat_model> asset,
                asset_data<assets::msh::flat_model> data) {
            std::scoped_lock lock{_model_load_queue_mutex};

            _model_load_queue.emplace_back(std::move(name), std::move(asset),
                                           std::move(data));
         });
};

object_class_library::object_class_library(assets::libraries_manager& asset_libraries) noexcept
   : _impl{asset_libraries}
{
}

object_class_library::~object_class_library() = default;

void object_class_library::update(std::span<std::span<const object>> object_spans) noexcept
{
   _impl->update(object_spans);
}

void object_class_library::clear() noexcept
{
   _impl->clear();
}

auto object_class_library::operator[](const lowercase_string& name) const noexcept
   -> const object_class&
{
   return _impl.get()[name];
}

}