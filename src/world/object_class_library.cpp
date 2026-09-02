#include "object_class_library.hpp"
#include "object.hpp"
#include "object_attached.hpp"
#include "object_class.hpp"

#include "object_classes/grass_patch_class.hpp"
#include "object_classes/leaf_patch_class.hpp"

#include "assets/asset_libraries.hpp"
#include "assets/msh/default_missing_scene.hpp"
#include "assets/odf/default_object_class_definition.hpp"

#include "container/pinned_vector.hpp"

#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include <bit>
#include <shared_mutex>
#include <string_view>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

using namespace std::literals;

namespace we::world {

namespace {

constexpr uint32 handle_index_bits = 15;
constexpr uint32 handle_generation_bits = 32 - handle_index_bits;

constexpr uint32 max_object_classes = 1 << handle_index_bits;
constexpr uint32 max_handle_generations = 1 << handle_generation_bits;

struct handle_unpacked {
   uint32 index : handle_index_bits = 0;
   uint32 generation : handle_generation_bits = 0;
};

bool operator==(handle_unpacked l, handle_unpacked r) noexcept
{
   return std::bit_cast<uint32>(l) == std::bit_cast<uint32>(r);
}

auto unpack_handle(const object_class_handle handle) noexcept -> handle_unpacked
{
   return std::bit_cast<handle_unpacked>(handle);
}

constexpr auto pack_handle(const handle_unpacked handle) noexcept -> object_class_handle
{
   return std::bit_cast<object_class_handle>(handle);
}

}

struct object_class_library::impl {
   explicit impl(assets::libraries_manager& asset_libraries) noexcept
      : _asset_libraries{asset_libraries}
   {
      clear();
   }

   ~impl() = default;

   impl(const impl&) noexcept = delete;
   auto operator=(const impl&) noexcept -> impl& = delete;

   void update(const float delta_time) noexcept
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

      for (const uint32 class_index : _leaf_patch_class_index) {
         _billboard_patch_class_pool[class_index]->update(delta_time);
      }
   }

   void clear() noexcept
   {
      _class_pool.clear();
      _class_index.clear();
      _class_free_list.clear();

      _billboard_patch_class_pool.clear();
      _leaf_patch_class_index.clear();

      _attached_objects_pool.clear();
      _attached_objects_index.clear();

      _class_pool.push_back({.handle = unpack_handle(null_handle), .ref_count = 0});

      init_object_class(0, _asset_libraries.odfs[lowercase_string{""s}]);
   }

   auto operator[](const object_class_handle packed_handle) const noexcept
      -> const object_class&
   {
      const handle_unpacked handle = unpack_handle(packed_handle);

      [[likely]] if (handle.index < _class_pool.size()) {
         const entry& entry = _class_pool[handle.index];

         [[likely]] if (entry.handle == handle) {
            return entry.object_class;
         }
      }

      return _class_pool[0].object_class;
   }

   auto get_billboard_patch_class(const object_class_handle packed_handle) const noexcept
      -> const billboard_patch_class&
   {
      const handle_unpacked handle = unpack_handle(packed_handle);

      [[likely]] if (handle.index < _class_pool.size() and
                     handle.index < _billboard_patch_class_pool.size()) {
         const entry& entry = _class_pool[handle.index];

         [[likely]] if (entry.handle == handle) {
            return *_billboard_patch_class_pool[handle.index];
         }
      }

      const static leaf_patch_class default_leaf_patch_class{
         *assets::odf::default_object_class_definition()};

      return default_leaf_patch_class;
   }

   auto get_attached_objects(const object_class_handle packed_handle) const noexcept
      -> std::span<const object_attached>
   {
      const handle_unpacked handle = unpack_handle(packed_handle);

      [[likely]] if (handle.index < _class_pool.size() and
                     handle.index < _attached_objects_pool.size()) {
         const entry& entry = _class_pool[handle.index];

         [[likely]] if (entry.handle == handle) {
            return _attached_objects_pool[handle.index];
         }
      }

      return {};
   }

   [[nodiscard]] auto acquire(const lowercase_string& name) noexcept -> object_class_handle
   {
      if (name.empty()) return null_handle;

      if (auto it = _class_index.find(name); it != _class_index.end()) {
         const auto [_, index] = *it;

         entry& entry = _class_pool[index];

         // This ain't ever going to happen. But guard against it anyway.
         if (entry.ref_count == UINT32_MAX) return null_handle;

         entry.ref_count += 1;

         return pack_handle(entry.handle);
      }

      if (not _class_free_list.empty()) {
         handle_unpacked handle = _class_free_list.back();

         _class_free_list.pop_back();

         handle.generation += 1;

         _class_pool[handle.index] = entry{.handle = handle, .ref_count = 1};
         _class_index.emplace(name, uint32{handle.index});

         init_object_class(handle.index, _asset_libraries.odfs[name]);

         return pack_handle(handle);
      }

      // Also ain't ever going to happen but still guard against it anyway.
      if (_class_pool.size() == max_object_classes) return null_handle;

      const uint32 index = static_cast<uint32>(_class_pool.size());
      const handle_unpacked handle = {.index = index, .generation = 0};

      _class_pool.push_back({.handle = handle, .ref_count = 1});
      _class_index.emplace(name, index);

      init_object_class(index, _asset_libraries.odfs[name]);

      return pack_handle(handle);
   }

   void free(const object_class_handle packed_handle) noexcept
   {
      const handle_unpacked handle = unpack_handle(packed_handle);

      // Freeing index 0 is expected and fine. We just do nothing in response to it.
      if (handle.index == 0) return;

      // This indicates serious corruption from somewhere. We would never have returned this as a valid handle.
      if (handle.index >= _class_pool.size()) std::terminate();

      entry& entry = _class_pool[handle.index];

      // Only free a handle whose index and generation matches.
      if (entry.handle != handle) return;

      // Double frees are problematic but don't necessarily indicate data corruption so failing fast is overkill.
      if (entry.ref_count == 0) return;

      entry.ref_count -= 1;

      if (entry.ref_count == 0) {
         entry.handle = unpack_handle(null_handle);
         entry.object_class = {};

         for (auto it = _class_index.begin(); it != _class_index.end(); ++it) {
            if (it->second == handle.index) {
               _class_index.erase(it);

               if (handle.index < _billboard_patch_class_pool.size()) {
                  _billboard_patch_class_pool[handle.index] = nullptr;
                  std::erase(_leaf_patch_class_index, handle.index);
               }

               if (handle.index < _attached_objects_pool.size()) {
                  for (const object_attached& object :
                       _attached_objects_pool[handle.index]) {
                     free(object.class_handle);
                  }

                  _attached_objects_pool[handle.index] = {};
                  std::erase(_attached_objects_index, handle.index);
               }

               break;
            }
         }

         _class_free_list.push_back(handle);
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

   constexpr static object_class_handle null_handle = object_class_handle{0};

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

         init_object_class(index, loaded.asset);
      }
   }

   void model_loaded(const loaded_model& loaded)
   {
      for (std::size_t entry_index = 0; entry_index < _class_pool.size(); ++entry_index) {
         entry& entry = _class_pool[entry_index];

         if (entry.object_class.model_name != loaded.name) continue;

         entry.object_class.model_asset = loaded.asset;
         entry.object_class.model = loaded.data;

         if (entry.object_class.flags.has_attached_objects) {
            init_attached_objects_transforms(entry.object_class,
                                             _attached_objects_pool[entry_index]);
         }
      }
   }

   void init_object_class(uint32 class_index,
                          asset_ref<assets::odf::definition> new_definition_asset)
   {
      object_class& cls = _class_pool[class_index].object_class;

      if (class_index < _billboard_patch_class_pool.size()) {
         _billboard_patch_class_pool[class_index] = nullptr;
         std::erase(_leaf_patch_class_index, class_index);
      }

      if (class_index < _attached_objects_pool.size()) {
         for (const object_attached& object : _attached_objects_pool[class_index]) {
            free(object.class_handle);
         }

         _attached_objects_pool[class_index] = {};
         std::erase(_attached_objects_index, class_index);
      }

      cls.definition_asset = new_definition_asset;
      cls.definition = cls.definition_asset.get_if();

      if (not cls.definition) {
         cls.definition = assets::odf::default_object_class_definition();
      }

      if (string::iends_with(cls.definition->header.geometry_name, ".msh"sv)) {
         cls.model_name = lowercase_string{cls.definition->header.geometry_name.substr(
            0, cls.definition->header.geometry_name.size() - ".msh"sv.size())};
      }
      else {
         cls.model_name = lowercase_string{cls.definition->header.geometry_name};
      }

      if (not cls.model_name.empty()) {
         cls.model_asset = _asset_libraries.models[cls.model_name];
         cls.model = cls.model_asset.get_if();
      }

      if (not cls.model) cls.model = assets::msh::default_missing_scene();

      cls.flags = {.hidden_ingame = true};

      for (const auto& prop : cls.definition->properties) {
         if (string::iequals(prop.key, "GeometryName")) {
            cls.flags.hidden_ingame = false;
         }
      }

      if (string::iequals(cls.definition->header.class_label, "leafpatch")) {
         cls.flags.is_billboard_patch = true;

         if (_billboard_patch_class_pool.size() <= class_index) {
            _billboard_patch_class_pool.resize(class_index + 1);
         }

         _billboard_patch_class_pool[class_index] =
            std::make_unique<leaf_patch_class>(*cls.definition);
         _leaf_patch_class_index.push_back(class_index);
      }
      else if (string::iequals(cls.definition->header.class_label,
                               "grasspatch")) {
         cls.flags.is_billboard_patch = true;

         if (_billboard_patch_class_pool.size() <= class_index) {
            _billboard_patch_class_pool.resize(class_index + 1);
         }

         _billboard_patch_class_pool[class_index] =
            std::make_unique<grass_patch_class>(*cls.definition);
         _leaf_patch_class_index.push_back(class_index);
      }
      else {
         std::vector<object_attached> attached_objects;
         std::string_view last_attach_odf;

         for (const assets::odf::property& prop : cls.definition->properties) {
            if (string::iequals(prop.key, "AttachOdf")) {
               last_attach_odf = prop.value;
            }
            else if (string::iequals(prop.key, "AttachEffect")) {
               last_attach_odf = "";
            }
            else if (string::iequals(prop.key, "AttachToHardPoint")) {
               std::string_view hard_point =
                  string::split_first_of_exclusive_whitespace(prop.value)[0];

               if (not last_attach_odf.empty()) {
                  object_attached attached = {
                     .class_name = lowercase_string{last_attach_odf},
                     .hard_point_name = std::string{hard_point},
                  };

                  attached.class_handle = acquire(attached.class_name);

                  attached_objects.push_back(std::move(attached));
               }
            }
         }

         if (not attached_objects.empty()) {
            cls.flags.has_attached_objects = true;

            if (_attached_objects_pool.size() <= class_index) {
               _attached_objects_pool.resize(class_index + 1);
            }

            _attached_objects_pool[class_index] = std::move(attached_objects);
            _attached_objects_index.push_back(class_index);

            init_attached_objects_transforms(cls, _attached_objects_pool[class_index]);
         }
      }
   }

   void init_attached_objects_transforms(const object_class& object_class,
                                         std::span<object_attached> attachments)
   {
      const std::span<const assets::msh::flat_model_node> nodes =
         object_class.model->nodes;

      for (object_attached& attached : attachments) {
         bool found = false;

         for (const assets::msh::flat_model_node& node : nodes) {
            if (string::iequals(node.name, attached.hard_point_name)) {
               attached.object_from_local = node.local_from_vertex;

               found = true;

               break;
            }
         }

         if (not found) attached.object_from_local = {};
      }
   }

   struct entry {
      handle_unpacked handle;
      object_class object_class;
      uint32 ref_count = 0;
   };

   pinned_vector<entry> _class_pool =
      pinned_vector_init{.max_size = max_object_classes, .initial_capacity = 1024};
   absl::flat_hash_map<lowercase_string, uint32> _class_index;
   std::vector<handle_unpacked> _class_free_list;

   pinned_vector<std::unique_ptr<billboard_patch_class>> _billboard_patch_class_pool =
      pinned_vector_init{.max_size = max_object_classes, .initial_capacity = 1024};
   std::vector<uint32> _leaf_patch_class_index;

   pinned_vector<std::vector<object_attached>> _attached_objects_pool =
      pinned_vector_init{.max_size = max_object_classes, .initial_capacity = 1024};
   std::vector<uint32> _attached_objects_index;

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

void object_class_library::update(const float delta_time) noexcept
{
   _impl->update(delta_time);
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

auto object_class_library::get_billboard_patch_class(const object_class_handle handle) const noexcept
   -> const billboard_patch_class&
{
   return _impl->get_billboard_patch_class(handle);
}

auto object_class_library::get_attached_objects(const object_class_handle handle) const noexcept
   -> std::span<const object_attached>
{
   return _impl->get_attached_objects(handle);
}

auto object_class_library::acquire(const lowercase_string& name) noexcept -> object_class_handle
{
   return _impl->acquire(name);
}

void object_class_library::free(const object_class_handle handle) noexcept
{
   return _impl->free(handle);
}

auto object_class_library::null_handle() noexcept -> object_class_handle
{
   return impl::null_handle;
}

auto object_class_library::debug_ref_count(const lowercase_string& name) const noexcept
   -> uint32
{
   return _impl->debug_ref_count(name);
}

}