#pragma once

#include "gpu/rhi.hpp"
#include "object_class_thumbnail.hpp"
#include "utility/implementation_storage.hpp"

#include <memory>
#include <string_view>

namespace we {

class output_stream;

}

namespace we::async {

class thread_pool;

}

namespace we::assets {

struct libraries_manager;

}

namespace we::graphics {

struct model_manager;
struct dynamic_buffer_allocator;
struct root_signature_library;
struct pipeline_library;

struct thumbnail_manager_init {
   const std::shared_ptr<async::thread_pool>& thread_pool;
   assets::libraries_manager& asset_libraries;
   output_stream& error_output;
   gpu::device& device;
   float display_scale = 1.0f;
};

struct thumbnail_manager {
   explicit thumbnail_manager(const thumbnail_manager_init& init);

   ~thumbnail_manager();

   thumbnail_manager(const thumbnail_manager& other) noexcept = delete;
   auto operator=(const thumbnail_manager& other) noexcept
      -> thumbnail_manager& = delete;

   thumbnail_manager(thumbnail_manager&& other) noexcept = delete;
   auto operator=(thumbnail_manager&& other) noexcept -> thumbnail_manager& = delete;

   [[nodiscard]] auto request_object_class_thumbnail(const std::string_view name)
      -> object_class_thumbnail;

   [[nodiscard]] auto request_texture_thumbnail(const std::string_view name)
      -> object_class_thumbnail;

   void update_cpu_cache();

   void update_gpu(model_manager& model_manager,
                   root_signature_library& root_signature_library,
                   pipeline_library& pipeline_library,
                   dynamic_buffer_allocator& dynamic_buffer_allocator,
                   gpu::graphics_command_list& command_list);

   void end_frame();

   void display_scale_changed(const float new_display_scale);

   void async_save_disk_cache(const char* path) noexcept;

   void async_load_disk_cache(const char* path) noexcept;

   void reset() noexcept;

private:
   struct impl;

   implementation_storage<impl, 2048> _impl;
};

}