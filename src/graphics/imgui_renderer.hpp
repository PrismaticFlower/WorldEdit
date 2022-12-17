#pragma once

#include "copy_command_list_pool.hpp"
#include "dynamic_buffer_allocator.hpp"
#include "gpu/resource.hpp"
#include "gpu/rhi.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"

#include <array>

struct ImDrawData;

namespace we::graphics {

struct imgui_renderer {
   imgui_renderer(gpu::device& device, copy_command_list_pool& copy_command_list_pool);

   ~imgui_renderer();

   void render_draw_data(ImDrawData* draw_data,
                         root_signature_library& root_signature_library,
                         pipeline_library& pipeline_library,
                         gpu::graphics_command_list& command_list);

   void recreate_font_atlas(copy_command_list_pool& copy_command_list_pool);

private:
   void upload_mesh_data(ImDrawData* draw_data);

   void setup_draw_state(ImDrawData* draw_data,
                         root_signature_library& root_signature_library,
                         pipeline_library& pipeline_library,
                         gpu::graphics_command_list& command_list);

   void resize_mesh_buffers(uint32 indices_count, uint32 vertices_count);

   gpu::device& _device;

   gpu::unique_resource_handle _font_texture;
   gpu::unique_resource_view _font_srv;

   gpu::unique_resource_handle _mesh_buffer;
   std::array<std::byte*, gpu::frame_pipeline_length> _mesh_buffers_cpu;
   std::array<gpu_virtual_address, gpu::frame_pipeline_length> _mesh_buffers_gpu;

   uint32 _indices_size = 0;
   uint32 _vertices_size = 0;
   uint32 _vertices_offset = 0;
};

}
