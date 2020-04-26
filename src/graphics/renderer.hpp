#pragma once

#include "camera.hpp"
#include "geometric_shapes.hpp"
#include "gpu/buffer.hpp"
#include "gpu/depth_stencil_texture.hpp"
#include "gpu/device.hpp"
#include "gpu/dynamic_buffer_allocator.hpp"
#include "model_manager.hpp"
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
   void draw_world_meta_objects(const camera& camera, const world::world& world,
                                ID3D12GraphicsCommandList5& command_list);

   void build_object_render_list(
      const world::world& world,
      const std::unordered_map<std::string, world::object_class>& world_classes);

   const HWND _window;

   gpu::device _device{_window};
   gpu::command_allocators _world_command_allocators =
      _device.create_command_allocators(D3D12_COMMAND_LIST_TYPE_DIRECT);
   utility::com_ptr<ID3D12GraphicsCommandList5> _world_command_list =
      _device.create_command_list(D3D12_COMMAND_LIST_TYPE_DIRECT);

   gpu::dynamic_buffer_allocator _dynamic_buffer_allocator{1024 * 1024 * 4, _device};

   gpu::depth_stencil_texture _depth_stencil_texture{
      _device,
      {.format = DXGI_FORMAT_D24_UNORM_S8_UINT,
       .width = _device.swap_chain.width(),
       .height = _device.swap_chain.height(),
       .optimized_clear_value = {.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                 .DepthStencil = {.Depth = 0.0f, .Stencil = 0x0}}},
      D3D12_HEAP_TYPE_DEFAULT,
      D3D12_RESOURCE_STATE_DEPTH_WRITE};

   model_manager _model_manager{_device};
   geometric_shapes _geometric_shapes{_device};

   struct render_list_item {
      float distance;
      uint32 index_count;
      uint32 start_index;
      uint32 start_vertex;
      D3D12_INDEX_BUFFER_VIEW index_buffer_view;
      std::array<D3D12_VERTEX_BUFFER_VIEW, 3> vertex_buffer_views;
      float4x4 transform;
   };

   std::vector<render_list_item> _object_render_list;
};

}