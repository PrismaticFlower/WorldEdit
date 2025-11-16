#include "thumbnail_manager.hpp"

#include "dynamic_buffer_allocator.hpp"
#include "gpu/resource.hpp"
#include "model_manager.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"

#include "assets/asset_libraries.hpp"
#include "assets/odf/definition.hpp"
#include "assets/texture/texture.hpp"

#include "async/thread_pool.hpp"

#include "io/error.hpp"
#include "io/memory_mapped_file.hpp"
#include "io/read_file.hpp"

#include "math/align.hpp"
#include "math/matrix_funcs.hpp"

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

constexpr uint32 cache_save_version = 2;
constexpr uint32 max_texture_length = 16384;
constexpr uint32 atlas_thumbnails = 256;
constexpr uint32 cpu_cache_reservation = 2048;
constexpr uint32 aa_factor = 4;
constexpr float thumbnail_base_length = 128.0f;

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

struct objects_invalidation_save_entry {
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

struct objects_invalidation_tracker {
   explicit objects_invalidation_tracker(assets::libraries_manager& asset_libraries)
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

   auto get_save_data() noexcept -> std::vector<objects_invalidation_save_entry>
   {
      std::scoped_lock lock{_mutex};

      std::vector<objects_invalidation_save_entry> save_data;
      save_data.reserve(_odfs.size());

      for (const auto& [name, entry] : _odfs) {
         using name_time = objects_invalidation_save_entry::name_time;

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

   void restore_from_save_data(const std::span<const objects_invalidation_save_entry> save_data) noexcept
   {
      {
         std::scoped_lock lock{_mutex};

         _odfs.reserve(save_data.size());
      }

      for (const objects_invalidation_save_entry& entry : save_data) {
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

using textures_invalidation_save_entry = objects_invalidation_save_entry::name_time;

struct textures_invalidation_tracker {
   explicit textures_invalidation_tracker(assets::libraries_manager& asset_libraries)
      : _texture_change_listener{asset_libraries.textures.listen_for_changes(
           [this](const lowercase_string& name) { texture_changed(name); })},
        _asset_libraries{asset_libraries}
   {
   }

   void track(std::string_view texture_name) noexcept
   {
      std::scoped_lock lock{_mutex};

      _textures.emplace(texture_name);
   }

   auto get_invalidated() -> std::span<std::string>
   {
      _invalidated.clear();

      std::scoped_lock lock{_invalidated_background_mutex};

      std::swap(_invalidated, _invalidated_background);

      return _invalidated;
   }

   auto get_save_data() noexcept -> std::vector<textures_invalidation_save_entry>
   {
      std::scoped_lock lock{_mutex};

      std::vector<textures_invalidation_save_entry> save_data;
      save_data.reserve(_textures.size());

      for (const lowercase_string& name : _textures) {
         save_data.emplace_back(name, _asset_libraries.textures);
      }

      return save_data;
   }

   void restore_from_save_data(const std::span<const textures_invalidation_save_entry> save_data) noexcept
   {
      {
         std::scoped_lock lock{_mutex};

         _textures.reserve(save_data.size());
      }

      for (const textures_invalidation_save_entry& entry : save_data) {
         if (entry.invalidated(_asset_libraries.textures)) {
            std::scoped_lock lock{_mutex, _invalidated_background_mutex};

            if (not _textures.contains(entry.name)) {
               _invalidated_background.push_back(entry.name);
            }

            continue;
         }

         std::scoped_lock lock{_mutex};

         _textures.emplace(entry.name);
      }
   }

   void clear() noexcept
   {
      _invalidated.clear();

      std::scoped_lock lock{_mutex, _invalidated_background_mutex};

      _textures.clear();
      _invalidated_background.clear();
   }

private:
   void texture_changed(const lowercase_string& name) noexcept
   {
      {
         std::shared_lock lock{_mutex};

         if (not _textures.contains(name)) return;
      }

      std::scoped_lock lock{_invalidated_background_mutex};

      _invalidated_background.push_back(name);
   }

   std::vector<std::string> _invalidated;

   std::shared_mutex _mutex;
   absl::flat_hash_set<lowercase_string> _textures;

   std::shared_mutex _invalidated_background_mutex;
   std::vector<std::string> _invalidated_background;

   assets::libraries_manager& _asset_libraries;

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

struct disk_cache {
   uint32 thumbnail_length = 0;

   absl::flat_hash_map<std::string, std::unique_ptr<std::byte[]>> objects_cpu_memory_cache;
   std::vector<objects_invalidation_save_entry> objects_invalidation_save_data;

   absl::flat_hash_map<std::string, std::unique_ptr<std::byte[]>> textures_cpu_memory_cache;
   std::vector<textures_invalidation_save_entry> textures_invalidation_save_data;
};

void save_disk_cache(const io::path& path, const disk_cache& cache) noexcept
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
         write_object(cache.thumbnail_length);
         write_object(static_cast<uint32>(sizeof(std::size_t)));

         write_object(cache.objects_cpu_memory_cache.size());

         for (const auto& [name, data] : cache.objects_cpu_memory_cache) {
            write_object(name.size());
            write_string(name);
            write_bytes(std::span{data.get(), cache.thumbnail_length * cache.thumbnail_length *
                                                 sizeof(uint32)});
         }

         write_object(cache.objects_invalidation_save_data.size());

         for (const objects_invalidation_save_entry& entry :
              cache.objects_invalidation_save_data) {
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

         write_object(cache.textures_cpu_memory_cache.size());

         for (const auto& [name, data] : cache.textures_cpu_memory_cache) {
            write_object(name.size());
            write_string(name);
            write_bytes(std::span{data.get(), cache.thumbnail_length * cache.thumbnail_length *
                                                 sizeof(uint32)});
         }

         write_object(cache.textures_invalidation_save_data.size());

         for (const textures_invalidation_save_entry& entry :
              cache.textures_invalidation_save_data) {
            write_object(entry.name.size());
            write_string(entry.name);
            write_object(entry.time);
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

auto load_disk_cache(const io::path& path) noexcept -> disk_cache
{
   try {
      disk_cache loaded;

      const std::vector<std::byte> bytes = io::read_file_to_bytes(path);

      utility::binary_reader cache{bytes};

      const uint32 version = cache.read<uint32>();
      const uint32 thumbnail_length = cache.read<uint32>();
      const uint32 size_t_size = cache.read<uint32>();
      const uint32 thumbnail_size =
         thumbnail_length * thumbnail_length * sizeof(uint32);

      if (version != cache_save_version) return {};
      if (size_t_size != sizeof(std::size_t)) return {};

      loaded.thumbnail_length = thumbnail_length;

      const std::size_t cached_object_thumbnails = cache.read<std::size_t>();

      loaded.objects_cpu_memory_cache.reserve(cached_object_thumbnails);

      for (std::size_t i = 0; i < cached_object_thumbnails; ++i) {
         const std::span<const std::byte> name_bytes =
            cache.read_bytes(cache.read<std::size_t>());
         const std::string_view name{reinterpret_cast<const char*>(name_bytes.data()),
                                     name_bytes.size()};

         const std::span<const std::byte> thumbnail = cache.read_bytes(thumbnail_size);

         std::unique_ptr<std::byte[]>& memory = loaded.objects_cpu_memory_cache[name];

         if (not memory) {
            memory = std::make_unique_for_overwrite<std::byte[]>(thumbnail_size);
         }

         std::memcpy(memory.get(), thumbnail.data(), thumbnail_size);
      }

      const std::size_t cached_object_invalidation_entries =
         cache.read<std::size_t>();

      loaded.objects_invalidation_save_data.reserve(cached_object_invalidation_entries);

      for (std::size_t entry_index = 0;
           entry_index < cached_object_invalidation_entries; ++entry_index) {
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

         using name_time = objects_invalidation_save_entry::name_time;

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

         loaded.objects_invalidation_save_data
            .emplace_back(name_time{odf, odf_last_write_time},
                          name_time{model, model_last_write_time},
                          std::move(textures));
      }

      const std::size_t cached_texture_thumbnails = cache.read<std::size_t>();

      loaded.textures_cpu_memory_cache.reserve(cached_texture_thumbnails);

      for (std::size_t i = 0; i < cached_texture_thumbnails; ++i) {
         const std::span<const std::byte> name_bytes =
            cache.read_bytes(cache.read<std::size_t>());
         const std::string_view name{reinterpret_cast<const char*>(name_bytes.data()),
                                     name_bytes.size()};

         const std::span<const std::byte> thumbnail = cache.read_bytes(thumbnail_size);

         std::unique_ptr<std::byte[]>& memory =
            loaded.textures_cpu_memory_cache[name];

         if (not memory) {
            memory = std::make_unique_for_overwrite<std::byte[]>(thumbnail_size);
         }

         std::memcpy(memory.get(), thumbnail.data(), thumbnail_size);
      }

      const std::size_t cached_texture_invalidation_entries =
         cache.read<std::size_t>();

      loaded.textures_invalidation_save_data.reserve(cached_texture_invalidation_entries);

      for (std::size_t entry_index = 0;
           entry_index < cached_texture_invalidation_entries; ++entry_index) {
         const std::span<const std::byte> name_bytes =
            cache.read_bytes(cache.read<std::size_t>());
         const std::string_view name{reinterpret_cast<const char*>(name_bytes.data()),
                                     name_bytes.size()};
         const uint64 last_write_time = cache.read<uint64>();

         loaded.textures_invalidation_save_data.emplace_back(std::string{name},
                                                             last_write_time);
      }

      return loaded;
   }
   catch (std::runtime_error&) {
      return {};
   }
}

struct thumbnail_index {
   uint16 x = 0;
   uint16 y = 0;
};

struct thumbnail_manager_memory_cache {
   explicit thumbnail_manager_memory_cache(uint32 thumbnail_length)
      : _thumbnail_length{thumbnail_length} {};

   void store_object(const std::string_view name,
                     std::unique_ptr<std::byte[]> thumbnail) noexcept
   {
      if (auto existing = _objects.find(name); existing != _objects.end()) {
         existing->second = std::move(thumbnail);
      }
      else {
         _objects.emplace(std::move(name), std::move(thumbnail));
      }
   }

   auto load_object(const std::string_view name) const noexcept
      -> std::span<const std::byte>
   {
      if (auto existing = _objects.find(name); existing != _objects.end()) {
         return {existing->second.get(),
                 _thumbnail_length * _thumbnail_length * sizeof(uint32)};
      }

      return {};
   }

   void clear()
   {
      _objects.clear();
   }

private:
   uint32 _thumbnail_length = 0;

   absl::flat_hash_map<std::string, std::unique_ptr<std::byte[]>> _objects;
};

struct thumbnail_manager_atlas_allocator {
   /// @brief Reset the allocator.
   /// @param width The new atlas width.
   /// @param height The new atlas height.
   void reset(uint32 width, uint32 height) noexcept
   {
      _free_items.clear();
      _recycle_objects.clear();

      _free_items.reserve(width * height);
      _recycle_objects.reserve(width * height);

      for (uint16 y = 0; y < height; ++y) {
         for (uint16 x = 0; x < width; ++x) {
            _free_items.push_back({x, y});
         }
      }
   }

   /// @brief Allocate an index from the atlas.
   /// @return The index or nullopt if the atlas is full.
   auto allocate() noexcept -> std::optional<thumbnail_index>
   {
      if (not _free_items.empty()) {
         const thumbnail_index thumbnail_index = _free_items.back();

         _free_items.pop_back();

         return thumbnail_index;
      }

      if (not _recycle_objects.empty()) {
         uint64 oldest_frame = UINT64_MAX;
         decltype(_recycle_objects)::iterator recycle_it = _recycle_objects.begin();

         for (auto it = _recycle_objects.begin(); it != _recycle_objects.end(); ++it) {
            const auto& [_, item] = *it;

            if (item.last_used_frame < oldest_frame) {
               oldest_frame = item.last_used_frame;
               recycle_it = it;
            }
         }

         const thumbnail_index thumbnail_index = recycle_it->second.index;

         _recycle_objects.erase(recycle_it);

         return thumbnail_index;
      }

      return std::nullopt;
   }

   /// @brief Free an index in the atlas.
   /// @param index The atlas to free.
   void free(thumbnail_index index) noexcept
   {
      _free_items.push_back(index);
   }

   /// @brief Try to restore a previously recycled object.
   /// @param name The name of the object.
   /// @return The index of the object's thumbnail or nullopt if it has already been overwritten.
   auto try_restore_object(const std::string_view name) noexcept
      -> std::optional<thumbnail_index>
   {
      if (auto it = _recycle_objects.find(name); it != _recycle_objects.end()) {
         const thumbnail_index thumbnail_index = it->second.index;

         _recycle_objects.erase(it);

         return thumbnail_index;
      }

      return std::nullopt;
   }

   /// @brief Recycle an object thumbnail. Allowing it to be reused but potentially restored at later date/
   /// @param name The name of the object.
   /// @param index The index of the thumbnail.
   /// @param last_used_frame The frame the thumbnail was last used on.
   void recycle_object(const std::string_view name, thumbnail_index index,
                       uint64 last_used_frame) noexcept
   {
      if (not _recycle_objects
                 .emplace(name, recycle_item{index, last_used_frame})
                 .second) {
         _free_items.push_back(index);
      }
   }

   /// @brief Invalidate a previously recycled thumbnail.
   /// @param name The name of the object whose thumbnail to invalidate.
   void invalidate_recycling_object(const std::string_view name) noexcept
   {
      if (auto it = _recycle_objects.find(name); it != _recycle_objects.end()) {
         const thumbnail_index index = it->second.index;

         _free_items.push_back(index);
         _recycle_objects.erase(it);
      }
   }

   /// @brief Try to restore a previously recycled texture.
   /// @param name The name of the texture.
   /// @return The index of the texture's thumbnail or nullopt if it has already been overwritten.
   auto try_restore_texture(const std::string_view name) noexcept
      -> std::optional<thumbnail_index>
   {
      if (auto it = _recycle_textures.find(name); it != _recycle_textures.end()) {
         const thumbnail_index thumbnail_index = it->second.index;

         _recycle_textures.erase(it);

         return thumbnail_index;
      }

      return std::nullopt;
   }

   /// @brief Recycle an texture thumbnail. Allowing it to be reused but potentially restored at later date/
   /// @param name The name of the texture.
   /// @param index The index of the thumbnail.
   /// @param last_used_frame The frame the thumbnail was last used on.
   void recycle_texture(const std::string_view name, thumbnail_index index,
                        uint64 last_used_frame) noexcept
   {
      if (not _recycle_textures
                 .emplace(name, recycle_item{index, last_used_frame})
                 .second) {
         _free_items.push_back(index);
      }
   }

   /// @brief Invalidate a previously recycled thumbnail.
   /// @param name The name of the texture whose thumbnail to invalidate.
   void invalidate_recycling_texture(const std::string_view name) noexcept
   {
      if (auto it = _recycle_textures.find(name); it != _recycle_textures.end()) {
         const thumbnail_index index = it->second.index;

         _free_items.push_back(index);
         _recycle_textures.erase(it);
      }
   }

private:
   std::vector<thumbnail_index> _free_items;

   struct recycle_item {
      thumbnail_index index;
      uint64 last_used_frame;
   };

   absl::flat_hash_map<std::string, recycle_item> _recycle_objects;
   absl::flat_hash_map<std::string, recycle_item> _recycle_textures;
};

struct thumbnail_manager_type_items {
   absl::flat_hash_map<std::string, thumbnail_index> front;
   absl::flat_hash_map<std::string, thumbnail_index> back;
   absl::flat_hash_map<std::string, std::unique_ptr<std::byte[]>> cpu_memory_cache;

   thumbnail_manager_type_items() noexcept
   {
      front.reserve(atlas_thumbnails);
      back.reserve(atlas_thumbnails);
      cpu_memory_cache.reserve(cpu_cache_reservation);
   }

   /// @brief Gets the thumbnail index for an item or allocates one if needed. Returns nullopt if there was no space in the atlas.
   /// @param name The name of the item.
   /// @param atlas_allocator The allocator for the atlas.
   /// @return The thumbnail index for the item or nullopt if the atlas is full.
   auto get_or_allocate_thumbnail_index(const std::string_view name,
                                        thumbnail_manager_atlas_allocator& atlas_allocator) noexcept
      -> std::optional<thumbnail_index>
   {
      if (auto it = back.find(name); it != back.end()) {
         return it->second;
      }

      if (auto it = front.find(name); it != front.end()) {
         return it->second;
      }

      if (const std::optional<thumbnail_index> thumbnail_index =
             atlas_allocator.allocate();
          thumbnail_index) {
         front.emplace(name, *thumbnail_index);

         return thumbnail_index;
      }

      return std::nullopt;
   }

   void clear()
   {
      front.clear();
      back.clear();
      cpu_memory_cache.clear();
   }

   /// @brief Swaps back and front then recycle all thumbnails that were unused this frame.
   /// @param atlas_allocator The allocator for the atlas.
   /// @param frame The current frame.
   void end_frame(thumbnail_manager_atlas_allocator& atlas_allocator, uint64 frame) noexcept
   {
      std::swap(back, front);

      for (const auto& [name, index] : front) {
         if (not back.contains(name)) {
            atlas_allocator.recycle_object(name, index, frame);
         }
      }

      front.clear();
   }
};

}

struct thumbnail_manager::impl {
   explicit impl(const thumbnail_manager_init& init)
      : _thread_pool{init.thread_pool},
        _asset_libraries{init.asset_libraries},
        _error_output{init.error_output},
        _device{init.device},
        _display_scale{init.display_scale}
   {
      create_gpu_resources();
   }

   ~impl()
   {
      if (_save_disk_cache_task) _save_disk_cache_task->wait();
   }

   auto request_object_class_thumbnail(const std::string_view name) -> object_class_thumbnail
   {
      if (auto it = _objects.back.find(name); it != _objects.back.end()) {
         const thumbnail_index index = it->second;

         _objects.front.emplace(name, index);

         return {.imgui_texture_id = _atlas_srv.get().index,
                 .uv_left = index.x / _atlas_items_width,
                 .uv_top = index.y / _atlas_items_height,
                 .uv_right = (index.x + 1) / _atlas_items_width,
                 .uv_bottom = (index.y + 1) / _atlas_items_height};
      }

      if (const std::optional<thumbnail_index> index =
             _atlas_allocator.try_restore_object(name);
          index) {
         _objects.back.emplace(name, *index);
         _objects.front.emplace(name, *index);

         return {.imgui_texture_id = _atlas_srv.get().index,
                 .uv_left = index->x / _atlas_items_width,
                 .uv_top = index->y / _atlas_items_height,
                 .uv_right = (index->x + 1) / _atlas_items_width,
                 .uv_bottom = (index->y + 1) / _atlas_items_height};
      }

      if (not _cache_upload_item) {
         if (auto it = _objects.cpu_memory_cache.find(name);
             it != _objects.cpu_memory_cache.end()) {
            if (const std::optional<thumbnail_index> index =
                   _objects.get_or_allocate_thumbnail_index(name, _atlas_allocator);
                index) {
               _cache_upload_item.emplace(*index);

               const std::byte* const src_data = it->second.get();

               texture_copy(_upload_pointers[_device.frame_index()],
                            _upload_readback_pitch, src_data,
                            _thumbnail_length * sizeof(uint32),
                            _thumbnail_length, sizeof(uint32));

               return {.imgui_texture_id = _atlas_srv.get().index,
                       .uv_left = index->x / _atlas_items_width,
                       .uv_top = index->y / _atlas_items_height,
                       .uv_right = (index->x + 1) / _atlas_items_width,
                       .uv_bottom = (index->y + 1) / _atlas_items_height};
            }
         }
      }

      if (not _objects.cpu_memory_cache.contains(name)) {
         enqueue_create_object_thumbnail(name);
      }

      return {.imgui_texture_id = _missing_thumbnail_srv.get().index,
              .uv_left = 0.0f,
              .uv_top = 0.0f,
              .uv_right = 0.0f,
              .uv_bottom = 0.0f};
   }

   auto request_texture_thumbnail(const std::string_view name) -> object_class_thumbnail
   {
      if (auto it = _textures.back.find(name); it != _textures.back.end()) {
         const thumbnail_index index = it->second;

         _textures.front.emplace(name, index);

         return {.imgui_texture_id = _atlas_srv.get().index,
                 .uv_left = index.x / _atlas_items_width,
                 .uv_top = index.y / _atlas_items_height,
                 .uv_right = (index.x + 1) / _atlas_items_width,
                 .uv_bottom = (index.y + 1) / _atlas_items_height};
      }

      if (const std::optional<thumbnail_index> index =
             _atlas_allocator.try_restore_texture(name);
          index) {
         _textures.back.emplace(name, *index);
         _textures.front.emplace(name, *index);

         return {.imgui_texture_id = _atlas_srv.get().index,
                 .uv_left = index->x / _atlas_items_width,
                 .uv_top = index->y / _atlas_items_height,
                 .uv_right = (index->x + 1) / _atlas_items_width,
                 .uv_bottom = (index->y + 1) / _atlas_items_height};
      }

      if (not _cache_upload_item) {
         if (auto it = _textures.cpu_memory_cache.find(name);
             it != _textures.cpu_memory_cache.end()) {
            if (const std::optional<thumbnail_index> index =
                   _textures.get_or_allocate_thumbnail_index(name, _atlas_allocator);
                index) {
               _cache_upload_item.emplace(*index);

               const std::byte* const src_data = it->second.get();

               texture_copy(_upload_pointers[_device.frame_index()],
                            _upload_readback_pitch, src_data,
                            _thumbnail_length * sizeof(uint32),
                            _thumbnail_length, sizeof(uint32));

               return {.imgui_texture_id = _atlas_srv.get().index,
                       .uv_left = index->x / _atlas_items_width,
                       .uv_top = index->y / _atlas_items_height,
                       .uv_right = (index->x + 1) / _atlas_items_width,
                       .uv_bottom = (index->y + 1) / _atlas_items_height};
            }
         }
      }

      if (not _textures.cpu_memory_cache.contains(name)) {
         enqueue_create_texture_thumbnail(name);
      }

      return {.imgui_texture_id = _missing_thumbnail_srv.get().index,
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
            _objects.cpu_memory_cache[_readback_names[_device.frame_index()]];

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
         disk_cache loaded = _load_disk_cache_task->get();

         _load_disk_cache_task = std::nullopt;

         if (loaded.thumbnail_length == _thumbnail_length) {
            _cpu_cache_dirty = false;

            _objects.cpu_memory_cache = std::move(loaded.objects_cpu_memory_cache);

            if (_restore_objects_invalidation_tracker_task) {
               _restore_objects_invalidation_tracker_task->wait();
            }

            _restore_objects_invalidation_tracker_task = _thread_pool->exec(
               [this, objects_invalidation_save_data =
                         std::move(loaded.objects_invalidation_save_data)] {
                  _objects_invalidation_tracker.restore_from_save_data(
                     objects_invalidation_save_data);
               });

            _textures.cpu_memory_cache = std::move(loaded.textures_cpu_memory_cache);

            if (_restore_textures_invalidation_tracker_task) {
               _restore_textures_invalidation_tracker_task->wait();
            }

            _restore_textures_invalidation_tracker_task = _thread_pool->exec(
               [this, textures_invalidation_save_data =
                         std::move(loaded.textures_invalidation_save_data)] {
                  _textures_invalidation_tracker.restore_from_save_data(
                     textures_invalidation_save_data);
               });
         }
      }
   }

   void update_gpu(model_manager& model_manager,
                   root_signature_library& root_signatures, pipeline_library& pipelines,
                   dynamic_buffer_allocator& dynamic_buffer_allocator,
                   gpu::graphics_command_list& command_list)
   {
      update_pending_textures();

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
            _objects.get_or_allocate_thumbnail_index(_rendering->name, _atlas_allocator);

         if (not thumbnail_index) return;

         draw(model, *thumbnail_index, root_signatures, pipelines,
              dynamic_buffer_allocator, command_list);

         _readback_names[_device.frame_index()] = _rendering->name;

         if (status == model_status::ready or
             status == model_status::ready_textures_missing) {
            std::scoped_lock lock{_pending_render_mutex};

            _pending_render.erase(_rendering->name);
            _objects_invalidation_tracker.track(_rendering->name,
                                                _rendering->model_name, model);
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
      _objects.end_frame(_atlas_allocator, _frame);
      _textures.end_frame(_atlas_allocator, _frame);

      _frame += 1;

      for (const auto& name : _objects_invalidation_tracker.get_invalidated()) {
         if (auto it = _objects.back.find(name); it != _objects.back.end()) {
            const thumbnail_index index = it->second;

            _atlas_allocator.free(index);
            _objects.back.erase(it);
         }

         _atlas_allocator.invalidate_recycling_object(name);
         _objects.cpu_memory_cache.erase(name);
      }

      for (const auto& name : _textures_invalidation_tracker.get_invalidated()) {
         if (auto it = _textures.back.find(name); it != _textures.back.end()) {
            const thumbnail_index index = it->second;

            _atlas_allocator.free(index);
            _textures.back.erase(it);
         }

         _atlas_allocator.invalidate_recycling_texture(name);
         _textures.cpu_memory_cache.erase(name);
      }
   }

   void display_scale_changed(const float new_display_scale)
   {
      if (_display_scale == new_display_scale) return;

      _display_scale = new_display_scale;

      _thumbnail_length = static_cast<uint32>(128 * _display_scale);
      _pending_textures_thumbnail_length.store(_thumbnail_length,
                                               std::memory_order_relaxed);

      reset();
   }

   void async_save_disk_cache(const char* path) noexcept
   {
      if (not _cpu_cache_dirty) return;
      if (_objects.cpu_memory_cache.empty() and _textures.cpu_memory_cache.empty()) {
         return;
      }

      if (_save_disk_cache_task) _save_disk_cache_task->wait();

      _save_disk_cache_task = _thread_pool->exec(
         [disk_cache_path = io::path{path},
          cache = disk_cache{.thumbnail_length = _thumbnail_length,
                             .objects_cpu_memory_cache = std::move(_objects.cpu_memory_cache),
                             .objects_invalidation_save_data =
                                _objects_invalidation_tracker.get_save_data(),

                             .textures_cpu_memory_cache = std::move(_textures.cpu_memory_cache),
                             .textures_invalidation_save_data =
                                _textures_invalidation_tracker.get_save_data()}] {
            save_disk_cache(disk_cache_path, cache);
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

      _objects.clear();
      _textures.clear();

      {
         std::scoped_lock lock{_pending_odfs_mutex, _pending_render_mutex,
                               _missing_model_odfs_mutex};

         _pending_odfs.clear();
         _pending_render.clear();
         _missing_model_odfs.clear();
      }

      {
         std::scoped_lock lock{_pending_textures_mutex, _pending_textures_upload_mutex,
                               _failed_textures_mutex};

         _pending_textures.clear();
         _pending_textures_upload.clear();
         _failed_textures.clear();
      }

      _objects_invalidation_tracker.clear();
   }

private:
   void update_pending_textures() noexcept
   {
      if (_cache_upload_item) return;

      std::scoped_lock lock{_pending_textures_upload_mutex};

      for (auto it = _pending_textures_upload.begin();
           it != _pending_textures_upload.end();) {
         auto pending_it = it++;
         auto& [name, pending] = *pending_it;

         if (pending.thumbnail_length != _thumbnail_length) {
            _pending_textures_upload.erase(pending_it);

            continue;
         }

         if (const std::optional<thumbnail_index> thumbnail_index =
                _textures.get_or_allocate_thumbnail_index(name, _atlas_allocator);
             thumbnail_index) {
            _cache_upload_item.emplace(*thumbnail_index);

            const std::byte* const src_data = pending.data.get();

            texture_copy(_upload_pointers[_device.frame_index()], _upload_readback_pitch,
                         src_data, _thumbnail_length * sizeof(uint32),
                         _thumbnail_length, sizeof(uint32));

            _cpu_cache_dirty = true;
            _textures.cpu_memory_cache.emplace(name, std::move(pending.data));
            _textures_invalidation_tracker.track(name);
            _pending_textures_upload.erase(pending_it);
         }

         return;
      }
   }

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

   void enqueue_create_object_thumbnail(const std::string_view name)
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

   void enqueue_create_texture_thumbnail(const std::string_view name)
   {
      {
         std::shared_lock lock{_failed_textures_mutex};

         if (_failed_textures.contains(name)) return;
      }

      {
         std::shared_lock lock{_pending_textures_upload_mutex};

         if (_pending_textures_upload.contains(name)) return;
      }

      asset_ref texture_asset_ref = _asset_libraries.textures[lowercase_string{name}];

      if (not texture_asset_ref.exists()) return;

      asset_data texture_data = texture_asset_ref.get_if();

      if (texture_data) {
         texture_loaded(name, texture_asset_ref, texture_data);
      }
      else {
         std::scoped_lock lock{_pending_textures_mutex};

         _pending_textures.emplace(std::string{name}, std::move(texture_asset_ref));
      }
   }

   void odf_loaded(const std::string_view name,
                   [[maybe_unused]] asset_ref<assets::odf::definition> ref,
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

   void texture_loaded(const std::string_view name,
                       [[maybe_unused]] const asset_ref<assets::texture::texture>& ref,
                       const asset_data<assets::texture::texture>& texture) noexcept
   {
      const uint32 thumbnail_length =
         _pending_textures_thumbnail_length.load(std::memory_order_relaxed);

      pending_texture_upload upload{
         .thumbnail_length = thumbnail_length,
         .data = std::make_unique<std::byte[]>(
            thumbnail_length * thumbnail_length * sizeof(uint32)),
      };

      uint32 mip_level = 0;

      for (; mip_level < texture->mip_levels(); ++mip_level) {
         assets::texture::texture_subresource_view view =
            texture->subresource({.mip_level = mip_level});

         if (thumbnail_length >= std::max(view.width(), view.height())) {
            break;
         }
      }

      assets::texture::texture_subresource_view view =
         texture->subresource({.mip_level = mip_level});

      const uint32 y_dest_offset = (thumbnail_length - view.height()) / 2;
      const uint32 x_dest_offset = (thumbnail_length - view.width()) / 2;
      const uint32 dest_row_pitch = thumbnail_length * sizeof(uint32);

      for (uint32 y = 0; y < view.height(); ++y) {
         const uint32 dest_y = y + y_dest_offset;

         std::memcpy(upload.data.get() + (dest_y * dest_row_pitch) +
                        (x_dest_offset * sizeof(uint32)),
                     view.data() + y * view.row_pitch(),
                     view.width() * sizeof(uint32));
      }

      std::scoped_lock lock{_pending_textures_mutex, _pending_textures_upload_mutex};

      _pending_textures.erase(name);
      _pending_textures_upload.emplace(std::string{name}, std::move(upload));
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
          _device};
      _atlas_srv = {_device.create_shader_resource_view(_atlas_texture.get(),
                                                        {.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB}),
                    _device};

      _atlas_allocator.reset(atlas_items_width, atlas_items_height);

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
                         _device};
      _render_rtv = {_device.create_render_target_view(_render_texture.get(),
                                                       {.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                                                        .dimension =
                                                           gpu::rtv_dimension::texture2d}),
                     _device};
      _render_srv = {_device.create_shader_resource_view(_render_texture.get(),
                                                         {.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB}),
                     _device};

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
                        _device};
      _depth_dsv = {_device.create_depth_stencil_view(_depth_texture.get(),
                                                      {.format = DXGI_FORMAT_D16_UNORM,
                                                       .dimension =
                                                          gpu::dsv_dimension::texture2d}),
                    _device};

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
          _device};
      _downsample_rtv = {_device.create_render_target_view(
                            _downsample_texture.get(),
                            {.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                             .dimension = gpu::rtv_dimension::texture2d}),
                         _device};

      _upload_readback_pitch =
         math::align_up<uint32>(_thumbnail_length * sizeof(uint32),
                                gpu::texture_data_pitch_alignment);

      _upload_buffer = {_device.create_buffer({.size = _thumbnail_length * _upload_readback_pitch *
                                                       gpu::frame_pipeline_length,
                                               .debug_name =
                                                  "Thumbnails Upload Buffer"},
                                              gpu::heap_type::upload),
                        _device};

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
          _device};

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

   uint32 _thumbnail_length =
      static_cast<uint32>(thumbnail_base_length * _display_scale);

   uint64 _frame = 0;

   gpu::unique_resource_handle _atlas_texture;
   gpu::unique_resource_view _atlas_srv;

   gpu::unique_resource_view _missing_thumbnail_srv =
      {_device.create_null_shader_resource_view(gpu::srv_dimension::texture2d,
                                                {.format = DXGI_FORMAT_R8G8B8A8_UNORM}),
       _device};

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

   thumbnail_manager_type_items _objects;
   thumbnail_manager_type_items _textures;
   thumbnail_manager_atlas_allocator _atlas_allocator;

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

   std::atomic_uint32_t _pending_textures_thumbnail_length = _thumbnail_length;

   std::shared_mutex _pending_textures_mutex;
   absl::flat_hash_map<std::string, asset_ref<assets::texture::texture>> _pending_textures;

   struct pending_texture_upload {
      uint32 thumbnail_length;
      std::unique_ptr<std::byte[]> data;
   };

   std::shared_mutex _pending_textures_upload_mutex;
   absl::flat_hash_map<std::string, pending_texture_upload> _pending_textures_upload;

   std::shared_mutex _failed_textures_mutex;
   absl::flat_hash_set<std::string> _failed_textures;

   objects_invalidation_tracker _objects_invalidation_tracker{_asset_libraries};
   textures_invalidation_tracker _textures_invalidation_tracker{_asset_libraries};

   std::optional<async::task<void>> _save_disk_cache_task;
   std::optional<async::task<disk_cache>> _load_disk_cache_task;
   std::optional<async::task<void>> _restore_objects_invalidation_tracker_task;
   std::optional<async::task<void>> _restore_textures_invalidation_tracker_task;

   event_listener<void(const lowercase_string&, asset_ref<assets::odf::definition>,
                       asset_data<assets::odf::definition>)>
      _odf_load_listener = _asset_libraries.odfs.listen_for_loads(
         [this](const lowercase_string& name,
                const asset_ref<assets::odf::definition>& ref,
                const asset_data<assets::odf::definition>& data) {
            {
               std::shared_lock lock{_pending_odfs_mutex};

               if (not _pending_odfs.contains(name)) return;
            }

            odf_loaded(name, ref, data);
         });
   event_listener<void(const lowercase_string&, asset_ref<assets::texture::texture>,
                       asset_data<assets::texture::texture>)>
      _texture_load_listener = _asset_libraries.textures.listen_for_loads(
         [this](const lowercase_string& name,
                const asset_ref<assets::texture::texture>& ref,
                const asset_data<assets::texture::texture>& data) {
            {
               std::shared_lock lock{_pending_textures_mutex};

               if (not _pending_textures.contains(name)) return;
            }

            texture_loaded(name, ref, data);
         });
   event_listener<void(const lowercase_string&, asset_ref<assets::texture::texture>)> _texture_load_failure_listener =
      _asset_libraries.textures.listen_for_load_failures(
         [this](const lowercase_string& name,
                const asset_ref<assets::texture::texture>&) {
            std::scoped_lock lock{_pending_textures_mutex, _failed_textures_mutex};

            _pending_textures.erase(name);
            _failed_textures.emplace(std::string{name});
         });
   event_listener<void(const lowercase_string&)> _texture_change_listener =
      _asset_libraries.textures.listen_for_changes([this](const lowercase_string& name) {
         std::scoped_lock lock{_failed_textures_mutex};

         _failed_textures.erase(name);
      });
};

const float thumbnail_manager::thumbnail_base_length =
   we::graphics::thumbnail_base_length;

auto thumbnail_manager::request_object_class_thumbnail(const std::string_view name)
   -> object_class_thumbnail
{
   return _impl->request_object_class_thumbnail(name);
}

auto thumbnail_manager::request_texture_thumbnail(const std::string_view name)
   -> object_class_thumbnail
{
   return _impl->request_texture_thumbnail(name);
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