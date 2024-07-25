#include "object_class_library.hpp"
#include "assets/asset_libraries.hpp"
#include "container/pinned_vector.hpp"
#include "object.hpp"
#include "object_class.hpp"

#include <bit>
#include <shared_mutex>

#include <absl/container/flat_hash_map.h>

using namespace std::string_literals;

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
      _class_pool.push_back(
         {.handle = unpack_handle(null_handle),
          .object_class = {_asset_libraries, asset_libraries.odfs[lowercase_string{""s}]},
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

         _class_pool[handle.index] =
            entry{.handle = handle,
                  .object_class = {_asset_libraries, _asset_libraries.odfs[name]},
                  .ref_count = 1};
         _class_index.emplace(name, uint32{handle.index});

         return pack_handle(handle);
      }

      // Also ain't ever going to happen but still guard against it anyway.
      if (_class_pool.size() == max_object_classes) return null_handle;

      const uint32 index = static_cast<uint32>(_class_pool.size());
      const handle_unpacked handle = {.index = index, .generation = 0};

      _class_pool.push_back(
         {.handle = handle,
          .object_class = {_asset_libraries, _asset_libraries.odfs[name]},
          .ref_count = 1});
      _class_index.emplace(name, index);

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
      handle_unpacked handle;
      object_class object_class;
      uint32 ref_count = 0;
   };

   pinned_vector<entry> _class_pool =
      pinned_vector_init{.max_size = max_object_classes, .initial_capacity = 1024};
   absl::flat_hash_map<lowercase_string, uint32> _class_index;
   std::vector<handle_unpacked> _class_free_list;

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