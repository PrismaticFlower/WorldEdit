
#include "imgui_renderer.hpp"

#include "math/align.hpp"

#include <imgui.h>

namespace we::graphics {

namespace {

constinit std::byte backend_renderer_user_data_cookie{};

}

imgui_renderer::imgui_renderer(gpu::device& device) : _device{device}
{
   ImGuiIO& io = ImGui::GetIO();

   IM_ASSERT(io.BackendRendererUserData == nullptr &&
             "Already initialized a renderer backend!");

   io.BackendRendererUserData = &backend_renderer_user_data_cookie;
   io.BackendRendererName = "imgui_impl_rhi";
   io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
   io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

   resize_mesh_buffers(10000, 5000);
}

imgui_renderer::~imgui_renderer()
{
   ImGuiIO& io = ImGui::GetIO();

   io.BackendRendererName = nullptr;
   io.BackendRendererUserData = nullptr;

   for (ImTextureData* tex : ImGui::GetPlatformIO().Textures) {
      tex->SetTexID(ImTextureID_Invalid);
      tex->SetStatus(ImTextureStatus_Destroyed);
   }
}

void imgui_renderer::render_draw_data(ImDrawData* draw_data,
                                      root_signature_library& root_signature_library,
                                      pipeline_library& pipeline_library,
                                      dynamic_buffer_allocator& dynamic_buffer_allocator,
                                      gpu::graphics_command_list& command_list)
{
   update_textures(draw_data, dynamic_buffer_allocator, command_list);
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

void imgui_renderer::update_textures(ImDrawData* draw_data,
                                     dynamic_buffer_allocator& dynamic_buffer_allocator,
                                     gpu::graphics_command_list& command_list)
{
   if (draw_data->Textures == nullptr) return;

   bool need_flush_barriers = false;

   for (ImTextureData* tex : *draw_data->Textures) {
      if (tex->Status == ImTextureStatus_WantCreate) {
         const std::size_t texture_slot = allocate_texture_slot();

         texture& gpu_texture = _textures[texture_slot];

         const DXGI_FORMAT format = [&] {
            switch (tex->Format) {
            case ImTextureFormat_RGBA32:
               return DXGI_FORMAT_R8G8B8A8_UNORM;
            case ImTextureFormat_Alpha8:
               return DXGI_FORMAT_A8_UNORM;
            default:
               std::unreachable();
            }
         }();

         gpu_texture.resource = {_device.create_texture(
                                    {
                                       .dimension = gpu::texture_dimension::t_2d,
                                       .format = format,
                                       .width = static_cast<uint32>(tex->Width),
                                       .height = static_cast<uint32>(tex->Height),
                                       .debug_name = "Dear ImGui Texture",
                                    },
                                    gpu::barrier_layout::direct_queue_copy_dest,
                                    gpu::legacy_resource_state::copy_dest),
                                 _device};
         gpu_texture.srv =
            {_device.create_shader_resource_view(gpu_texture.resource.get(),
                                                 {
                                                    .format = format,
                                                 }),
             _device};

         const uint32 upload_pitch =
            math::align_up(static_cast<uint32>(tex->GetPitch()),
                           gpu::texture_data_pitch_alignment);
         const uint64 upload_size = upload_pitch * static_cast<uint32>(tex->Height);

         gpu::unique_resource_handle upload_buffer =
            {_device.create_buffer({.size = upload_size,
                                    .debug_name = "Dear ImGui Upload Texture"},
                                   gpu::heap_type::upload),
             _device};

         std::byte* const upload_ptr =
            static_cast<std::byte*>(_device.map(upload_buffer.get(), 0, {}));

         for (int y = 0; y < tex->Height; ++y) {
            std::memcpy(upload_ptr + y * upload_pitch, tex->GetPixelsAt(0, y),
                        tex->GetPitch());
         }

         command_list.copy_buffer_to_texture(gpu_texture.resource.get(), 0, 0,
                                             0, 0, upload_buffer.get(),
                                             {.format = format,
                                              .width = static_cast<uint32>(tex->Width),
                                              .height = static_cast<uint32>(tex->Height),
                                              .row_pitch = upload_pitch});

         [[likely]] if (_device.supports_enhanced_barriers()) {
            command_list.deferred_barrier(gpu::texture_barrier{
               .sync_before = gpu::barrier_sync::copy,
               .sync_after = gpu::barrier_sync::pixel_shading,
               .access_before = gpu::barrier_access::copy_dest,
               .access_after = gpu::barrier_access::shader_resource,
               .layout_before = gpu::barrier_layout::direct_queue_copy_dest,
               .layout_after = gpu::barrier_layout::direct_queue_shader_resource,
               .resource = gpu_texture.resource.get()});
         }
         else {
            command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
               .resource = gpu_texture.resource.get(),
               .state_before = gpu::legacy_resource_state::copy_dest,
               .state_after = gpu::legacy_resource_state::pixel_shader_resource});
         }

         command_list.reference_resource(upload_buffer.get());

         need_flush_barriers = true;

         tex->SetTexID(gpu_texture.srv.get().index);
         tex->SetStatus(ImTextureStatus_OK);
         tex->BackendUserData = reinterpret_cast<void*>(texture_slot);
      }
      else if (tex->Status == ImTextureStatus_WantUpdates) {
         const DXGI_FORMAT format = [&] {
            switch (tex->Format) {
            case ImTextureFormat_RGBA32:
               return DXGI_FORMAT_R8G8B8A8_UNORM;
            case ImTextureFormat_Alpha8:
               return DXGI_FORMAT_A8_UNORM;
            default:
               std::unreachable();
            }
         }();

         texture& gpu_texture =
            _textures[reinterpret_cast<uint64>(tex->BackendUserData)];

         [[likely]] if (_device.supports_enhanced_barriers()) {
            command_list.deferred_barrier(
               gpu::texture_barrier{.sync_before = gpu::barrier_sync::none,
                                    .sync_after = gpu::barrier_sync::copy,
                                    .access_before = gpu::barrier_access::no_access,
                                    .access_after = gpu::barrier_access::copy_dest,
                                    .layout_before = gpu::barrier_layout::direct_queue_shader_resource,
                                    .layout_after = gpu::barrier_layout::direct_queue_copy_dest,
                                    .resource = gpu_texture.resource.get()});
         }
         else {
            command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
               .resource = gpu_texture.resource.get(),
               .state_before = gpu::legacy_resource_state::pixel_shader_resource,
               .state_after = gpu::legacy_resource_state::copy_dest});
         }

         command_list.flush_barriers();

         for (const ImTextureRect& update : tex->Updates) {
            const uint32 update_pitch =
               static_cast<uint32>(update.w * tex->BytesPerPixel);

            const uint32 upload_pitch =
               math::align_up(update_pitch, gpu::texture_data_pitch_alignment);
            const uint64 upload_size = upload_pitch * static_cast<uint32>(update.h);

            dynamic_buffer_allocator::allocation upload_allocation =
               dynamic_buffer_allocator.allocate_aligned(upload_size,
                                                         gpu::texture_data_placement_alignment);

            for (int y = 0; y < update.h; ++y) {
               std::memcpy(upload_allocation.cpu_address + y * upload_pitch,
                           tex->GetPixelsAt(update.x, update.y + y), update_pitch);
            }

            command_list
               .copy_buffer_to_texture(gpu_texture.resource.get(), 0, update.x,
                                       update.y, 0, upload_allocation.resource,
                                       {.offset = upload_allocation.offset,
                                        .format = format,
                                        .width = static_cast<uint32>(update.w),
                                        .height = static_cast<uint32>(update.h),
                                        .row_pitch = upload_pitch});
         }

         [[likely]] if (_device.supports_enhanced_barriers()) {
            command_list.deferred_barrier(gpu::texture_barrier{
               .sync_before = gpu::barrier_sync::copy,
               .sync_after = gpu::barrier_sync::pixel_shading,
               .access_before = gpu::barrier_access::copy_dest,
               .access_after = gpu::barrier_access::shader_resource,
               .layout_before = gpu::barrier_layout::direct_queue_copy_dest,
               .layout_after = gpu::barrier_layout::direct_queue_shader_resource,
               .resource = gpu_texture.resource.get()});
         }
         else {
            command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
               .resource = gpu_texture.resource.get(),
               .state_before = gpu::legacy_resource_state::copy_dest,
               .state_after = gpu::legacy_resource_state::pixel_shader_resource});
         }

         need_flush_barriers = true;

         tex->SetStatus(ImTextureStatus_OK);
      }
      else if (tex->Status == ImTextureStatus_WantDestroy and tex->UnusedFrames > 0) {
         _textures[reinterpret_cast<uint64>(tex->BackendUserData)] = {};

         tex->SetTexID(ImTextureID_Invalid);
         tex->SetStatus(ImTextureStatus_Destroyed);
         tex->BackendUserData = nullptr;
      }
   }

   if (need_flush_barriers) command_list.flush_barriers();
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
                   _device};

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

auto imgui_renderer::allocate_texture_slot() noexcept -> std::size_t
{
   for (std::size_t i = 0; i < _textures.size(); ++i) {
      if (not _textures[i].resource) return i;
   }

   const std::size_t new_index = _textures.size();

   _textures.emplace_back();

   return new_index;
}

}
