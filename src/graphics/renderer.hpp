#pragma once

#include "camera.hpp"
#include "gpu/buffer.hpp"
#include "gpu/device.hpp"
#include "gpu/dynamic_buffer_allocator.hpp"
#include "world/world.hpp"

namespace sk::graphics {

class renderer {
public:
   explicit renderer(const HWND window);

   void draw_frame(const camera& camera, const world::world& world);

   void window_resized(int width, int height);

private:
   const HWND _window;

   gpu::device _device{_window};

   gpu::command_allocators _command_allocators =
      _device.create_command_allocators(D3D12_COMMAND_LIST_TYPE_DIRECT);

   gpu::dynamic_buffer_allocator _dynamic_buffer_allocator{1024 * 1024 * 4, _device};

   graphics::gpu::buffer _box_vertex_buffer;
   graphics::gpu::buffer _box_index_buffer;
};

}