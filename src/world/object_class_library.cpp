#include "object_class_library.hpp"
#include "assets/asset_libraries.hpp"
#include "container/pinned_vector.hpp"
#include "object.hpp"
#include "object_class.hpp"

#include <shared_mutex>

#include <absl/container/flat_hash_map.h>

using namespace std::string_literals;

namespace we::world {

namespace {

constexpr uint32 max_object_classes = 131'072;

auto unpack_handle(const object_class_handle handle) noexcept -> uint32
{
   return static_cast<uint32>(handle);
}

auto pack_handle(const uint32 index) noexcept -> object_class_handle
{
   return object_class_handle{index};
}

}

struct object_class_library::impl {
   explicit impl(assets::libraries_manager& asset_libraries) noexcept
      : _asset_libraries{asset_libraries}
   {
      _class_pool.push_back(
         {.object_class = {_asset_libraries, asset_libraries.odfs[lowercase_string{""s}]},
          .ref_count = 0});
   }

   ~impl() = default;

   impl(const impl&) noexcept = delete;
   auto operator=(const impl&) noexcept -> impl& = delete;

   void update() noexcept
   {
      {
         std::scoped_lock lock{_definition_load_queue_mutex, _model_load_queue_mutex};

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
   }

   void clear() noexcept
   {
      _class_pool.resize(1);
      _class_index.clear();
      _class_free_list.clear();
   }

   auto operator[](const object_class_handle handle) const noexcept -> const object_class&
   {
      const uint32 index = unpack_handle(handle);

      [[likely]] if (index < _class_pool.size()) {
         return _class_pool[index].object_class;
      }

      return _class_pool[0].object_class;
   }

   [[nodiscard]] auto acquire(const lowercase_string& name) noexcept -> object_class_handle
   {
      if (name.empty()) return pack_handle(0);

      if (auto it = _class_index.find(name); it != _class_index.end()) {
         const auto [_, index] = *it;

         entry& entry = _class_pool[index];

         // This ain't ever going to happen. But guard against it anyway. Returning a 0 handle is always safe.
         if (entry.ref_count == UINT32_MAX) return pack_handle(0);

         entry.ref_count += 1;

         return pack_handle(index);
      }

      if (not _class_free_list.empty()) {
         const uint32 index = _class_free_list.back();

         _class_free_list.pop_back();

         _class_pool[index] =
            entry{.object_class = {_asset_libraries, _asset_libraries.odfs[name]},
                  .ref_count = 1};
         _class_index.emplace(name, index);

         return pack_handle(index);
      }

      // Also ain't ever going to happen but still guard against it anyway.
      if (_class_pool.size() == _class_pool.max_size()) {
         return pack_handle(0);
      }

      const uint32 index = static_cast<uint32>(_class_pool.size());

      _class_pool.push_back(
         {.object_class = {_asset_libraries, _asset_libraries.odfs[name]},
          .ref_count = 1});
      _class_index.emplace(name, index);

      return pack_handle(index);
   }

   void free(const object_class_handle handle) noexcept
   {
      const uint32 index = unpack_handle(handle);

      // Freeing index 0 is expected and fine. We just do nothing in response to it.
      if (index == 0) return;

      // This indicates serious corruption from somewhere.
      if (index >= _class_pool.size()) std::terminate();

      entry& entry = _class_pool[index];

      entry.ref_count -= 1;

      if (entry.ref_count == 0) {
         entry.object_class = {};

         for (auto it = _class_index.begin(); it != _class_index.end(); ++it) {
            if (it->second == index) {
               _class_index.erase(it);

               break;
            }
         }
      }
      else if (entry.ref_count == UINT32_MAX) {
         std::terminate(); // Double free 🙁
      }
   }

   auto debug_ref_count(const lowercase_string& name) const noexcept -> uint32
   {
      if (auto it = _class_index.find(name); it != _class_index.end()) {
         const auto [_, index] = *it;

         return _class_pool[index].ref_count;
      }

      return 0;
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
      if (auto it = _class_index.find(loaded.name); it != _class_index.end()) {
         const auto [_, index] = *it;

         _class_pool[index].object_class.update_definition(_asset_libraries,
                                                           loaded.asset);
      }
   }

   void model_loaded(const loaded_model& loaded)
   {
      for (entry& entry : _class_pool) {
         if (entry.object_class.model_name != loaded.name) continue;

         entry.object_class.model_asset = loaded.asset;
         entry.object_class.model = loaded.data;
      }
   }

   struct entry {
      object_class object_class;
      uint32 ref_count = 0;
   };

   pinned_vector<entry> _class_pool =
      pinned_vector_init{.max_size = max_object_classes, .initial_capacity = 1024};
   absl::flat_hash_map<lowercase_string, uint32> _class_index;
   std::vector<uint32> _class_free_list;

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

void object_class_library::update() noexcept
{
   _impl->update();
}

void object_class_library::clear() noexcept
{
   _impl->clear();
}

auto object_class_library::operator[](const object_class_handle handle) const noexcept
   -> const object_class&
{
   return _impl.get()[handle];
}

auto object_class_library::acquire(const lowercase_string& name) noexcept -> object_class_handle
{
   return _impl->acquire(name);
}

void object_class_library::free(const object_class_handle handle) noexcept
{
   return _impl->free(handle);
}

auto object_class_library::debug_ref_count(const lowercase_string& name) const noexcept
   -> uint32
{
   return _impl->debug_ref_count(name);
}

}