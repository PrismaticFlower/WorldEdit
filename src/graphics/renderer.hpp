#pragma once

#include "camera.hpp"
#include "gpu/device.hpp"

namespace sk::graphics {

class renderer {
public:
   explicit renderer(const HWND window);

   void draw_frame();

   void window_resized(int width, int height);

private:
   const HWND _window;
   gpu::device _device{_window};
   graphics::gpu::buffer _box_vertex_buffer;
   graphics::gpu::buffer _box_index_buffer;
   graphics::gpu::buffer _temp_proj_matrix;
};

}