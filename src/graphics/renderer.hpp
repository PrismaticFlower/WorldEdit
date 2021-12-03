#pragma once

#include "assets/asset_libraries.hpp"
#include "camera.hpp"
#include "frustrum.hpp"
#include "geometric_shapes.hpp"
#include "gpu/buffer.hpp"
#include "gpu/command_list.hpp"
#include "gpu/depth_stencil_texture.hpp"
#include "gpu/device.hpp"
#include "gpu/dynamic_buffer_allocator.hpp"
#include "light_clusters.hpp"
#include "model_manager.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "shader_library.hpp"
#include "shader_list.hpp"
#include "terrain.hpp"
#include "texture_manager.hpp"
#include "world/object_class.hpp"
#include "world/world.hpp"
#include "world_mesh_list.hpp"

#include <array>
#include <vector>

#include <absl/container/flat_hash_map.h>

namespace we::graphics {

struct renderer_config {
   bool use_raytracing = true;
};

class renderer {
public:
   renderer(const HWND window, assets::libraries_manager& asset_libraries);

   void draw_frame(
      const camera& camera, const world::world& world,
      const absl::flat_hash_map<lowercase_string, std::shared_ptr<world::object_class>>& world_classes);

   void window_resized(uint16 width, uint16 height);

   void mark_dirty_terrain() noexcept;

private:
   struct render_list_item {
      float distance;
      ID3D12PipelineState* pipeline;
      ID3D12PipelineState* depth_prepass_pipeline;
      D3D12_INDEX_BUFFER_VIEW index_buffer_view;
      std::array<D3D12_VERTEX_BUFFER_VIEW, 2> vertex_buffer_views;
      gpu::virtual_address object_constants_address;
      gpu::descriptor_range material_descriptor_range;
      uint32 index_count;
      uint32 start_index;
      uint32 start_vertex;
   };

   void update_camera_constant_buffer(const camera& camera,
                                      gpu::graphics_command_list& command_list);

   void draw_world(const frustrum& view_frustrum,
                   gpu::graphics_command_list& command_list);

   void draw_world_render_list_depth_prepass(const std::vector<render_list_item>& list,
                                             gpu::graphics_command_list& command_list);

   void draw_world_render_list(const std::vector<render_list_item>& list,
                               gpu::graphics_command_list& command_list);

   void draw_world_meta_objects(
      const frustrum& view_frustrum, const world::world& world,
      const absl::flat_hash_map<lowercase_string, std::shared_ptr<world::object_class>>& world_classes,
      gpu::graphics_command_list& command_list);

   void build_world_mesh_list(
      gpu::graphics_command_list& command_list, const world::world& world,
      const absl::flat_hash_map<lowercase_string, std::shared_ptr<world::object_class>>& world_classes);

   void build_object_render_list(const frustrum& view_frustrum);

   void update_textures(gpu::graphics_command_list& command_list);

   auto create_raytacing_blas(gpu::graphics_command_list& command_list,
                              const model& model, const mesh_part& part) -> gpu::buffer;

   const HWND _window;

   bool _terrain_dirty = true; // ughhhh, this feels so ugly

   gpu::device _device{_window};
   gpu::graphics_command_list _world_command_list =
      _device.create_graphics_command_list("World Command List");

   gpu::dynamic_buffer_allocator _dynamic_buffer_allocator{1024 * 1024 * 4, _device};

   gpu::buffer _camera_constant_buffer =
      _device.create_buffer({.size = math::align_up((uint32)sizeof(float4x4), 256)},
                            D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);
   gpu::descriptor_allocation _camera_constant_buffer_view =
      _device.allocate_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);

   gpu::depth_stencil_texture _depth_stencil_texture{
      _device,
      {.format = DXGI_FORMAT_D24_UNORM_S8_UINT,
       .width = _device.swap_chain.width(),
       .height = _device.swap_chain.height(),
       .optimized_clear_value = {.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                 .DepthStencil = {.Depth = 1.0f, .Stencil = 0x0}}},
      D3D12_RESOURCE_STATE_DEPTH_WRITE};

   shader_library _shaders{shader_list};
   root_signature_library _root_signatures{_device};
   pipeline_library _pipelines{*_device.device_d3d, _shaders, _root_signatures};

   texture_manager _texture_manager;
   model_manager _model_manager;
   geometric_shapes _geometric_shapes{_device};
   light_clusters _light_clusters{_device, _device.swap_chain.width(),
                                  _device.swap_chain.height()};
   terrain _terrain{_device, _texture_manager};

   constexpr static std::size_t max_drawn_objects = 2048;
   constexpr static std::size_t objects_constants_buffer_size =
      max_drawn_objects * sizeof(world_mesh_constants);

   std::array<gpu::buffer, gpu::render_latency> _object_constants_upload_buffers;
   std::array<std::byte*, gpu::render_latency> _object_constants_upload_cpu_ptrs;
   gpu::buffer _object_constants_buffer =
      _device.create_buffer({.size = objects_constants_buffer_size},
                            D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);

   world_mesh_list _world_mesh_list;
   std::vector<render_list_item> _opaque_object_render_list;
   std::vector<render_list_item> _transparent_object_render_list;

   renderer_config _config;
};
}
