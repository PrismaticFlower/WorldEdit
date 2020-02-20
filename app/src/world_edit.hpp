
#include "graphics/gpu/device.hpp"

#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>

namespace sk {

class world_edit {
public:
   explicit world_edit(const HWND window);

   bool update();

   void resized(int width, int height);

   void focused();

   void unfocused();

private:
   HWND _window{};
   graphics::gpu::device _gpu_device;
   graphics::gpu::buffer _triangle_buffer;
};

}