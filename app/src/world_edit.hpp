
#include "graphics/gpu_device.hpp"

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
   graphics::gpu_device _gpu_device;
   graphics::buffer _triangle_buffer;
};

}