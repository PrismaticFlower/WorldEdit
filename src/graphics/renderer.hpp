#pragma once

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
#include "texture_manager.hpp"
#include "world/object_class.hpp"
#include "world/world.hpp"

#include <array>
#include <unordered_map>
#include <vector>

namespace sk::graphics {

class renderer {
public:
   explicit renderer(const HWND window);

   void draw_frame(const camera& camera, const world::world& world,
                   const std::unordered_map<std::string, world::object_class>& world_classes);

   void window_resized(uint16 width, uint16 height);

private:
   void update_camera_constant_buffer(const camera& camera,
                                      gpu::command_list& command_list);

   void draw_world(const frustrum& view_frustrum, const world::world& world,
                   const std::unordered_map<std::string, world::object_class>& world_classes,
                   gpu::command_list& command_list);

   void draw_world_meta_objects(
      const frustrum& view_frustrum, const world::world& world,
      const std::unordered_map<std::string, world::object_class>& world_classes,
      gpu::command_list& command_list);

   void build_object_render_list(
      const frustrum& view_frustrum, const world::world& world,
      const std::unordered_map<std::string, world::object_class>& world_classes);

   const HWND _window;

   gpu::device _device{_window};
   gpu::command_allocators _world_command_allocators =
      _device.create_command_allocators(D3D12_COMMAND_LIST_TYPE_DIRECT);
   gpu::command_list _world_command_list{D3D12_COMMAND_LIST_TYPE_DIRECT, _device};

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
                                 .DepthStencil = {.Depth = 0.0f, .Stencil = 0x0}}},
      D3D12_RESOURCE_STATE_DEPTH_WRITE};

   texture_manager _texture_manager{_device};
   model_manager _model_manager{_device, _texture_manager};
   geometric_shapes _geometric_shapes{_device};
   light_clusters _light_clusters{_device};

   struct render_list_item {
      uint32 index_count;
      uint32 start_index;
      uint32 start_vertex;
      D3D12_INDEX_BUFFER_VIEW index_buffer_view;
      std::array<D3D12_VERTEX_BUFFER_VIEW, 3> vertex_buffer_views;
      D3D12_GPU_VIRTUAL_ADDRESS object_constants_address;
   };

   std::vector<render_list_item> _object_render_list;
};

}
