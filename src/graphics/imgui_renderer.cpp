
#include "imgui_renderer.hpp"
#include "math/align.hpp"

#include <bit>

#include <imgui.h>

namespace we::graphics {

namespace {

constinit std::byte backend_renderer_user_data_cookie{};

}

imgui_renderer::imgui_renderer(gpu::device& device,
                               copy_command_list_pool& copy_command_list_pool)
   : _device{device}
{
   ImGuiIO& io = ImGui::GetIO();

   IM_ASSERT(io.BackendRendererUserData == nullptr &&
             "Already initialized a renderer backend!");

   io.BackendRendererUserData = &backend_renderer_user_data_cookie;
   io.BackendRendererName = "imgui_impl_rhi";
   io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

   recreate_font_atlas(copy_command_list_pool);
   resize_mesh_buffers(10000, 5000);
}

imgui_renderer::~imgui_renderer()
{
   ImGuiIO& io = ImGui::GetIO();

   io.BackendRendererName = nullptr;
   io.BackendRendererUserData = nullptr;
}

void imgui_renderer::render_draw_data(ImDrawData* draw_data,
                                      root_signature_library& root_signature_library,
                                      pipeline_library& pipeline_library,
                                      gpu::graphics_command_list& command_list)
{
   upload_mesh_data(draw_data);
   setup_draw_state(draw_data, root_signature_library, pipeline_library, command_list);

   uint32 current_index_offset = 0;
   uint32 current_vertex_offset = 0;
   uint32 current_texture_index = UINT32_MAX;

   for (const ImDrawList* cmd_list : draw_data->CmdLists) {
      for (const ImDrawCmd& cmd : cmd_list->CmdBuffer) {
         if (cmd.UserCallback != nullptr) {
            if (cmd.UserCallback == ImDrawCallback_ResetRenderState) {
               current_texture_index = UINT32_MAX;

               setup_draw_state(draw_data, root_signature_library,
                                pipeline_library, command_list);
            }
            else {
               cmd.UserCallback(cmd_list, &cmd);
            }
         }
         else {
            const ImVec2 clip_min{cmd.ClipRect.x - draw_data->DisplayPos.x,
                                  cmd.ClipRect.y - draw_data->DisplayPos.y};
            const ImVec2 clip_max{cmd.ClipRect.z - draw_data->DisplayPos.x,
                                  cmd.ClipRect.w - draw_data->DisplayPos.y};

            if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y) continue;

            command_list.rs_set_scissor_rects(
               {static_cast<uint32>(clip_min.x), static_cast<uint32>(clip_min.y),
                static_cast<uint32>(clip_max.x), static_cast<uint32>(clip_max.y)});

            const uint32 texture_index =
               static_cast<uint32>(std::bit_cast<std::uintptr_t>(cmd.GetTexID()));

            if (std::exchange(current_texture_index, texture_index) != texture_index) {
               command_list.set_graphics_32bit_constant(rs::imgui::texture,
                                                        current_texture_index, 0);
            }

            command_list.draw_indexed_instanced(cmd.ElemCount, 1,
                                                cmd.IdxOffset + current_index_offset,
                                                cmd.VtxOffset + current_vertex_offset,
                                                0);
         }
      }

      current_index_offset += cmd_list->IdxBuffer.Size;
      current_vertex_offset += cmd_list->VtxBuffer.Size;
   }
}

void imgui_renderer::upload_mesh_data(ImDrawData* draw_data)
{
   if (static_cast<uint32>(draw_data->TotalIdxCount) > _indices_size or
       static_cast<uint32>(draw_data->TotalVtxCount) > _vertices_size) {
      resize_mesh_buffers(draw_data->TotalIdxCount + 10000,
                          draw_data->TotalVtxCount + 5000);
   }

   uint16* index_buffer =
      reinterpret_cast<uint16*>(_mesh_buffers_cpu[_device.frame_index()]);
   ImDrawVert* vertex_buffer = reinterpret_cast<ImDrawVert*>(
      _mesh_buffers_cpu[_device.frame_index()] + _vertices_offset);

   for (const ImDrawList* cmd_list : draw_data->CmdLists) {
      std::memcpy(index_buffer, cmd_list->IdxBuffer.Data,
                  cmd_list->IdxBuffer.Size * sizeof(uint16));
      std::memcpy(vertex_buffer, cmd_list->VtxBuffer.Data,
                  cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));

      index_buffer += cmd_list->IdxBuffer.Size;
      vertex_buffer += cmd_list->VtxBuffer.Size;
   }
}

void imgui_renderer::setup_draw_state(ImDrawData* draw_data,
                                      root_signature_library& root_signature_library,
                                      pipeline_library& pipeline_library,
                                      gpu::graphics_command_list& command_list)
{
   command_list.set_graphics_root_signature(root_signature_library.imgui.get());
   command_list.set_pipeline_state(pipeline_library.imgui.get());

   const std::array inv_viewport_size{1.0f / draw_data->DisplaySize.x,
                                      1.0f / draw_data->DisplaySize.y};

   command_list.set_graphics_32bit_constants(rs::imgui::inv_viewport_size,
                                             std::as_bytes(std::span{inv_viewport_size}),
                                             0);

   command_list.ia_set_index_buffer(
      {.buffer_location = _mesh_buffers_gpu[_device.frame_index()],
       .size_in_bytes = _indices_size * static_cast<uint32>(sizeof(uint16))});
   command_list.ia_set_vertex_buffers(
      0, gpu::vertex_buffer_view{
            .buffer_location = _mesh_buffers_gpu[_device.frame_index()] + _vertices_offset,
            .size_in_bytes = _vertices_size * static_cast<uint32>(sizeof(ImDrawVert)),
            .stride_in_bytes = static_cast<uint32>(sizeof(ImDrawVert))});
   command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

   command_list.rs_set_viewports(
      {.width = draw_data->DisplaySize.x, .height = draw_data->DisplaySize.y});
}

void imgui_renderer::recreate_font_atlas(copy_command_list_pool& copy_command_list_pool)
{
   ImGuiIO& io = ImGui::GetIO();
   unsigned char* pixels = nullptr;
   int width = 0;
   int height = 0;
   io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

   int texture_pitch = math::align_up(width, gpu::texture_data_pitch_alignment);

   const uint32 upload_buffer_size = static_cast<uint32>(texture_pitch * height);

   gpu::unique_resource_handle upload_buffer{
      _device.create_buffer({.size = static_cast<uint32>(texture_pitch * height),
                             .debug_name = "dear ImGui Font Atlas Upload"},
                            gpu::heap_type::upload),
      _device.copy_queue};

   std::byte* upload_ptr =
      static_cast<std::byte*>(_device.map(upload_buffer.get(), 0, {0, 0}));

   for (int y = 0; y < height; ++y) {
      std::memcpy(upload_ptr + (texture_pitch * y), pixels + (width * y), width);
   }

   _device.unmap(upload_buffer.get(), 0, {0, upload_buffer_size});

   _font_texture = {_device.create_texture({.dimension = gpu::texture_dimension::t_2d,
                                            .format = DXGI_FORMAT_R8_UNORM,
                                            .width = static_cast<uint32>(width),
                                            .height = static_cast<uint32>(height),
                                            .debug_name =
                                               "dear ImGui Font Atlas"},
                                           gpu::barrier_layout::common,
                                           gpu::legacy_resource_state::common),
                    _device.direct_queue};

   pooled_copy_command_list command_list = copy_command_list_pool.aquire_and_reset();

   command_list->copy_buffer_to_texture(_font_texture.get(), 0, 0, 0, 0,
                                        upload_buffer.get(),
                                        {.format = DXGI_FORMAT_R8_UNORM,
                                         .width = static_cast<uint32>(width),
                                         .height = static_cast<uint32>(height),
                                         .row_pitch =
                                            static_cast<uint32>(texture_pitch)});

   command_list->close();

   _device.copy_queue.execute_command_lists(command_list.get());
   _device.direct_queue.sync_with(_device.copy_queue);

   _font_srv = {_device.create_shader_resource_view(
                   _font_texture.get(),
                   {.format = DXGI_FORMAT_R8_UNORM,
                    .shader_4_component_mapping =
                       {
                          .component_0 = gpu::shader_component_mapping::force_value_1,
                          .component_1 = gpu::shader_component_mapping::force_value_1,
                          .component_2 = gpu::shader_component_mapping::force_value_1,
                          .component_3 = gpu::shader_component_mapping::from_memory_component_0,
                       }}),
                _device.direct_queue};

   io.Fonts->SetTexID(std::bit_cast<ImTextureID>(std::uintptr_t{_font_srv.get().index}));
}

void imgui_renderer::resize_mesh_buffers(uint32 indices_size, uint32 vertices_size)
{
   const uint32 indices_byte_size =
      static_cast<uint32>(math::align_up(indices_size * sizeof(uint16), 65536));
   const uint32 vertices_byte_size =
      static_cast<uint32>(math::align_up(vertices_size * sizeof(ImDrawVert), 65536));

   indices_size = indices_byte_size / sizeof(uint16);
   vertices_size = vertices_byte_size / sizeof(ImDrawVert);

   const uint32 buffer_slice_size = indices_byte_size + vertices_byte_size;
   const uint32 buffer_size = buffer_slice_size * gpu::frame_pipeline_length;

   _mesh_buffer = {_device.create_buffer({.size = buffer_size, .debug_name = "dear ImGui Mesh Buffer"},
                                         gpu::heap_type::upload),
                   _device.direct_queue};

   for (std::size_t i = 0; i < gpu::frame_pipeline_length; ++i) {
      const std::size_t slice_offset = buffer_slice_size * i;

      _mesh_buffers_cpu[i] =
         static_cast<std::byte*>(_device.map(_mesh_buffer.get(), 0, {0, 0})) + slice_offset;
      _mesh_buffers_gpu[i] =
         _device.get_gpu_virtual_address(_mesh_buffer.get()) + slice_offset;
   }

   _indices_size = indices_size;
   _vertices_size = vertices_size;
   _vertices_offset = indices_byte_size;
}
}
