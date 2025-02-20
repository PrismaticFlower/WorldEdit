#include "thumbnail_manager.hpp"
#include "assets/asset_libraries.hpp"
#include "assets/odf/definition.hpp"
#include "async/thread_pool.hpp"
#include "dynamic_buffer_allocator.hpp"
#include "gpu/resource.hpp"
#include "io/error.hpp"
#include "io/memory_mapped_file.hpp"
#include "io/read_file.hpp"
#include "math/align.hpp"
#include "math/matrix_funcs.hpp"
#include "model_manager.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "utility/binary_reader.hpp"
#include "utility/string_icompare.hpp"

#include <optional>
#include <shared_mutex>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

using namespace std::literals;

namespace we::graphics {

namespace {

constexpr uint32 cache_save_version = 1;
constexpr uint32 max_texture_length = 16384;
constexpr uint32 atlas_thumbnails = 256;
constexpr uint32 aa_factor = 4;

struct camera_info {
   float4x4 projection_from_world;
   float3 camera_position;
};

auto make_camera_info(const model& model) -> camera_info
{
   constexpr float pitch = -0.7853982f;
   constexpr float yaw = 0.7853982f;

   const float3 bounding_sphere_centre = (model.bbox.max + model.bbox.min) / 2.0f;
   const float bounding_sphere_radius = distance(model.bbox.max, model.bbox.min) / 2.0f;

   float4x4 rotation = make_rotation_matrix_from_euler({pitch, yaw, 0.0f});

   const float3 backward{rotation[2].x, rotation[2].y, rotation[2].z};

   const float3 position = bounding_sphere_centre + bounding_sphere_radius * backward;

   float4x4 world_from_view = rotation;
   world_from_view[3] = {position, 1.0f};

   float4x4 rotation_inverse = transpose(rotation);
   float4x4 view_from_world = rotation_inverse;
   view_from_world[3] = {rotation_inverse * -position, 1.0f};

   float4x4 projection_from_view = {{1.0f, 0.0f, 0.0f, 0.0f}, //
                                    {0.0f, 1.0f, 0.0f, 0.0f}, //
                                    {0.0f, 0.0f, 1.0f, 0.0f}, //
                                    {0.0f, 0.0f, 0.0f, 1.0f}};

   const float near_clip = 0.0f;
   const float far_clip = bounding_sphere_radius * 2.0f;

   const float view_width = bounding_sphere_radius * 2.0f;
   const float view_height = bounding_sphere_radius * 2.0f;
   const float z_range = 1.0f / (far_clip - near_clip);

   projection_from_view[0].x = 2.0f / view_width;
   projection_from_view[1].y = 2.0f / view_height;
   projection_from_view[2].z = z_range;
   projection_from_view[3].z = z_range * far_clip;

   const float4x4 projection_from_world = projection_from_view * view_from_world;

   return {.projection_from_world = projection_from_world, .camera_position = position};
}

struct invalidation_save_entry {
   struct name_time {
      template<typename T>
      name_time(lowercase_string init_name, assets::library<T>& assets)
         : name{std::move(init_name)}
      {
         time = assets.query_last_write_time(name);
      }

      name_time(std::string_view init_name, uint64 time)
         : name{init_name}, time{time}
      {
      }

      template<typename T>
      bool invalidated(assets::library<T>& assets) const noexcept
      {
         return time != assets.query_last_write_time(name);
      }

      lowercase_string name;
      uint64 time = 0;
   };

   bool invalidated(assets::libraries_manager& asset_libraries) const noexcept
   {
      if (odf.invalidated(asset_libraries.odfs)) return true;
      if (model.invalidated(asset_libraries.models)) return true;

      for (const auto& texture : textures) {
         if (texture.invalidated(asset_libraries.textures)) {
            return true;
         }
      }

      return false;
   }

   name_time odf;
   name_time model;
   std::vector<name_time> textures;
};

struct invalidation_tracker {
   explicit invalidation_tracker(assets::libraries_manager& asset_libraries)
      : _odf_change_listener{asset_libraries.odfs.listen_for_changes(
           [this](const lowercase_string& name) { odf_changed(name); })},
        _msh_change_listener{asset_libraries.models.listen_for_changes(
           [this](const lowercase_string& name) { msh_changed(name); })},
        _texture_change_listener{asset_libraries.textures.listen_for_changes(
           [this](const lowercase_string& name) { texture_changed(name); })},
        _asset_libraries{asset_libraries}
   {
   }

   void track(std::string_view odf_name, std::string_view model_name,
              const model& model) noexcept
   {
      std::scoped_lock lock{_mutex};

      uint32 texture_count = 0;

      for (const auto& part : model.parts) {
         if (not part.material.texture_names.diffuse_map.empty()) {
            texture_count += 1;
         }
         if (not part.material.texture_names.normal_map.empty()) {
            texture_count += 1;
         }
         if (not part.material.texture_names.detail_map.empty()) {
            texture_count += 1;
         }
         if (not part.material.texture_names.env_map.empty()) {
            texture_count += 1;
         }
      }

      odf_entry entry{.model = lowercase_string{model_name}};

      entry.textures.reserve(texture_count);

      for (const auto& part : model.parts) {
         if (not part.material.texture_names.diffuse_map.empty()) {
            entry.textures.push_back(part.material.texture_names.diffuse_map);
         }
         if (not part.material.texture_names.normal_map.empty()) {
            entry.textures.push_back(part.material.texture_names.normal_map);
         }
         if (not part.material.texture_names.detail_map.empty()) {
            entry.textures.push_back(part.material.texture_names.detail_map);
         }
         if (not part.material.texture_names.env_map.empty()) {
            entry.textures.push_back(part.material.texture_names.env_map);
         }
      }

      lowercase_string name{odf_name};

      if (_odfs.contains(name)) release(name);

      child_entry& model_entry = _models[entry.model];

      model_entry.ref_count += 1;
      model_entry.odfs.emplace_back(name);

      for (auto& texture : entry.textures) {
         child_entry& texture_entry = _textures[texture];

         texture_entry.ref_count += 1;
         texture_entry.odfs.emplace_back(name);
      }

      _odfs.emplace(std::move(name), std::move(entry));
   }

   auto get_invalidated() -> std::span<std::string>
   {
      _invalidated.clear();

      std::scoped_lock lock{_invalidated_background_mutex};

      std::swap(_invalidated, _invalidated_background);

      return _invalidated;
   }

   auto get_save_data() noexcept -> std::vector<invalidation_save_entry>
   {
      std::scoped_lock lock{_mutex};

      std::vector<invalidation_save_entry> save_data;
      save_data.reserve(_odfs.size());

      for (const auto& [name, entry] : _odfs) {
         using name_time = invalidation_save_entry::name_time;

         std::vector<name_time> textures;
         textures.reserve(entry.textures.size());

         for (const auto& texture : entry.textures) {
            textures.emplace_back(texture, _asset_libraries.textures);
         }

         save_data.emplace_back(name_time{name, _asset_libraries.odfs},
                                name_time{entry.model, _asset_libraries.models},
                                std::move(textures));
      }

      return save_data;
   }

   void restore_from_save_data(const std::span<const invalidation_save_entry> save_data) noexcept
   {
      {
         std::scoped_lock lock{_mutex};

         _odfs.reserve(save_data.size());
      }

      for (const invalidation_save_entry& entry : save_data) {
         if (entry.invalidated(_asset_libraries)) {
            std::scoped_lock lock{_mutex, _invalidated_background_mutex};

            if (not _odfs.contains(entry.odf.name)) {
               _invalidated_background.push_back(entry.odf.name);
            }

            continue;
         }

         std::scoped_lock lock{_mutex};

         if (_odfs.contains(entry.odf.name)) continue;

         std::vector<lowercase_string> odf_entry_textures;
         odf_entry_textures.reserve(entry.textures.size());

         for (auto& texture : entry.textures) {
            odf_entry_textures.emplace_back(texture.name);
         }

         _odfs.emplace(entry.odf.name,
                       odf_entry{.model = entry.model.name,
                                 .textures = std::move(odf_entry_textures)});

         child_entry& model_entry = _models[entry.model.name];

         model_entry.ref_count += 1;
         model_entry.odfs.emplace_back(entry.odf.name);

         for (auto& texture : entry.textures) {
            child_entry& texture_entry = _textures[texture.name];

            texture_entry.ref_count += 1;
            texture_entry.odfs.emplace_back(entry.odf.name);
         }
      }
   }

   void clear() noexcept
   {
      _invalidated.clear();

      std::scoped_lock lock{_mutex, _invalidated_background_mutex};

      _odfs.clear();
      _models.clear();
      _textures.clear();
      _invalidated_background.clear();
   }

private:
   bool release(const lowercase_string& name)
   {
      auto odf_it = _odfs.find(name);

      if (odf_it == _odfs.end()) return false;

      const odf_entry& odf_entry = odf_it->second;

      if (auto model_it = _models.find(odf_entry.model); model_it != _models.end()) {
         child_entry& model_entry = model_it->second;

         model_entry.ref_count -= 1;

         std::erase(model_entry.odfs, name);

         if (model_entry.ref_count == 0) _models.erase(model_it);
      }

      for (const auto& texture : odf_entry.textures) {
         if (auto texture_it = _textures.find(texture); texture_it != _textures.end()) {
            child_entry& texture_entry = texture_it->second;

            texture_entry.ref_count -= 1;

            std::erase(texture_entry.odfs, name);

            if (texture_entry.ref_count == 0) _textures.erase(texture_it);
         }
      }

      _odfs.erase(name);

      return true;
   }

   void odf_changed(const lowercase_string& name) noexcept
   {
      std::scoped_lock lock{_mutex, _invalidated_background_mutex};

      if (release(name)) {
         _invalidated_background.emplace_back(name);
      }
   }

   void msh_changed(const lowercase_string& name) noexcept
   {
      std::scoped_lock lock{_mutex, _invalidated_background_mutex};

      const std::vector<lowercase_string> odfs = [&] {
         if (auto it = _models.find(name); it != _models.end()) {
            return std::move(it->second.odfs);
         }

         return std::vector<lowercase_string>{};
      }();

      for (const auto& odf_name : odfs) release(odf_name);

      _invalidated_background.append_range(odfs);
   }

   void texture_changed(const lowercase_string& name) noexcept
   {
      std::scoped_lock lock{_mutex, _invalidated_background_mutex};

      const std::vector<lowercase_string> odfs = [&] {
         if (auto it = _textures.find(name); it != _textures.end()) {
            return std::move(it->second.odfs);
         }

         return std::vector<lowercase_string>{};
      }();

      for (const auto& odf_name : odfs) release(odf_name);

      _invalidated_background.append_range(odfs);
   }

   struct odf_entry {
      lowercase_string model;
      std::vector<lowercase_string> textures;
   };

   struct child_entry {
      uint32 ref_count = 0;
      std::vector<lowercase_string> odfs;
   };

   std::vector<std::string> _invalidated;

   std::shared_mutex _mutex;
   absl::flat_hash_map<lowercase_string, odf_entry> _odfs;
   absl::flat_hash_map<lowercase_string, child_entry> _models;
   absl::flat_hash_map<lowercase_string, child_entry> _textures;

   std::shared_mutex _invalidated_background_mutex;
   std::vector<std::string> _invalidated_background;

   assets::libraries_manager& _asset_libraries;

   event_listener<void(const lowercase_string&)> _odf_change_listener;
   event_listener<void(const lowercase_string&)> _msh_change_listener;
   event_listener<void(const lowercase_string&)> _texture_change_listener;
};

void texture_copy(std::byte* dest, const std::size_t dest_pitch,
                  const std::byte* src, std::size_t src_pitch,
                  const std::size_t length, const std::size_t elem_byte_size)
{
   for (std::size_t y = 0; y < length; ++y) {
      std::memcpy(dest, src, length * elem_byte_size);

      dest += dest_pitch;
      src += src_pitch;
   }
}

void save_disk_cache(
   const io::path& path,
   const absl::flat_hash_map<std::string, std::unique_ptr<std::byte[]>>& cpu_memory_cache,
   const uint32 thumbnail_length,
   std::span<const invalidation_save_entry> invalidation_save_data) noexcept
{
   try {
      const auto output_to_memory = [&](std::byte* const output_data) {
         std::size_t write_head = 0;

         const auto write_object = [&]<typename T>(const T& value) {
            static_assert(std::is_trivially_copyable_v<T>);

            if (output_data) {
               std::memcpy(output_data + write_head, &value, sizeof(T));
            }

            write_head += sizeof(T);
         };

         const auto write_string = [&](const std::string_view str) {
            if (output_data) {
               std::memcpy(output_data + write_head, str.data(), str.size());
            }

            write_head += str.size();
         };

         const auto write_bytes = [&](const std::span<const std::byte> bytes) {
            if (output_data) {
               std::memcpy(output_data + write_head, bytes.data(), bytes.size());
            }

            write_head += bytes.size();
         };

         write_object(cache_save_version);
         write_object(thumbnail_length);
         write_object(static_cast<uint32>(sizeof(std::size_t)));

         write_object(cpu_memory_cache.size());

         for (const auto& [name, data] : cpu_memory_cache) {
            write_object(name.size());
            write_string(name);
            write_bytes(std::span{data.get(), thumbnail_length * thumbnail_length *
                                                 sizeof(uint32)});
         }

         write_object(invalidation_save_data.size());

         for (const auto& entry : invalidation_save_data) {
            write_object(entry.odf.name.size());
            write_string(entry.odf.name);
            write_object(entry.odf.time);
            write_object(entry.model.name.size());
            write_string(entry.model.name);
            write_object(entry.model.time);

            write_object(entry.textures.size());

            for (const auto& texture : entry.textures) {
               write_object(texture.name.size());
               write_string(texture.name);
               write_object(texture.time);
            }
         }

         return write_head;
      };

      const std::size_t size = output_to_memory(nullptr);

      io::memory_mapped_file file{
         io::memory_mapped_file_params{.path = path, .size = size}};

      output_to_memory(file.data());
   }
   catch (io::error&) {
   }
}

struct loaded_cache {
   absl::flat_hash_map<std::string, std::unique_ptr<std::byte[]>> cpu_memory_cache;
   uint32 thumbnail_length = 0;
   std::vector<invalidation_save_entry> invalidation_save_data;
};

auto load_disk_cache(const io::path& path) noexcept -> loaded_cache
{
   try {
      loaded_cache loaded;

      const std::vector<std::byte> bytes = io::read_file_to_bytes(path);

      utility::binary_reader cache{bytes};

      const uint32 version = cache.read<uint32>();
      const uint32 thumbnail_length = cache.read<uint32>();
      const uint32 size_t_size = cache.read<uint32>();
      const uint32 thumbnail_size =
         thumbnail_length * thumbnail_length * sizeof(uint32);

      if (version != cache_save_version) return {};
      if (size_t_size != sizeof(std::size_t)) return {};

      const std::size_t cached_thumbnails = cache.read<std::size_t>();

      loaded.cpu_memory_cache.reserve(cached_thumbnails);

      for (std::size_t i = 0; i < cached_thumbnails; ++i) {
         const std::span<const std::byte> name_bytes =
            cache.read_bytes(cache.read<std::size_t>());
         const std::string_view name{reinterpret_cast<const char*>(name_bytes.data()),
                                     name_bytes.size()};

         const std::span<const std::byte> thumbnail = cache.read_bytes(thumbnail_size);

         std::unique_ptr<std::byte[]>& memory = loaded.cpu_memory_cache[name];

         if (not memory) {
            memory = std::make_unique_for_overwrite<std::byte[]>(thumbnail_size);
         }

         std::memcpy(memory.get(), thumbnail.data(), thumbnail_size);
      }

      const std::size_t cached_invalidation_entries = cache.read<std::size_t>();

      loaded.invalidation_save_data.reserve(cached_invalidation_entries);

      for (std::size_t entry_index = 0;
           entry_index < cached_invalidation_entries; ++entry_index) {
         const std::span<const std::byte> odf_bytes =
            cache.read_bytes(cache.read<std::size_t>());
         const std::string_view odf{reinterpret_cast<const char*>(odf_bytes.data()),
                                    odf_bytes.size()};
         const uint64 odf_last_write_time = cache.read<uint64>();

         const std::span<const std::byte> model_bytes =
            cache.read_bytes(cache.read<std::size_t>());
         const std::string_view model{reinterpret_cast<const char*>(model_bytes.data()),
                                      model_bytes.size()};
         const uint64 model_last_write_time = cache.read<uint64>();

         const std::size_t texture_count = cache.read<std::size_t>();

         using name_time = invalidation_save_entry::name_time;

         std::vector<name_time> textures;
         textures.reserve(texture_count);

         for (std::size_t texture_index = 0; texture_index < texture_count;
              ++texture_index) {
            const std::span<const std::byte> texture_bytes =
               cache.read_bytes(cache.read<std::size_t>());
            const std::string_view texture{reinterpret_cast<const char*>(
                                              texture_bytes.data()),
                                           texture_bytes.size()};
            const uint64 texture_last_write_time = cache.read<uint64>();

            textures.emplace_back(texture, texture_last_write_time);
         }

         loaded.invalidation_save_data.emplace_back(name_time{odf, odf_last_write_time},
                                                    name_time{model, model_last_write_time},
                                                    std::move(textures));
      }

      loaded.thumbnail_length = thumbnail_length;

      return loaded;
   }
   catch (std::runtime_error&) {
      return {};
   }
}

}

struct thumbnail_manager::impl {
   explicit impl(const thumbnail_manager_init& init)
      : _thread_pool{init.thread_pool},
        _asset_libraries{init.asset_libraries},
        _error_output{init.error_output},
        _device{init.device},
        _display_scale{init.display_scale}
   {
      _back_items.reserve(atlas_thumbnails);
      _recycle_items.reserve(atlas_thumbnails);
      _cpu_memory_cache.reserve(2048);

      create_gpu_resources();
   }

   ~impl()
   {
      if (_save_disk_cache_task) _save_disk_cache_task->wait();
   }

   auto request_object_class_thumbnail(const std::string_view name) -> object_class_thumbnail
   {
      if (auto it = _back_items.find(name); it != _back_items.end()) {
         const thumbnail_index index = it->second;

         _front_items.emplace(name, index);

         return {.imgui_texture_id =
                    reinterpret_cast<void*>(uint64{_atlas_srv.get().index}),
                 .uv_left = index.x / _atlas_items_width,
                 .uv_top = index.y / _atlas_items_height,
                 .uv_right = (index.x + 1) / _atlas_items_width,
                 .uv_bottom = (index.y + 1) / _atlas_items_height};
      }

      if (auto it = _recycle_items.find(name); it != _recycle_items.end()) {
         const thumbnail_index index = it->second.index;

         _back_items.emplace(name, index);
         _front_items.emplace(name, index);
         _recycle_items.erase(it);

         return {.imgui_texture_id =
                    reinterpret_cast<void*>(uint64{_atlas_srv.get().index}),
                 .uv_left = index.x / _atlas_items_width,
                 .uv_top = index.y / _atlas_items_height,
                 .uv_right = (index.x + 1) / _atlas_items_width,
                 .uv_bottom = (index.y + 1) / _atlas_items_height};
      }

      if (not _cache_upload_item) {
         if (auto it = _cpu_memory_cache.find(name); it != _cpu_memory_cache.end()) {
            if (const std::optional<thumbnail_index> index =
                   get_or_allocate_thumbnail_index(name);
                index) {
               _cache_upload_item.emplace(*index);

               const std::byte* const src_data = it->second.get();

               texture_copy(_upload_pointers[_device.frame_index()],
                            _upload_readback_pitch, src_data,
                            _thumbnail_length * sizeof(uint32),
                            _thumbnail_length, sizeof(uint32));

               return {.imgui_texture_id =
                          reinterpret_cast<void*>(uint64{_atlas_srv.get().index}),
                       .uv_left = index->x / _atlas_items_width,
                       .uv_top = index->y / _atlas_items_height,
                       .uv_right = (index->x + 1) / _atlas_items_width,
                       .uv_bottom = (index->y + 1) / _atlas_items_height};
            }
         }
      }

      if (not _cpu_memory_cache.contains(name)) enqueue_create_thumbnail(name);

      return {.imgui_texture_id =
                 reinterpret_cast<void*>(uint64{_atlas_srv.get().index}),
              .uv_left = 0.0f,
              .uv_top = 0.0f,
              .uv_right = 0.0f,
              .uv_bottom = 0.0f};
   }

   void update_cpu_cache()
   {
      if (not _readback_names[_device.frame_index()].empty()) {
         const uint32 size = _thumbnail_length * _thumbnail_length * sizeof(uint32);

         std::unique_ptr<std::byte[]>& memory =
            _cpu_memory_cache[_readback_names[_device.frame_index()]];

         if (not memory) {
            memory = std::make_unique_for_overwrite<std::byte[]>(size);
         }

         texture_copy(memory.get(), _thumbnail_length * sizeof(uint32),
                      _readback_pointers[_device.frame_index()],
                      _upload_readback_pitch, _thumbnail_length, sizeof(uint32));

         _readback_names[_device.frame_index()].clear();
         _cpu_cache_dirty = true;
      }

      if (_save_disk_cache_task and _save_disk_cache_task->ready()) {
         _save_disk_cache_task = std::nullopt;
      }

      if (_load_disk_cache_task and _load_disk_cache_task->ready()) {
         loaded_cache loaded = _load_disk_cache_task->get();

         _load_disk_cache_task = std::nullopt;

         if (loaded.thumbnail_length == _thumbnail_length) {
            _cpu_memory_cache = std::move(loaded.cpu_memory_cache);
            _cpu_cache_dirty = false;

            if (_restore_invalidation_tracker_task) {
               _restore_invalidation_tracker_task->wait();
            }

            _restore_invalidation_tracker_task = _thread_pool->exec(
               [this, invalidation_save_data = std::move(loaded.invalidation_save_data)] {
                  _invalidation_tracker.restore_from_save_data(invalidation_save_data);
               });
         }
      }
   }

   void update_gpu(model_manager& model_manager,
                   root_signature_library& root_signatures, pipeline_library& pipelines,
                   dynamic_buffer_allocator& dynamic_buffer_allocator,
                   gpu::graphics_command_list& command_list)
   {
      if (_cache_upload_item) {
         copy_cache_upload_item(_cache_upload_item->index, command_list);

         _cache_upload_item = std::nullopt;
      }

      if (not _rendering) {
         std::scoped_lock lock{_pending_render_mutex};

         if (_pending_render.empty()) return;

         const auto& [name, pending] = *_pending_render.begin();

         _rendering = rendering{.name = name,
                                .model_name = pending.model_name,
                                .model = pending.model};
      }

      const model& model = model_manager[_rendering->model_name];
      const model_status status = model_manager.status(_rendering->model_name);

      if (status == model_status::ready or status == model_status::ready_textures_missing or
          status == model_status::ready_textures_loading) {
         const std::optional<thumbnail_index> thumbnail_index =
            get_or_allocate_thumbnail_index(_rendering->name);

         if (not thumbnail_index) return;

         draw(model, *thumbnail_index, root_signatures, pipelines,
              dynamic_buffer_allocator, command_list);

         _readback_names[_device.frame_index()] = _rendering->name;

         if (status == model_status::ready or
             status == model_status::ready_textures_missing) {
            std::scoped_lock lock{_pending_render_mutex};

            _pending_render.erase(_rendering->name);
            _invalidation_tracker.track(_rendering->name, _rendering->model_name, model);
            _rendering = std::nullopt;
         }
      }
      else if (status == model_status::errored or status == model_status::missing) {
         _error_output.write("Unable to render thumbnail for '{}'. Model "
                             "errored or missing.\n",
                             _rendering->name);

         std::scoped_lock lock{_pending_render_mutex, _missing_model_odfs_mutex};

         _missing_model_odfs.emplace(_rendering->name);
         _pending_render.erase(_rendering->name);
         _rendering = std::nullopt;
      }
   }

   void end_frame()
   {
      std::swap(_back_items, _front_items);

      for (const auto& [name, index] : _front_items) {
         if (not _back_items.contains(name)) {
            if (not _recycle_items.emplace(name, recycle_item{index, _frame}).second) {
               _free_items.push_back(index);
            }
         }
      }

      _front_items.clear();

      _frame += 1;

      for (const auto& name : _invalidation_tracker.get_invalidated()) {
         if (auto it = _back_items.find(name); it != _back_items.end()) {
            const thumbnail_index index = it->second;

            _free_items.push_back(index);
            _back_items.erase(it);
         }

         if (auto it = _recycle_items.find(name); it != _recycle_items.end()) {
            const thumbnail_index index = it->second.index;

            _free_items.push_back(index);
            _recycle_items.erase(it);
         }

         _cpu_memory_cache.erase(name);
      }
   }

   void display_scale_changed(const float new_display_scale)
   {
      if (_display_scale == new_display_scale) return;

      _display_scale = new_display_scale;

      _thumbnail_length = static_cast<uint32>(128 * _display_scale);

      _free_items.clear();
      _front_items.clear();
      _back_items.clear();
      _recycle_items.clear();
      _cpu_memory_cache.clear();

      create_gpu_resources();
   }

   void async_save_disk_cache(const char* path) noexcept
   {
      if (not _cpu_cache_dirty) return;
      if (_cpu_memory_cache.empty()) return;

      if (_save_disk_cache_task) _save_disk_cache_task->wait();

      _save_disk_cache_task =
         _thread_pool->exec([disk_cache_path = io::path{path},
                             cpu_memory_cache = std::move(_cpu_memory_cache),
                             thumbnail_length = _thumbnail_length,
                             &invalidation_tracker = _invalidation_tracker] {
            save_disk_cache(disk_cache_path, cpu_memory_cache, thumbnail_length,
                            invalidation_tracker.get_save_data());
         });
   }

   void async_load_disk_cache(const char* path) noexcept
   {
      _load_disk_cache_task =
         _thread_pool->exec(async::task_priority::low,
                            [disk_cache_path = io::path{path}] {
                               return load_disk_cache(disk_cache_path);
                            });
   }

   void reset() noexcept
   {
      create_gpu_resources();

      _front_items.clear();
      _back_items.clear();
      _recycle_items.clear();
      _cpu_memory_cache.clear();

      {
         std::scoped_lock lock{_pending_odfs_mutex, _pending_render_mutex,
                               _missing_model_odfs_mutex};

         _pending_odfs.clear();
         _pending_render.clear();
         _missing_model_odfs.clear();
      }

      _invalidation_tracker.clear();
   }

private:
   struct thumbnail_index {
      uint16 x = 0;
      uint16 y = 0;
   };

   void copy_cache_upload_item(const thumbnail_index index,
                               gpu::graphics_command_list& command_list)
   {
      [[likely]] if (_device.supports_enhanced_barriers()) {
         // Barrier free access!
      }
      else {
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _atlas_texture.get(),
            .state_before = gpu::legacy_resource_state::pixel_shader_resource,
            .state_after = gpu::legacy_resource_state::copy_dest});

         command_list.flush_barriers();
      }

      command_list.copy_buffer_to_texture(_atlas_texture.get(), 0,
                                          index.x * _thumbnail_length,
                                          index.y * _thumbnail_length, 0,
                                          _upload_buffer.get(),
                                          {.offset = _upload_offsets[_device.frame_index()],
                                           .format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                                           .width = _thumbnail_length,
                                           .height = _thumbnail_length,
                                           .row_pitch = _upload_readback_pitch});

      [[likely]] if (_device.supports_enhanced_barriers()) {
         command_list.deferred_barrier(
            gpu::texture_barrier{.sync_before = gpu::barrier_sync::copy,
                                 .sync_after = gpu::barrier_sync::pixel_shading,
                                 .access_before = gpu::barrier_access::copy_dest,
                                 .access_after = gpu::barrier_access::shader_resource,
                                 .layout_before = gpu::barrier_layout::direct_queue_common,
                                 .layout_after = gpu::barrier_layout::direct_queue_common,
                                 .resource = _atlas_texture.get()});
      }
      else {
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _atlas_texture.get(),
            .state_before = gpu::legacy_resource_state::copy_dest,
            .state_after = gpu::legacy_resource_state::pixel_shader_resource});
      }
   }

   void draw(const model& model, const thumbnail_index index,
             root_signature_library& root_signatures, pipeline_library& pipelines,
             dynamic_buffer_allocator& dynamic_buffer_allocator,
             gpu::graphics_command_list& command_list)
   {
      command_list.clear_render_target_view(_render_rtv.get(),
                                            float4{0.0f, 0.0f, 0.0f, 0.0f});
      command_list.clear_depth_stencil_view(_depth_dsv.get(),
                                            {.clear_depth = true}, 0.0f, 0x0);

      command_list.rs_set_viewports(
         gpu::viewport{.width = static_cast<float>(_thumbnail_length * aa_factor),
                       .height = static_cast<float>(_thumbnail_length * aa_factor)});
      command_list.rs_set_scissor_rects({.right = _thumbnail_length * aa_factor,
                                         .bottom = _thumbnail_length * aa_factor});
      command_list.om_set_render_targets(_render_rtv.get(), _depth_dsv.get());

      command_list.set_graphics_root_signature(root_signatures.thumbnail_mesh.get());

      const camera_info camera = make_camera_info(model);

      command_list.set_graphics_32bit_constants(rs::thumbnail_mesh::camera_position,
                                                std::as_bytes(std::span{&camera.camera_position,
                                                                        1}),
                                                0);
      command_list.set_graphics_cbv(rs::thumbnail_mesh::camera_matrix_cbv,
                                    dynamic_buffer_allocator
                                       .allocate_and_copy(camera.projection_from_world)
                                       .gpu_address);

      command_list.ia_set_index_buffer(model.gpu_buffer.index_buffer_view);
      command_list.ia_set_vertex_buffers(
         0, std::array{model.gpu_buffer.position_vertex_buffer_view,
                       model.gpu_buffer.attributes_vertex_buffer_view});
      command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

      std::optional<thumbnail_mesh_pipeline_flags> current_pipeline_flags;

      for (const auto& part : model.parts) {
         const auto part_flags = part.material.thumbnail_mesh_flags();

         if (are_flags_set(part_flags, thumbnail_mesh_pipeline_flags::transparent)) {
            continue;
         }

         if (current_pipeline_flags != part_flags) {
            command_list.set_pipeline_state(pipelines.thumbnail_mesh[part_flags].get());

            current_pipeline_flags = part_flags;
         }

         command_list.set_graphics_cbv(rs::thumbnail_mesh::material_cbv,
                                       part.material.constant_buffer_view);
         command_list.draw_indexed_instanced(part.index_count, 1, part.start_index,
                                             part.start_vertex, 0);
      }

      for (const auto& part : model.parts) {
         const auto part_flags = part.material.thumbnail_mesh_flags();

         if (not are_flags_set(part_flags, thumbnail_mesh_pipeline_flags::transparent)) {
            continue;
         }

         if (current_pipeline_flags != part_flags) {
            command_list.set_pipeline_state(pipelines.thumbnail_mesh[part_flags].get());

            current_pipeline_flags = part_flags;
         }

         command_list.set_graphics_cbv(rs::thumbnail_mesh::material_cbv,
                                       part.material.constant_buffer_view);
         command_list.draw_indexed_instanced(part.index_count, 1, part.start_index,
                                             part.start_vertex, 0);
      }

      [[likely]] if (_device.supports_enhanced_barriers()) {
         command_list.deferred_barrier(
            gpu::texture_barrier{.sync_before = gpu::barrier_sync::render_target,
                                 .sync_after = gpu::barrier_sync::pixel_shading,
                                 .access_before = gpu::barrier_access::render_target,
                                 .access_after = gpu::barrier_access::shader_resource,
                                 .layout_before = gpu::barrier_layout::render_target,
                                 .layout_after = gpu::barrier_layout::direct_queue_shader_resource,
                                 .resource = _render_texture.get()});
         command_list.deferred_barrier(
            gpu::texture_barrier{.sync_before = gpu::barrier_sync::none,
                                 .sync_after = gpu::barrier_sync::render_target,
                                 .access_before = gpu::barrier_access::no_access,
                                 .access_after = gpu::barrier_access::render_target,
                                 .layout_before = gpu::barrier_layout::direct_queue_copy_source,
                                 .layout_after = gpu::barrier_layout::render_target,
                                 .resource = _downsample_texture.get()});
      }
      else {
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _render_texture.get(),
            .state_before = gpu::legacy_resource_state::render_target,
            .state_after = gpu::legacy_resource_state::pixel_shader_resource});
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _downsample_texture.get(),
            .state_before = gpu::legacy_resource_state::copy_source,
            .state_after = gpu::legacy_resource_state::render_target});
      }

      command_list.flush_barriers();

      command_list.clear_render_target_view(_downsample_rtv.get(),
                                            float4{0.0f, 0.0f, 0.0f, 0.0f});

      command_list.rs_set_viewports(
         gpu::viewport{.width = static_cast<float>(_thumbnail_length),
                       .height = static_cast<float>(_thumbnail_length)});
      command_list.rs_set_scissor_rects(
         {.right = _thumbnail_length, .bottom = _thumbnail_length});
      command_list.om_set_render_targets(_downsample_rtv.get());

      command_list.set_graphics_root_signature(
         root_signatures.thumbnail_downsample.get());
      command_list.set_graphics_32bit_constant(rs::thumbnail_downsample::thumbnail,
                                               _render_srv.get().index, 0);

      command_list.set_pipeline_state(pipelines.thumbnail_downsample.get());

      command_list.draw_instanced(3, 1, 0, 0);

      [[likely]] if (_device.supports_enhanced_barriers()) {
         command_list.deferred_barrier(
            gpu::texture_barrier{.sync_before = gpu::barrier_sync::pixel_shading,
                                 .sync_after = gpu::barrier_sync::none,
                                 .access_before = gpu::barrier_access::shader_resource,
                                 .access_after = gpu::barrier_access::no_access,
                                 .layout_before = gpu::barrier_layout::direct_queue_shader_resource,
                                 .layout_after = gpu::barrier_layout::render_target,
                                 .resource = _render_texture.get()});
         command_list.deferred_barrier(
            gpu::texture_barrier{.sync_before = gpu::barrier_sync::render_target,
                                 .sync_after = gpu::barrier_sync::copy,
                                 .access_before = gpu::barrier_access::render_target,
                                 .access_after = gpu::barrier_access::copy_source,
                                 .layout_before = gpu::barrier_layout::render_target,
                                 .layout_after = gpu::barrier_layout::direct_queue_copy_source,
                                 .resource = _downsample_texture.get()});
      }
      else {
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _render_texture.get(),
            .state_before = gpu::legacy_resource_state::pixel_shader_resource,
            .state_after = gpu::legacy_resource_state::render_target});
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _downsample_texture.get(),
            .state_before = gpu::legacy_resource_state::render_target,
            .state_after = gpu::legacy_resource_state::copy_source});
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _atlas_texture.get(),
            .state_before = gpu::legacy_resource_state::pixel_shader_resource,
            .state_after = gpu::legacy_resource_state::copy_dest});
      }

      command_list.flush_barriers();

      command_list.copy_texture_region(_atlas_texture.get(), 0,
                                       index.x * _thumbnail_length,
                                       index.y * _thumbnail_length, 0,
                                       _downsample_texture.get(), 0);
      command_list.copy_texture_to_buffer(_readback_buffer.get(),
                                          {.offset = _readback_offsets[_device.frame_index()],
                                           .format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                                           .width = _thumbnail_length,
                                           .height = _thumbnail_length,
                                           .row_pitch = _upload_readback_pitch},
                                          0, 0, 0, _downsample_texture.get(), 0);

      [[likely]] if (_device.supports_enhanced_barriers()) {
         command_list.deferred_barrier(
            gpu::texture_barrier{.sync_before = gpu::barrier_sync::copy,
                                 .sync_after = gpu::barrier_sync::pixel_shading,
                                 .access_before = gpu::barrier_access::copy_dest,
                                 .access_after = gpu::barrier_access::shader_resource,
                                 .layout_before = gpu::barrier_layout::direct_queue_common,
                                 .layout_after = gpu::barrier_layout::direct_queue_common,
                                 .resource = _atlas_texture.get()});
      }
      else {
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _atlas_texture.get(),
            .state_before = gpu::legacy_resource_state::copy_dest,
            .state_after = gpu::legacy_resource_state::pixel_shader_resource});
      }
   }

   void enqueue_create_thumbnail(const std::string_view name)
   {
      {
         std::scoped_lock lock{_missing_model_odfs_mutex, _pending_render_mutex};

         if (_missing_model_odfs.contains(name)) return;
         if (_pending_render.contains(name)) return;
      }

      asset_ref odf_asset_ref = _asset_libraries.odfs[lowercase_string{name}];

      if (not odf_asset_ref.exists()) return;

      asset_data odf_data = odf_asset_ref.get_if();

      if (odf_data) {
         odf_loaded(name, odf_asset_ref, odf_data);
      }
      else {
         std::scoped_lock lock{_pending_odfs_mutex};

         _pending_odfs.emplace(std::string{name}, std::move(odf_asset_ref));
      }
   }

   void odf_loaded(const std::string_view name, asset_ref<assets::odf::definition> ref,
                   asset_data<assets::odf::definition> definition) noexcept
   {
      const lowercase_string model_name = [&] {
         if (string::iends_with(definition->header.geometry_name, ".msh"sv)) {
            return lowercase_string{definition->header.geometry_name.substr(
               0, definition->header.geometry_name.size() - ".msh"sv.size())};
         }
         else {
            return lowercase_string{definition->header.geometry_name};
         }
      }();

      if (model_name.empty()) {
         std::scoped_lock lock{_missing_model_odfs_mutex};

         _missing_model_odfs.emplace(name);

         return;
      }

      std::scoped_lock lock{_pending_render_mutex, _pending_odfs_mutex,
                            _missing_model_odfs_mutex};

      _pending_odfs.erase(name);
      _pending_render.emplace(name,
                              pending_render{model_name,
                                             _asset_libraries.models[model_name]});
      _missing_model_odfs.erase(name);
   }

   auto get_or_allocate_thumbnail_index(const std::string_view name) noexcept
      -> std::optional<thumbnail_index>
   {
      if (auto it = _back_items.find(name); it != _back_items.end()) {
         return it->second;
      }

      if (auto it = _front_items.find(name); it != _front_items.end()) {
         return it->second;
      }

      if (not _free_items.empty()) {
         const thumbnail_index thumbnail_index = _free_items.back();

         _free_items.pop_back();
         _front_items.emplace(name, thumbnail_index);

         return thumbnail_index;
      }

      if (not _recycle_items.empty()) {
         uint64 oldest_frame = UINT64_MAX;
         decltype(_recycle_items)::iterator recycle_it = _recycle_items.begin();

         for (auto it = _recycle_items.begin(); it != _recycle_items.end(); ++it) {
            const auto& [_, item] = *it;

            if (item.last_used_frame < oldest_frame) {
               oldest_frame = item.last_used_frame;
               recycle_it = it;
            }
         }

         const thumbnail_index thumbnail_index = recycle_it->second.index;

         _recycle_items.erase(recycle_it);
         _front_items.emplace(name, thumbnail_index);

         return thumbnail_index;
      }

      return std::nullopt;
   }

   void create_gpu_resources()
   {
      const uint32 atlas_items_width = max_texture_length / _thumbnail_length;
      const uint32 atlas_items_height =
         std::min((atlas_thumbnails + (atlas_items_width - 1)) / atlas_items_width,
                  max_texture_length / _thumbnail_length);

      _atlas_texture =
         {_device.create_texture({.dimension = gpu::texture_dimension::t_2d,
                                  .format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                                  .width = atlas_items_width * _thumbnail_length,
                                  .height = atlas_items_height * _thumbnail_length,
                                  .debug_name = "Thumbnails Atlas"},
                                 gpu::barrier_layout::direct_queue_common,
                                 gpu::legacy_resource_state::pixel_shader_resource),
          _device.direct_queue};
      _atlas_srv = {_device.create_shader_resource_view(_atlas_texture.get(),
                                                        {.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB}),
                    _device.direct_queue};

      _free_items.clear();
      _free_items.reserve(atlas_items_width * atlas_items_height);

      for (uint16 y = 0; y < atlas_items_height; ++y) {
         for (uint16 x = 0; x < atlas_items_width; ++x) {
            _free_items.push_back({x, y});
         }
      }

      _atlas_items_width = static_cast<float>(atlas_items_width);
      _atlas_items_height = static_cast<float>(atlas_items_height);

      _render_texture = {_device.create_texture(
                            {.dimension = gpu::texture_dimension::t_2d,
                             .flags = {.allow_render_target = true},
                             .format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                             .width = _thumbnail_length * aa_factor,
                             .height = _thumbnail_length * aa_factor,
                             .optimized_clear_value = {.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                                                       .color = {0.0f, 0.0f, 0.0f, 0.0f}},
                             .debug_name = "Thumbnails Render Target"},
                            gpu::barrier_layout::render_target,
                            gpu::legacy_resource_state::render_target),
                         _device.direct_queue};
      _render_rtv = {_device.create_render_target_view(_render_texture.get(),
                                                       {.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                                                        .dimension =
                                                           gpu::rtv_dimension::texture2d}),
                     _device.direct_queue};
      _render_srv = {_device.create_shader_resource_view(_render_texture.get(),
                                                         {.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB}),
                     _device.direct_queue};

      _depth_texture = {_device.create_texture(
                           {.dimension = gpu::texture_dimension::t_2d,
                            .flags = {.allow_depth_stencil = true,
                                      .deny_shader_resource = true},
                            .format = DXGI_FORMAT_D16_UNORM,
                            .width = _thumbnail_length * aa_factor,
                            .height = _thumbnail_length * aa_factor,
                            .optimized_clear_value = {.format = DXGI_FORMAT_D16_UNORM,
                                                      .depth_stencil = {.depth = 0.0f}},
                            .debug_name = "Thumbnails Depth Buffer"},
                           gpu::barrier_layout::depth_stencil_write,
                           gpu::legacy_resource_state::depth_write),
                        _device.direct_queue};
      _depth_dsv = {_device.create_depth_stencil_view(_depth_texture.get(),
                                                      {.format = DXGI_FORMAT_D16_UNORM,
                                                       .dimension =
                                                          gpu::dsv_dimension::texture2d}),
                    _device.direct_queue};

      _downsample_texture =
         {_device.create_texture(
             {.dimension = gpu::texture_dimension::t_2d,
              .flags = {.allow_render_target = true},
              .format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
              .width = _thumbnail_length,
              .height = _thumbnail_length,
              .optimized_clear_value = {.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                                        .color = {0.0f, 0.0f, 0.0f, 0.0f}},
              .debug_name = "Thumbnails Downsample Render Target"},
             gpu::barrier_layout::direct_queue_copy_source,
             gpu::legacy_resource_state::copy_source),
          _device.direct_queue};
      _downsample_rtv = {_device.create_render_target_view(
                            _downsample_texture.get(),
                            {.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                             .dimension = gpu::rtv_dimension::texture2d}),
                         _device.direct_queue};

      _upload_readback_pitch =
         math::align_up<uint32>(_thumbnail_length * sizeof(uint32),
                                gpu::texture_data_pitch_alignment);

      _upload_buffer = {_device.create_buffer({.size = _thumbnail_length * _upload_readback_pitch *
                                                       gpu::frame_pipeline_length,
                                               .debug_name =
                                                  "Thumbnails Upload Buffer"},
                                              gpu::heap_type::upload),
                        _device.direct_queue};

      std::byte* const upload_mapped_buffer = static_cast<std::byte*>(
         _device.map(_upload_buffer.get(), 0,
                     {0, _thumbnail_length * _upload_readback_pitch *
                            gpu::frame_pipeline_length}));

      for (uint32 i = 0; i < gpu::frame_pipeline_length; ++i) {
         _upload_offsets[i] = (_thumbnail_length * _upload_readback_pitch) * i;
         _upload_pointers[i] = upload_mapped_buffer + _upload_offsets[i];
      }

      _readback_buffer =
         {_device.create_buffer({.size = _thumbnail_length * _upload_readback_pitch *
                                         gpu::frame_pipeline_length,
                                 .debug_name = "Thumbnails Readback Buffer"},
                                gpu::heap_type::readback),
          _device.direct_queue};

      const std::byte* const readback_mapped_buffer = static_cast<std::byte*>(
         _device.map(_readback_buffer.get(), 0,
                     {0, _thumbnail_length * _upload_readback_pitch *
                            gpu::frame_pipeline_length}));

      for (uint32 i = 0; i < gpu::frame_pipeline_length; ++i) {
         _readback_offsets[i] = (_thumbnail_length * _upload_readback_pitch) * i;
         _readback_pointers[i] = readback_mapped_buffer + _readback_offsets[i];
      }
   }

   std::shared_ptr<async::thread_pool> _thread_pool;
   assets::libraries_manager& _asset_libraries;
   output_stream& _error_output;
   gpu::device& _device;

   bool _cpu_cache_dirty = false;
   float _atlas_items_width = 1;
   float _atlas_items_height = 1;
   float _display_scale = 1.0f;

   uint32 _thumbnail_length = static_cast<uint32>(128 * _display_scale);

   uint64 _frame = 0;

   gpu::unique_resource_handle _atlas_texture;
   gpu::unique_resource_view _atlas_srv;

   gpu::unique_resource_handle _render_texture;
   gpu::unique_rtv_handle _render_rtv;
   gpu::unique_resource_view _render_srv;

   gpu::unique_resource_handle _depth_texture;
   gpu::unique_dsv_handle _depth_dsv;

   gpu::unique_resource_handle _downsample_texture;
   gpu::unique_rtv_handle _downsample_rtv;

   uint32 _upload_readback_pitch = 0;

   gpu::unique_resource_handle _upload_buffer;
   std::array<uint32, gpu::frame_pipeline_length> _upload_offsets;
   std::array<std::byte*, gpu::frame_pipeline_length> _upload_pointers;

   gpu::unique_resource_handle _readback_buffer;
   std::array<uint32, gpu::frame_pipeline_length> _readback_offsets;
   std::array<const std::byte*, gpu::frame_pipeline_length> _readback_pointers;
   std::array<std::string, gpu::frame_pipeline_length> _readback_names;

   struct cache_upload_item {
      thumbnail_index index;
   };

   std::optional<cache_upload_item> _cache_upload_item;

   struct rendering {
      std::string name;
      lowercase_string model_name;
      asset_ref<assets::msh::flat_model> model;
   };

   std::optional<rendering> _rendering;

   std::vector<thumbnail_index> _free_items;
   absl::flat_hash_map<std::string, thumbnail_index> _front_items;
   absl::flat_hash_map<std::string, thumbnail_index> _back_items;

   struct recycle_item {
      thumbnail_index index;
      uint64 last_used_frame;
   };

   absl::flat_hash_map<std::string, recycle_item> _recycle_items;

   absl::flat_hash_map<std::string, std::unique_ptr<std::byte[]>> _cpu_memory_cache;

   std::shared_mutex _pending_odfs_mutex;
   absl::flat_hash_map<std::string, asset_ref<assets::odf::definition>> _pending_odfs;

   struct pending_render {
      lowercase_string model_name;
      asset_ref<assets::msh::flat_model> model;
   };

   std::shared_mutex _pending_render_mutex;
   absl::flat_hash_map<std::string, pending_render> _pending_render;

   std::shared_mutex _missing_model_odfs_mutex;
   absl::flat_hash_set<std::string> _missing_model_odfs;

   invalidation_tracker _invalidation_tracker{_asset_libraries};

   std::optional<async::task<void>> _save_disk_cache_task;
   std::optional<async::task<loaded_cache>> _load_disk_cache_task;
   std::optional<async::task<void>> _restore_invalidation_tracker_task;

   event_listener<void(const lowercase_string&, asset_ref<assets::odf::definition>,
                       asset_data<assets::odf::definition>)>
      _asset_load_listener = _asset_libraries.odfs.listen_for_loads(
         [this](const lowercase_string& name,
                const asset_ref<assets::odf::definition>& ref,
                const asset_data<assets::odf::definition>& data) {
            {
               std::shared_lock lock{_pending_odfs_mutex};

               if (not _pending_odfs.contains(name)) return;
            }

            odf_loaded(name, ref, data);
         });
};

auto thumbnail_manager::request_object_class_thumbnail(const std::string_view name)
   -> object_class_thumbnail
{
   return _impl->request_object_class_thumbnail(name);
}

void thumbnail_manager::update_cpu_cache()
{
   return _impl->update_cpu_cache();
}

void thumbnail_manager::update_gpu(model_manager& model_manager,
                                   root_signature_library& root_signature_library,
                                   pipeline_library& pipeline_library,
                                   dynamic_buffer_allocator& dynamic_buffer_allocator,
                                   gpu::graphics_command_list& command_list)
{
   return _impl->update_gpu(model_manager, root_signature_library, pipeline_library,
                            dynamic_buffer_allocator, command_list);
}

void thumbnail_manager::end_frame()
{
   return _impl->end_frame();
}

void thumbnail_manager::display_scale_changed(const float new_display_scale)
{
   return _impl->display_scale_changed(new_display_scale);
}

void thumbnail_manager::async_save_disk_cache(const char* path) noexcept
{
   return _impl->async_save_disk_cache(path);
}

void thumbnail_manager::async_load_disk_cache(const char* path) noexcept
{
   return _impl->async_load_disk_cache(path);
}

void thumbnail_manager::reset() noexcept
{
   return _impl->reset();
}

thumbnail_manager::thumbnail_manager(const thumbnail_manager_init& init)
   : _impl{init}
{
}

thumbnail_manager::~thumbnail_manager() = default;
}